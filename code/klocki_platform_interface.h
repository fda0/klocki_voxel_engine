#pragma once
#include "stf0.h"
#include "stf0_rng.h"


#define db debug_break()
#define ba(expr) break_at(expr);


// Compile parameters
#define Def_SuperSlow 0 // more testing


#if Def_Internal || Def_Slow
#else
#  if Def_SuperSlow
#    error "Def_SuperSlow without Def_Internal or Def_Slow? Are you sure?"
#  endif
#endif


// Chunk dim parameters
#define Chunk_Dim_X (32)
#define Chunk_Dim_Y (32)
#define Chunk_Dim_Z (32)
#define Chunk_Dim_Power_X (5)
#define Chunk_Dim_Power_Y (5)
#define Chunk_Dim_Power_Z (5)

#define Chunk_Dim_Vec3_S64 {Chunk_Dim_X, Chunk_Dim_Y, Chunk_Dim_Z}

// Renderer parameters
#define Render_Buffered_Frames 3
#define Block_Indices_Per_Face (6)
#define Block_Verts_Per_Face (4)



#include "klocki_shared.h"
#include "plat_win32_helpers.h"
#include "klocki_profile.h"



#define Block_Side (16)

#define Glyph_Atlas_X (320)
#define Glyph_Atlas_Y (192)
#define Glyph_Side (32)

#define Default_Text_Scale (1.f)
#define Default_Text_Color (get_color(1.f, 0.8f, 0.2f))


enum Dir_Flags
{
    DirFlag_Up    = (1 << 0),
    DirFlag_Down  = (1 << 1),
    DirFlag_North = (1 << 2),
    DirFlag_South = (1 << 3),
    DirFlag_East  = (1 << 4),
    DirFlag_West  = (1 << 5),
};

enum Dir_Index
{
    DirIndex_Up,
    DirIndex_Down,
    DirIndex_North,
    DirIndex_South,
    DirIndex_East,
    DirIndex_West,
    
    DirIndex_Count
};




union Block_Tex_Ids
{
    struct
    {
        u16 up;
        u16 down;
        u16 north;
        u16 east;
        u16 south;
        u16 west;
    };
    u16 dirs[DirIndex_Count];
};

struct To_Load_Texture
{
    String file_name;
    u32 texture_id;
};


#include "metadesk_generated.h"



struct Platform_Data
{
    s32 tls_index;
};


struct Thread_Context
{
    Arena frame_arena; // life/scope: [update_and_render -> opengl_render_commands]
    Arena scratch_arenas[2];
};



struct Input_Event
{
    b32 mouse_left;
    
    Input_Event *next;
    Input_Event *prev;
};

struct Button
{
    b8 down;
    b8 was_down;
};


struct Input
{
    Input_Event event_sentinel;
    u32 event_count;
    
    v2 mouse_abs;
    v2 mouse_rel;
    s32 mouse_wheel;
    
    union
    {
        Button buttons[64];
        struct
        {
            Button mouse_left, mouse_right, mouse_middle, mouse_x1, mouse_x2;
            Button jump, sneak;
            Button forward, backward, left, right;
            Button cam_up, cam_down, cam_left, cam_right;
            Button f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11;
            Button n1, n2, n3, n4, n5, n6, n7, n8, n9, n0;
        };
    };
};



struct Vert_Voxel
{
    u8 packed;
};
struct Frame_Voxel
{
    Vert_Voxel *verts; // {ambient(2b), pos_offset(3b)}
    u32 *face_array;  // {normal(3b), tex_id(14b) pos(15b)}
    s32 face_count;
    
    v3 mesh_translate;
};


struct Vert_Ui
{
    v3 p;
    v2 tex;
    u32 texid14_color12;
};
struct Frame_Ui
{
    Vert_Ui *verts;
    s32 face_count;
    s32 face_index;
};

struct Vert_Debug
{
    v3 p;
    u32 rgb18_texid8_texpos2;
};
struct Frame_Debug
{
    Vert_Debug *verts;
    s32 face_count;
    s32 face_index;
};


struct Vert_Line
{
    v3 p;
    v3 color;
};
struct Frame_Line
{
    Vert_Line *verts;
    s32 line_max_count;
    s32 line_index;
};




enum Atlas_Target
{
    Atlas_Font,
    Atlas_Blocks,
};


struct Gpu_Mesh_Buffer
{
    Frame_Voxel *frame;
    u32 mesh_id;
};

#define PLATFORM_CREATE_MESH_BUFFER(Name) Gpu_Mesh_Buffer Name(s32 face_count)
typedef PLATFORM_CREATE_MESH_BUFFER(Platform_Create_Mesh_Buffer);

#define PLATFORM_FINALIZE_MESH_BUFFER(Name) void Name(u32 mesh_id, b32 unmap_buffers)
typedef PLATFORM_FINALIZE_MESH_BUFFER(Platform_Finalize_Mesh_Buffer);

