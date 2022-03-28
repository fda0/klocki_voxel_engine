static_function s32
count_meshes_in_linked_list(Mesh *first_mesh)
{
    s32 face_count = 0;
    for (Mesh *mesh = first_mesh;
         mesh;
         mesh = mesh->next)
    {
        face_count += mesh->face_count;
    }
    return face_count;
}



static_function void
render_chunk(Mesh *first_mesh, Vert_Voxel *vert_array, u32 *face_array)
{
    profile_function();
    assert(first_mesh && vert_array && face_array);
    
    
    u8 mh = 0; // minus half cube dim
    u8 h = 1; // half cube dim
    
    u8 opposite_side[DirIndex_Count] =
    {
        DirIndex_Down,
        DirIndex_Up,
        DirIndex_South,
        DirIndex_North,
        DirIndex_West,
        DirIndex_East
    };
    
    u8 verts[DirIndex_Count][Block_Verts_Per_Face];
    verts[DirIndex_Up][0] = {pack_pos3( h,  h, mh)};
    verts[DirIndex_Up][1] = {pack_pos3(mh,  h, mh)};
    verts[DirIndex_Up][2] = {pack_pos3( h,  h,  h)};
    verts[DirIndex_Up][3] = {pack_pos3(mh,  h,  h)};

    verts[DirIndex_Down][0] = {pack_pos3(mh, mh, mh)};
    verts[DirIndex_Down][1] = {pack_pos3( h, mh, mh)};
    verts[DirIndex_Down][2] = {pack_pos3(mh, mh,  h)};
    verts[DirIndex_Down][3] = {pack_pos3( h, mh,  h)};
    
    verts[DirIndex_North][0] = {pack_pos3(mh, mh,  h)};
    verts[DirIndex_North][1] = {pack_pos3( h, mh,  h)};
    verts[DirIndex_North][2] = {pack_pos3(mh,  h,  h)};
    verts[DirIndex_North][3] = {pack_pos3( h,  h,  h)};
    
    verts[DirIndex_South][0] = {pack_pos3( h, mh, mh)};
    verts[DirIndex_South][1] = {pack_pos3(mh, mh, mh)};
    verts[DirIndex_South][2] = {pack_pos3( h,  h, mh)};
    verts[DirIndex_South][3] = {pack_pos3(mh,  h, mh)};
    
    verts[DirIndex_East][0] = {pack_pos3( h, mh,  h)};
    verts[DirIndex_East][1] = {pack_pos3( h, mh, mh)};
    verts[DirIndex_East][2] = {pack_pos3( h,  h,  h)};
    verts[DirIndex_East][3] = {pack_pos3( h,  h, mh)};
    
    verts[DirIndex_West][0] = {pack_pos3(mh, mh, mh)};
    verts[DirIndex_West][1] = {pack_pos3(mh, mh,  h)};
    verts[DirIndex_West][2] = {pack_pos3(mh,  h, mh)};
    verts[DirIndex_West][3] = {pack_pos3(mh,  h,  h)};
    
    s32 indices_index = 0;
    s32 vert_index = 0;
    s32 face_index = 0;
    
    {
        for (Mesh *mesh = first_mesh;
             mesh;
             mesh = mesh->next)
        {
            for_s32(mesh_face_index, mesh->face_count)
            {
                Mesh_Face face = mesh->faces[mesh_face_index];
                
                u32 *face_data = face_array + face_index;
                face_index += 1;
                
                Vert_Voxel *vertex_data = vert_array + vert_index;
                vert_index += Block_Verts_Per_Face;
                
                
                //
                *face_data = face.Normal3_TexId14_Pos15;
                
                
                u32 normal3 = (face.Normal3_TexId14_Pos15 >> 29);
                assert(normal3 >= 0 && normal3 < DirIndex_Count);
                
                auto v = &verts[normal3][0];
                vertex_data[0] = {(u8)(v[0] | (((face.Ambient8     ) & Bitmask_2) << 3))};
                vertex_data[1] = {(u8)(v[1] | (((face.Ambient8 >> 2) & Bitmask_2) << 3))};
                vertex_data[2] = {(u8)(v[2] | (((face.Ambient8 >> 4) & Bitmask_2) << 3))};
                vertex_data[3] = {(u8)(v[3] | (((face.Ambient8 >> 6) & Bitmask_2) << 3))};
            }
        }
    }
}

