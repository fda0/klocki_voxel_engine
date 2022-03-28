
static_function Mesh *
get_new_mesh(World *world)
{
    Mesh *result = world->first_free_mesh;
    if (result)
    {
        world->first_free_mesh = result->next;
    }
    else
    {
        result = push_struct(&world->mesh_arena, Mesh);
    }
    
    result->face_count = 0;
    result->next = nullptr;
    return result;
}

static_function Mesh *
get_new_mesh(Worker_Job<Mesh *> *mesh_resource)
{
    assert(mesh_resource->task_start != mesh_resource->task_end);
    Mesh *result = mesh_resource->tasks[mesh_resource->task_start].payload;
    mesh_resource->task_start = (mesh_resource->task_start + 1) % array_count(mesh_resource->tasks);
    return result;
}


static_function Mesh_Face *
mesh_push_face(Mesh *mesh)
{
    assert(mesh->face_count < Mesh_Face_Count);
    Mesh_Face *result = mesh->faces + mesh->face_count;
    mesh->face_count += 1;
    return result;
}







static_function void
sort_mesh_faces_back_to_front(World *world, Chunk *chunk, Mesh *first_mesh)
{
    profile_function();
    assert(first_mesh);
    
    Camera *camera = &world->camera;
    chunk->transparecy_sorted_at_chunk = camera->at_chunk;
    chunk->transparency_sorted_at_rel_p = camera->p;
    
    Vec3_S64 chunk_offset = camera->at_chunk - chunk->at;
    v3 origin = v3_from_vec3_s64(hadamard(chunk_offset, Chunk_Dim_Vec3_S64));
    origin += camera->p;
    origin -= V3(0.5f); // shift by half so blocks have center at 0,0,0
    
    Scratch scratch(0);
    Checked_Array<Sort_Mesh_Face> sort_faces = create_checked_array<Sort_Mesh_Face>(scratch);
    
    for (Mesh *mesh = first_mesh;
         mesh;
         mesh = mesh->next)
    {
        for_s32(face_index, mesh->face_count)
        {
            Mesh_Face face = mesh->faces[face_index];
            v3 p = {
                (f32)((face.Normal3_TexId14_Pos15      ) & Bitmask_5),
                (f32)((face.Normal3_TexId14_Pos15 >>  5) & Bitmask_5),
                (f32)((face.Normal3_TexId14_Pos15 >> 10) & Bitmask_5),
            };
            
            *sort_faces.expand(scratch) = {
                face,
                get_length_sq(origin - p)
            };
        }
    }
    
    
    
    Sort_Mesh_Face *temp_sort_face = push_array(scratch, Sort_Mesh_Face, sort_faces.count);
    Sort_Mesh_Face *sorted_faces = merge_sort_double_array(sort_faces.array, temp_sort_face, sort_faces.count);
    u64 sorted_index = 0;
    
    for (Mesh *mesh = first_mesh;
         mesh;
         mesh = mesh->next)
    {
        for_s32(face_index, mesh->face_count)
        {
            mesh->faces[face_index] = sorted_faces[sorted_index].face;
            sorted_index += 1;
        }
    }
    
    assert(sorted_index == sort_faces.count);
}




static_function Mesh_Face *
mesh_push_face_from_resouce(Mesh **first_mesh, Worker_Job<Mesh *> *mesh_resource)
{
    if (!(*first_mesh))
    {
        (*first_mesh) = get_new_mesh(mesh_resource);
    }
    if ((*first_mesh)->face_count >= Mesh_Face_Count)
    {
        assert((*first_mesh)->face_count == Mesh_Face_Count);
        Mesh *new_mesh = get_new_mesh(mesh_resource);
        new_mesh->next = (*first_mesh);
        (*first_mesh) = new_mesh;
    }
    
    Mesh_Face *face = mesh_push_face((*first_mesh));
    return face;
}







