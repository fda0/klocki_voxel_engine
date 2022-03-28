#include "klocki_platform_interface.h"



struct Block
{
    Block_Type type;
};

struct Mesh_Face
{
    u32 Normal3_TexId14_Pos15;
    u32 Ambient8;
};


struct Mesh
{
#define Mesh_Face_Count (512) // @todo @test @speed test for realistic-optimal size?
    Mesh_Face faces[Mesh_Face_Count];
    s32 face_count;
    Mesh *next;
};


struct Sort_Mesh_Face
{
    Mesh_Face face;
    f32 dist_sq;
};




enum Chunk_Stage
{
    Chunk_Free,
    Chunk_ToFree,
    
    Chunk_InGeneration,
    
    Chunk_ToMesh,
    Chunk_InMeshing,
    Chunk_EmptyMesh,
    
    Chunk_InGpuUpload,
    Chunk_GpuUploaded,
};


enum Chunk_Priority_Stage
{
    ChunkPriority_None,
    
    ChunkPriority_ToMesh,
    ChunkPriority_InMeshing,
    ChunkPriority_InGpuUpload,
};


struct Chunk
{
    Vec3_S64 at;
    s64 distance_from_camera; // for sorting gpu uploaded chunks
    
    Block blocks[Chunk_Dim_Z][Chunk_Dim_Y][Chunk_Dim_X];
    b8 blocks_generated;
    b8 can_contain_blocks; // @todo add non air block counter instead?
    
    b8 mesh_is_valid;
    b8 mesh_is_empty;
    b8 mesh_uploaded_to_gpu;
    s64 initial_gpu_upload_timestamp;
    
    Mesh *temp_opaque_mesh;
    Gpu_Mesh_Buffer opaque_gpu_mesh;
    Gpu_Mesh_Buffer temp_opaque_gpu_mesh;
    
    Mesh *translucent_mesh;
    Mesh *temp_translucent_mesh;
    Gpu_Mesh_Buffer translucent_gpu_mesh;
    Gpu_Mesh_Buffer temp_translucent_gpu_mesh;
    
    Vec3_S64 transparecy_sorted_at_chunk;
    v3 transparency_sorted_at_rel_p;
    
    
    u64 blocks_version_frame;
    u64 mesh_version_frame;
    
    
    volatile b32 early_free;
    volatile s32 lock_counter;
    
    // used by main thread:
    Chunk_Stage stage;
    Chunk *next;
    Chunk *prev;
    
    Chunk_Priority_Stage priority_stage;
    Chunk *priority_next;
    Chunk *priority_prev;
};

struct Camera
{
    Vec3_S64 at_chunk;
    v3 p;
    v2 angle;
};

struct Chunk_Handles
{
#define Chunk_Min_X -16
#define Chunk_Min_Y -8
#define Chunk_Min_Z -16
#define Chunk_OnePast_X 16
#define Chunk_OnePast_Y 8
#define Chunk_OnePast_Z 16
#define Chunk_Count_X (Chunk_OnePast_X - Chunk_Min_X)
#define Chunk_Count_Y (Chunk_OnePast_Y - Chunk_Min_Y)
#define Chunk_Count_Z (Chunk_OnePast_Z - Chunk_Min_Z)
    
    Chunk *h[Chunk_Count_Z*Chunk_Count_Y*Chunk_Count_X];
};

struct Entity
{
    Vec3_S64 at_chunk;
    v3 half_dim;
    
    v3 p;
    v3 dp;
    
    v2 angle;
    
    b32 in_air;
    b32 updated;
};

struct World
{
    s32 held_block_id;
    Entity player;
    s64 timestamp_queued_jump;
    b32 fly_mode;
    
    Entity entities[255];
    s32 entity_count;
    
    Camera camera;
    b32 in_debug_camera;
    Camera debug_camera;
    