static_function void
render_chunk(Render_Chunk_Payload *data)
{
    assert(data->opaque_mesh || data->translucent_mesh);
    if (data->opaque_mesh)
    {
        render_chunk(data->opaque_mesh, data->opaque_verts, data->opaque_face_array);
    }
    if (data->translucent_mesh)
    {
        render_chunk(data->translucent_mesh, data->translucent_verts, data->translucent_face_array);
    }
}







static_function void
render_entity(Frame_Debug *frame, World *world, Entity *entity, v4 color = V4(1))
{
    u32 rgb = pack_color18(color);
    u32 texture_id_all = 0;
    u32 packed_all = (rgb << 10 | (texture_id_all << 2));
    u32 texture_id_south = 20;
    u32 packed_south = (rgb << 10 | (texture_id_south << 2));
    
    
    v3 chunk_offset = get_world_to_camera_chunk_offset(world, entity->at_chunk);
    
    v3 p = entity->p + chunk_offset;
    v3 pos_r = entity->half_dim;
    v3 neg_r = -entity->half_dim;
    
    v3 r_array[8] =
    {
        {neg_r.x, neg_r.y, neg_r.z},
        {neg_r.x, neg_r.y, pos_r.z},
        {neg_r.x, pos_r.y, neg_r.z},
        {neg_r.x, pos_r.y, pos_r.z},
        
        {pos_r.x, neg_r.y, neg_r.z},
        {pos_r.x, neg_r.y, pos_r.z},
        {pos_r.x, pos_r.y, neg_r.z},
        {pos_r.x, pos_r.y, pos_r.z},
    };
    
    f32 angle = entity->angle.x;
    
    if (angle)
    {
        m4x4 rot = y_rotation(angle);
        for_s32(r_index, array_count(r_array))
        {
            r_array[r_index] = transform(rot, V4(r_array[r_index], 0)).xyz;
        }
    }
    
    for_s32(r_index, array_count(r_array))
    {
        r_array[r_index] += p;
    }
    
    
    Vert_Debug verts[DirIndex_Count][Block_Verts_Per_Face];
    verts[DirIndex_Up][0] = {r_array[0b110], packed_all|0b00};
    verts[DirIndex_Up][1] = {r_array[0b010], packed_all|0b01};
    verts[DirIndex_Up][2] = {r_array[0b111], packed_all|0b10};
    verts[DirIndex_Up][3] = {r_array[0b011], packed_all|0b11};
    verts[DirIndex_Down][0] = {r_array[0b000], packed_all|0b00};
    verts[DirIndex_Down][1] = {r_array[0b100], packed_all|0b01};
    verts[DirIndex_Down][2] = {r_array[0b001], packed_all|0b10};
    verts[DirIndex_Down][3] = {r_array[0b101], packed_all|0b11};
    verts[DirIndex_North][0] = {r_array[0b001], packed_all|0b00};
    verts[DirIndex_North][1] = {r_array[0b101], packed_all|0b01};
    verts[DirIndex_North][2] = {r_array[0b011], packed_all|0b10};
    verts[DirIndex_North][3] = {r_array[0b111], packed_all|0b11};
    verts[DirIndex_South][0] = {r_array[0b100], packed_south|0b00};
    verts[DirIndex_South][1] = {r_array[0b000], packed_south|0b01};
    verts[DirIndex_South][2] = {r_array[0b110], packed_south|0b10};
    verts[DirIndex_South][3] = {r_array[0b010], packed_south|0b11};
    verts[DirIndex_East][0] = {r_array[0b000], packed_all|0b00};
    verts[DirIndex_East][1] = {r_array[0b001], packed_all|0b01};
    verts[DirIndex_East][2] = {r_array[0b010], packed_all|0b10};
    verts[DirIndex_East][3] = {r_array[0b011], packed_all|0b11};
    verts[DirIndex_West][0] = {r_array[0b101], packed_all|0b00};
    verts[DirIndex_West][1] = {r_array[0b100], packed_all|0b01};
    verts[DirIndex_West][2] = {r_array[0b111], packed_all|0b10};
    verts[DirIndex_West][3] = {r_array[0b110], packed_all|0b11};
    
    
    
    s32 face_count = 6;
    if (frame->face_index + face_count > frame->face_count)
    {
        frame->face_index -= face_count;
        assert(0);
    }
    
    for_s32(dir_index, DirIndex_Count)
    {
        Vert_Debug *v = verts[dir_index];
        
        Vert_Debug *target = frame->verts + frame->face_index*Block_Verts_Per_Face;
        frame->face_index += 1;
        
        target[0] = v[0];
        target[1] = v[1];
        target[2] = v[2];
        target[3] = v[3];
    }
}


