static_function Block *
safe_get_block(Chunk *chunk, s64 x, s64 y, s64 z)
{
    assert(x >= 0 && x < Chunk_Dim_X);
    assert(y >= 0 && y < Chunk_Dim_Y);
    assert(z >= 0 && z < Chunk_Dim_Z);
    Block *result = &chunk->blocks[z][y][x];
    return result;
}



static_function u32
chunk_handle_1d_index_from_3d_index(s64 index_x, s64 index_y, s64 index_z)
{
    u32 result = safe_truncate_to_u32((index_z*Chunk_Count_Y*Chunk_Count_X) +
                                      (index_y*Chunk_Count_X) +
                                      index_x);
    assert(result < (sizeof(Chunk_Handles) / sizeof(Chunk *)));
    return result;
}

static_function u32
chunk_handle_1d_index_from_3d_index(Vec3_S64 index)
{
    u32 result = chunk_handle_1d_index_from_3d_index(index.x, index.y, index.z);
    return result;
}

static_function Vec3_S64
chunk_handle_3d_index_from_1d_index(u32 index)
{
    Vec3_S64 result =
    {
        index % Chunk_Count_X,
        (index / Chunk_Count_X) % Chunk_Count_Y,
        (index / (Chunk_Count_X*Chunk_Count_Y))
    };
    return result;
}


static_function Chunk **
get_chunk_slot_address(World *world, Vec3_S64 at_chunk)
{
    Chunk **result = nullptr;
    Vec3_S64 chunk_index = at_chunk - world->chunks_handles_min_corner;
    
    if (chunk_index.x >= 0 && chunk_index.x < Chunk_Count_X &&
        chunk_index.y >= 0 && chunk_index.y < Chunk_Count_Y &&
        chunk_index.z >= 0 && chunk_index.z < Chunk_Count_Z)
    {
        u32 index = chunk_handle_1d_index_from_3d_index(chunk_index);
        result = &world->chunk_handles.h[index];
    }
    
    return result;
}

static_function Chunk **
get_chunk_slot_address(World *world, s64 chunk_x, s64 chunk_y, s64 chunk_z)
{
    return get_chunk_slot_address(world, vec3_s64(chunk_x, chunk_y, chunk_z));
}


static_function Chunk *
get_chunk_slot(World *world, Vec3_S64 at_chunk)
{
    Chunk *result = {};
    Vec3_S64 chunk_index = at_chunk - world->chunks_handles_min_corner;
    
    if (chunk_index.x >= 0 && chunk_index.x < Chunk_Count_X &&
        chunk_index.y >= 0 && chunk_index.y < Chunk_Count_Y &&
        chunk_index.z >= 0 && chunk_index.z < Chunk_Count_Z)
    {
        u32 index = chunk_handle_1d_index_from_3d_index(chunk_index);
        result = world->chunk_handles.h[index];
    }
    
    return result;
}

static_function Chunk *
get_chunk_slot(World *world, s64 chunk_x, s64 chunk_y, s64 chunk_z)
{
    return get_chunk_slot(world, vec3_s64(chunk_x, chunk_y, chunk_z));
}







struct Block_Result
{
    Block block;
    b32 loaded_chunk;
};
static_function Block_Result
get_block_result_at_world_coord(World *world, s64 world_x, s64 world_y, s64 world_z)
{
    Block_Result result = {};
    
    s64 chunk_x = (world_x >> Chunk_Dim_Power_X); 
    s64 chunk_y = (world_y >> Chunk_Dim_Power_Y); 
    s64 chunk_z = (world_z >> Chunk_Dim_Power_Z); 
    Vec3_S64 at_chunk = vec3_s64(chunk_x, chunk_y, chunk_z);
    
    Chunk *chunk_slot = get_chunk_slot(world, at_chunk);
    if (chunk_slot && chunk_slot->blocks_generated)
    {
        assert(chunk_slot->at == at_chunk);
        s64 x = (world_x & (Chunk_Dim_X - 1));
        s64 y = (world_y & (Chunk_Dim_Y - 1));
        s64 z = (world_z & (Chunk_Dim_Z - 1));
        result.block = chunk_slot->blocks[z][y][x];
        result.loaded_chunk = true;
    }
    
    return result;
}

static_function Block_Result
get_block_result_at_world_coord(World *world, Vec3_S64 at_world)
{
    return get_block_result_at_world_coord(world, at_world.x, at_world.y, at_world.z);
}





static_function Block
get_block_in_neighborhood(Chunk_Neighborhood *neighborhood, Vec3_S64 p)
{
    Vec3_S64 overflow = {};
    
    if (p.x < 0) {
        overflow.x -= 1;
        p.x += Chunk_Dim_X;
    }
    else if (p.x >= Chunk_Dim_X) {
        overflow.x += 1;
        p.x -= Chunk_Dim_X;
    }
    
    if (p.y < 0) {
        overflow.y -= 1;
        p.y += Chunk_Dim_Y;
    }
    else if (p.y >= Chunk_Dim_Y) {
        overflow.y += 1;
        p.y -= Chunk_Dim_Y;
    }
    
    if (p.z < 0) {
        overflow.z -= 1;
        p.z += Chunk_Dim_Z;
    }
    else if (p.z >= Chunk_Dim_Z) {
        overflow.z += 1;
        p.z -= Chunk_Dim_Z;
    }
    
    
    u64 neighbor_index = (overflow.x + 1)*9 + (overflow.y + 1)*3 + (overflow.z + 1);
    if (neighbor_index < array_count(neighborhood->all))
    {
        Chunk *chunk = neighborhood->all[neighbor_index];
        Block result = chunk->blocks[p.z][p.y][p.x];
        return result;
    }
    else
    {
        assert(0);
        return {};
    }
}


static_function b32
is_translucent(Block_Type type)
{
    switch (type)
    {
        case Block_Air:
        case Block_OakLeaves:
        case Block_Glass:
        case Block_Ice:
        case Block_Honey:
        case Block_Water:
        case Block_TestDirTranslucent:
        return true;
    }
    
    return false;
}

static_function b32
is_solid(Block_Type type)
{
    switch (type)
    {
        case Block_Air:
        case Block_Water:
        return false;
    }
    
    return true;
}









static_function Vec3_S64
rel_coord_floor_to_world_coord(Vec3_S64 at_chunk, v3 rel_p)
{
    Vec3_S64 world_result = {
        (Chunk_Dim_X * at_chunk.x) + (s64)floor_f32_to_s32(rel_p.x),
        (Chunk_Dim_Y * at_chunk.y) + (s64)floor_f32_to_s32(rel_p.y),
        (Chunk_Dim_Z * at_chunk.z) + (s64)floor_f32_to_s32(rel_p.z),
    };
    return world_result;
}

static_function Vec3_S64
combine_chunk_and_rel_coord_to_world_coord(Vec3_S64 at_chunk, Vec3_S64 rel_p)
{
    Vec3_S64 world_result = hadamard(at_chunk, Chunk_Dim_Vec3_S64) + rel_p;
    return world_result;
}














static_function v3
get_world_to_camera_chunk_offset(World *world, Vec3_S64 thing_at_chunk)
{
    Vec3_S64 at_chunk_rel_to_camera = thing_at_chunk - world->camera.at_chunk;
    v3 chunk_offset = v3_from_vec3_s64(hadamard(at_chunk_rel_to_camera, Chunk_Dim_Vec3_S64));
    return chunk_offset;
}

static_function Camera *
get_active_camera(World *world)
{
    Camera *camera = (world->in_debug_camera ? &world->debug_camera : &world->camera);
    return camera;
}