    b32 initialized;
    Vec3_S64 world_at_camera_chunk;
    Vec3_S64 chunks_handles_min_corner;
    Chunk_Handles chunk_handles;
    
    Vec3_S64 waiting_for_generation[Chunk_Count_Z*Chunk_Count_Y*Chunk_Count_X];
    s32 waiting_for_generation_count;
    
    
    // chunk->next & chunk->prev sentinels
    Chunk in_generation_sentinel;
    Chunk to_mesh_sentinel;
    Chunk in_meshing_sentinel;
    Chunk empty_mesh_sentinel;
    Chunk in_gpu_upload_sentinel;
    Chunk gpu_uploaded_sentinel;
    
    Chunk to_free_sentinel;
    Chunk free_sentinel;
    Chunk chunk_slots[4096*7];
    
    // chunk->priority_next & chunk->priority_prev sentinels
    Chunk priority_to_mesh_sentinel;
    Chunk priority_in_meshing_sentinel;
    Chunk priority_in_gpu_upload_sentinel;
    
    
    
    Arena mesh_arena;
    Mesh *first_free_mesh;
};










//~
#define Extended_Neighbors_Count (27)
static_global Vec3_S64 extended_offset_map[Extended_Neighbors_Count] =
{
    {-1,-1,-1}, {-1,-1, 0}, {-1,-1,+1},
    {-1, 0,-1}, {-1, 0, 0}, {-1, 0,+1},
    {-1,+1,-1}, {-1,+1, 0}, {-1,+1,+1},
    
    { 0,-1,-1}, { 0,-1, 0}, { 0,-1,+1},
    { 0, 0,-1}, { 0, 0, 0}, { 0, 0,+1},
    { 0,+1,-1}, { 0,+1, 0}, { 0,+1,+1},
    
    {+1,-1,-1}, {+1,-1, 0}, {+1,-1,+1},
    {+1, 0,-1}, {+1, 0, 0}, {+1, 0,+1},
    {+1,+1,-1}, {+1,+1, 0}, {+1,+1,+1},
};


struct Chunk_Neighborhood
{
    Chunk *all[Extended_Neighbors_Count];
};

static_function Chunk *
get_center_chunk(Chunk_Neighborhood *neighborhood)
{
    return neighborhood->all[13];
}





struct Render_Chunk_Payload
{
    Chunk *chunk;
    
    Mesh *opaque_mesh;
    Vert_Voxel *opaque_verts;
    u32 *opaque_face_array;
    
    Mesh *translucent_mesh;
    Vert_Voxel *translucent_verts;
    u32 *translucent_face_array;
};



enum Job_Type
{
    Job_MeshResource,
    Job_Mesh,
    Job_Generate,
    Job_GpuUpload,
    
    Job_PriorityMesh,
    Job_PriorityGpuUpload,
    
    Job_Count
};

template <class T> struct Worker_Task
{
    T payload;
};

template <class T> struct Worker_Job
{
    u32 task_start;
    u32 task_end;
#define Task_Count (megabytes(1))
    Worker_Task<T> tasks[Task_Count];
    u32 main_task_end;
    u32 main_added_tasks_count;
};

enum Worker_Message_Type : u8
{
    WorkerMessage_Exit,
    
    WorkerMessage_SetTaskStart,
    WorkerMessage_SetTaskEnd,
};

struct Worker_Message
{
    Worker_Message_Type type;
    Job_Type job_type;
    u32 value;
};

struct Worker
{
    struct App_State *app;
    HANDLE semaphore;
    u32 batch_number;
    
    u32 message_start;
    volatile u32 main_message_end; // modified by main thread; read by worker
    Worker_Message messages[32];
    
    Worker_Job<Mesh *> job_mesh_resource;
    Worker_Job<Chunk_Neighborhood> job_mesh;
    Worker_Job<Chunk *> job_generate;
    Worker_Job<Render_Chunk_Payload> job_gpu_upload;
    