static_function void
render_line(Frame_Line *frame, World *world, Vec3_S64 at_chunk, v3 from_p, v3 to_p, v4 color = V4(1))
{
    v3 chunk_offset = get_world_to_camera_chunk_offset(world, at_chunk);
    
    Vert_Line *target = frame->verts + frame->line_index*2;
    frame->line_index += 1;
    target[0] = {from_p + chunk_offset, color.xyz};
    target[1] = {to_p + chunk_offset, color.xyz};
}


static_function void
render_cube_lines(Frame_Line *frame, World *world, Vec3_S64 at_chunk, v3 from_p, v3 to_p, v4 color = V4(1))
{
    v3 chunk_offset = get_world_to_camera_chunk_offset(world, at_chunk);
    
    from_p += chunk_offset;
    to_p += chunk_offset;
    
    v3 r_array[8] =
    {
        {from_p.x, from_p.y, from_p.z},
        {from_p.x, from_p.y, to_p.z},
        {from_p.x, to_p.y, from_p.z},
        {from_p.x, to_p.y, to_p.z},
        
        {to_p.x, from_p.y, from_p.z},
        {to_p.x, from_p.y, to_p.z},
        {to_p.x, to_p.y, from_p.z},
        {to_p.x, to_p.y, to_p.z},
    };
    
    v3 points[24];
    points[0] = r_array[0b000]; // down
    points[1] = r_array[0b100];
    points[2] = r_array[0b100];
    points[3] = r_array[0b101];
    points[4] = r_array[0b101];
    points[5] = r_array[0b001];
    points[6] = r_array[0b001];
    points[7] = r_array[0b000];
    points[8] = r_array[0b001]; // sides
    points[9] = r_array[0b011];
    points[10] = r_array[0b000];
    points[11] = r_array[0b010];
    points[12] = r_array[0b100];
    points[13] = r_array[0b110];
    points[14] = r_array[0b101];
    points[15] = r_array[0b111];
    points[16] = r_array[0b010]; // up
    points[17] = r_array[0b110];
    points[18] = r_array[0b110];
    points[19] = r_array[0b111];
    points[20] = r_array[0b111];
    points[21] = r_array[0b011];
    points[22] = r_array[0b011];
    points[23] = r_array[0b010];
    
    for (s32 point_index = 0;
         point_index < 24;
         point_index += 2)
    {
        Vert_Line *target = frame->verts + frame->line_index*2;
        if (frame->line_index < frame->line_max_count)
        {
            frame->line_index += 1;
            assert(frame->line_max_count >= frame->line_index);
            target[0] = {points[point_index], color.xyz};
            target[1] = {points[point_index + 1], color.xyz};
        }
    }
}