#define PLATFORM_DELETE_MESH_BUFFER(Name) void Name(u32 mesh_id)
typedef PLATFORM_DELETE_MESH_BUFFER(Platform_Delete_Mesh_Buffer);

#define PLATFORM_SET_WORLD_TRANSFORM(Name) void Name(m4x4 *mat, v3 camera_rel_p)
typedef PLATFORM_SET_WORLD_TRANSFORM(Platform_Set_World_Transform);

#define PLATFORM_RENDER_MESH_BUFFER(Name) void Name(u32 mesh_id, f32 mesh_age_t)
typedef PLATFORM_RENDER_MESH_BUFFER(Platform_Render_Mesh_Buffer);


#define PLATFORM_CREATE_TEXTURE_2D(Name) void Name(Atlas_Target target, s32 width, s32 height, u32 *memory)
typedef PLATFORM_CREATE_TEXTURE_2D(Platform_Create_Texture_2d);

#define PLATFORM_CREATE_TEXTURE_3D(Name) void Name(Atlas_Target target)
typedef PLATFORM_CREATE_TEXTURE_3D(Platform_Create_Texture_3d);

#define PLATFORM_FILL_TEXTURE_3D(Name) void Name(Atlas_Target target, s32 depth_index, u32 *memory)
typedef PLATFORM_FILL_TEXTURE_3D(Platform_Fill_Texture_3d);

#define PLATFORM_GENERATE_MIPMAP_3D(Name) void Name(Atlas_Target target)
typedef PLATFORM_GENERATE_MIPMAP_3D(Platform_Generate_Mipmap_3d);


#define PLATFORM_SYNC_FRAME_RENDERING(Name) void Name(u32 render_buffer_index)
typedef PLATFORM_SYNC_FRAME_RENDERING(Platform_Sync_Frame_Rendering);

struct Platform_Exports
{
    Platform_Create_Mesh_Buffer *create_mesh_buffer;
    Platform_Finalize_Mesh_Buffer *finalize_mesh_buffer;
    Platform_Delete_Mesh_Buffer *delete_mesh_buffer;
    Platform_Set_World_Transform *set_world_transform;
    Platform_Render_Mesh_Buffer *render_mesh_buffer;
    
    Platform_Create_Texture_2d *create_texture_2d;
    Platform_Create_Texture_3d *create_texture_3d;
    Platform_Fill_Texture_3d *fill_texture_3d;
    Platform_Generate_Mipmap_3d *generate_mipmap_3d;
    
    Platform_Sync_Frame_Rendering *sync_frame_rendering;
};

struct Debug_Timing
{
    s64 time;
    String text;
};

struct Debug_Timing_Frame
{
    Debug_Timing timings[64];
    u32 timing_count;
};

struct Debug_Chunk_Sort_Highlight
{
    s64 timestamp;
    Vec3_S64 at_chunk;
};

struct Debug_Stats
{
    u64 game_quads_last_frame;
    
    Debug_Timing_Frame timing_frames[2];
    u32 active_timing_frame;
    
    u64 total_vram_buffers_size;
    
    u64 total_chunk_sorts_count;
    Debug_Chunk_Sort_Highlight sort_highlights[1024];
    s32 sort_highlight_start;
    s32 sort_highlight_one_past_last;
};


struct Platform_Interface
{
    // app read-write
    Arena perm_arena;
    void *app_state_initialized;
    
    // app read
    u64 frame_number;
    b32 dll_reload_pending;
    b32 dll_reload_unhandled;
    
    // app write
    b32 dll_reload_app_ready;
    b32 mouse_in_locked_mode;
    
    
    // render stuff
    u32 render_buffer_index;
    b32 debug_show_wireframe;
    Frame_Ui *frame_ui;
    Frame_Debug *frame_debug;
    Frame_Line *frame_line;
    
    
    // app read
    Input input;
    Platform_Data platform_data;
    
    b32 is_window_active;
    s32 window_width, window_height;
    f32 inv_perf_freq;
    
    Platform_Exports api;
    Debug_Stats stats;
    
#if Def_Profile
    PerformanceAPI_Functions dapi;
#endif
};





//~
#define APP_UPDATE(Name) void Name(Platform_Interface *interface)
typedef APP_UPDATE(App_Update);

struct App_Exports
{
    App_Update *app_update;
};

#define EXPORT_APP_API(Name) App_Exports Name()
typedef EXPORT_APP_API(Export_App_Api);



//~
static_function b32
debug_eq(f32 a, f32 b)
{
    f32 epsilon = 0.0001f;
    b32 result = ((a - epsilon) <= b &&
                  (a + epsilon) >= b);
    return result;
}

static_function b32
debug_eq(m4x4 a, m4x4 b)
{
    for_s32(i, 16)
    {
        if (!debug_eq(a.flat[i], b.flat[i])) return false;
    }
    
    return true;
}