    Worker_Job<Chunk_Neighborhood> job_priority_mesh;
    Worker_Job<Render_Chunk_Payload> job_priority_gpu_upload;
    
    
    // accessed by main thread
    volatile b32 did_exit;
};

struct Job_System
{
    Checked_Array<Worker> workers;
    Checked_Array<HANDLE> handles;
    HANDLE semaphore;
    
    s32 next_worker_to_be_used;
    b32 stop_messages_sent;
    u32 stopped_threads_count;
    u32 restarts_count;
};







//~
struct App_State
{
    World world;
    
    u32 reload_counter;

    
    Pcg_Entropy rng;
    
    s32 tex_width;
    s32 tex_height;
    
    s32 player_speed;
    
    f32 true_dt;
    f32 dt;
    f32 at;
    u64 timestamp_now;
    
    f32 longest_true_dt;
    
    
    v2 text_at;
    
    u64 debug_last_mouse_move_frame;
    b8 debug_go_forward;
    b8 debug_text;
    b8 debug_text_entity;
    b8 debug_collision_range;
    b8 debug_collision_visualization;
    b8 debug_chunk_boundries;
    b8 debug_sorts_highlight;
    
    Job_System job_system;
};


struct Collision_Entity
{
    v3 p;
    v3 half_dim;
};


struct Collision_Wall
{
    f32 wall_x;
    v3 entity_p;
    v3 entity_move;
    f32 min_y, max_y, min_z, max_z;
    v3 normal;
};









//~
static_global Dir_Flags dir_index_to_dir_flag_map[DirIndex_Count] =
{
    DirFlag_Up,
    DirFlag_Down,
    DirFlag_North,
    DirFlag_South,
    DirFlag_East,
    DirFlag_West,
};

static_global Vec3_S64 dir_offset_map[6] =
{
    {0,+1,0}, // Up
    {0,-1,0}, // Down
    {0,0,+1}, // N
    {0,0,-1}, // S
    {+1,0,0}, // E
    {-1,0,0}, // W
};

static_function Vec3_S64
get_dir_offset(Dir_Index dir)
{
    switch(dir)
    {
        case DirIndex_Up: return {0,+1,0}; // Up
        case DirIndex_Down: return {0,-1,0}; // Down
        case DirIndex_North: return {0,0,+1}; // N
        case DirIndex_South: return {0,0,-1}; // S
        case DirIndex_East: return {+1,0,0}; // E
        case DirIndex_West: return {-1,0,0}; // W
    }
    
    assert(0);
    return {};
}

static_function Vec3_S64
get_dir_offset_s32(s32 dir)
{
    return get_dir_offset((Dir_Index)dir);
}









static_function Dir_Flags
dir_flag_from_dir_index(s32 index)
{
    assert(index >= 0 && index < array_count(dir_index_to_dir_flag_map));
    return dir_index_to_dir_flag_map[index];
}

static_global String
dir_name_from_dir_index(s32 index)
{
    String result = {};
    switch (index)
    {
        case DirIndex_Up:    result = "Up"_f0; break;
        case DirIndex_Down:  result = "Down"_f0; break;
        case DirIndex_North: result = "North"_f0; break;
        case DirIndex_South: result = "South"_f0; break;
        case DirIndex_East:  result = "East"_f0; break;
        case DirIndex_West:  result = "West"_f0; break;
        default:             result = "DirIndex?"_f0; break;
    }
    return result;
}


static_global Block_Type
map_s64_to_block_type(s64 a)
{
    s64 count = (Block_Count - 1);
    s64 wrapped = ((((a % count) + count) % count) + 1);
    Block_Type result = (Block_Type)wrapped;
    return result;
}



//~
static_function void
increment_chunk_lock(Chunk *chunk)
{
    atomic_increment_s32(&chunk->lock_counter);
}
static_function void
decrement_chunk_lock(Chunk *chunk)
{
    atomic_decrement_s32(&chunk->lock_counter);
}