static_function void
generate_chunk_mesh(Chunk_Neighborhood *neighborhood,
                    Mesh **first_opaque_mesh, Mesh **first_transparent_mesh,
                    Worker_Job<Mesh *> *mesh_resource)
{
    profile_function();
    
    Chunk *center = get_center_chunk(neighborhood);
    assert(!center->mesh_is_valid);
    assert(!(*first_opaque_mesh));
    assert(!(*first_transparent_mesh));
    assert(center->can_contain_blocks);
    
    if (center->early_free)
    {
        for_s32(hood_index, array_count(neighborhood->all))
        {
            decrement_chunk_lock(neighborhood->all[hood_index]);
        }
        return;
    }
    
    
    
    // quad vert index pos for face up [if we look at +Z (north)]
    /*
            2    3
                              
            0    1
        */
    u32 ambient_values[8] =
    {
        // descriptions for up face
        (1 << 4) | (1 << 6), // North
        (1 << 0) | (1 << 4), // East
        (1 << 0) | (1 << 2), // South
        (1 << 2) | (1 << 6), // West
        (1 << 4), // NE [3]
        (1 << 0), // SE [1]
        (1 << 2), // SW [0]
        (1 << 6), // NW [2]
    };
    
    Vec3_S64 ambient_offsets[DirIndex_Count][8] =
    {
        {{0,0,+1}, {+1,0,0}, {0,0,-1}, {-1,0,0},  {+1,0,+1}, {+1,0,-1}, {-1,0,-1}, {-1,0,+1}}, // up
        {{0,0,+1}, {-1,0,0}, {0,0,-1}, {+1,0,0},  {-1,0,+1}, {-1,0,-1}, {+1,0,-1}, {+1,0,+1}}, // down
        {{0,+1,0}, {-1,0,0}, {0,-1,0}, {+1,0,0},  {-1,+1,0}, {-1,-1,0}, {+1,-1,0}, {+1,+1,0}}, // north
        {{0,+1,0}, {+1,0,0}, {0,-1,0}, {-1,0,0},  {+1,+1,0}, {+1,-1,0}, {-1,-1,0}, {-1,+1,0}}, // south
        {{0,+1,0}, {0,0,+1}, {0,-1,0}, {0,0,-1},  {0,+1,+1}, {0,-1,+1}, {0,-1,-1}, {0,+1,-1}}, // east
        {{0,+1,0}, {0,0,-1}, {0,-1,0}, {0,0,+1},  {0,+1,-1}, {0,-1,-1}, {0,-1,+1}, {0,+1,+1}}, // west
    };
    
    
    
    
    for_s32(block_z, Chunk_Dim_Z)
    {
        for_s32(block_y, Chunk_Dim_Y)
        {
            for_s32(block_x, Chunk_Dim_X)
            {
                Vec3_S64 block_p = {block_x, block_y, block_z};
                Block *block = &center->blocks[block_z][block_y][block_x];
                Block_Type block_type = block->type;
                
                if (block_type)
                {
                    Block_Tex_Ids tex_ids = map_block_to_tex_id[block_type];
                    
                    if (is_translucent(block_type))
                    {
                        for_s32(dir_index, DirIndex_Count)
                        {
                            Block_Type neighbor_type = {};
                            if (block_type != Block_OakLeaves)
                            {
                                Vec3_S64 neighbor_p = block_p + get_dir_offset_s32(dir_index);
                                neighbor_type = get_block_in_neighborhood(neighborhood, neighbor_p).type;
                            }
                            
                            if (neighbor_type != block_type)
                            {
                                u32 pos15bit = pack_pos15(block_x, block_y, block_z);
                                u32 tex_id = tex_ids.dirs[dir_index];
                                
                                Mesh_Face *face = mesh_push_face_from_resouce(first_transparent_mesh, mesh_resource);
                                face->Normal3_TexId14_Pos15 = (pos15bit |
                                                               (tex_id << 15) |
                                                               (dir_index << 29));
                                face->Ambient8 = {};
                            }
                        }
                    }
                    else
                    {
                        for_s32(dir_index, DirIndex_Count)
                        {
                            Vec3_S64 neighbor_p = block_p + get_dir_offset_s32(dir_index);
                            Block_Type neighbor_type = get_block_in_neighborhood(neighborhood, neighbor_p).type;
                            
                            if (is_translucent(neighbor_type))
                            {
                                u32 pos15bit = pack_pos15(block_x, block_y, block_z);
                                u32 tex_id = tex_ids.dirs[dir_index];
                                
                                Mesh_Face *face = mesh_push_face_from_resouce(first_opaque_mesh, mesh_resource);
                                face->Normal3_TexId14_Pos15 = (pos15bit |
                                                               (tex_id << 15) |
                                                               (dir_index << 29));
                                face->Ambient8 = {};
                                
                                for_s32(ambient_index, 8)
                                {
                                    Vec3_S64 ambient_offset = ambient_offsets[dir_index][ambient_index];
                                    Vec3_S64 ambient_p = neighbor_p + ambient_offset;
                                    Block_Type ambient_type = get_block_in_neighborhood(neighborhood, ambient_p).type;
                                    if (!is_translucent(ambient_type))
                                    {
                                        face->Ambient8 += ambient_values[ambient_index];
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    
    
    center->mesh_is_empty = (!(*first_opaque_mesh) && !(*first_transparent_mesh));
    
    
    Fence_ReadWrite();
    center->mesh_is_valid = true;
    for_s32(hood_index, array_count(neighborhood->all))
    {
        decrement_chunk_lock(neighborhood->all[hood_index]);
    }
}
