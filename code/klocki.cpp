#pragma comment(lib, "User32.lib")
#include "klocki.h"

#if Def_Ship
#else
static_global Platform_Interface *plat = {};
#include "klocki_shared.cpp"
#endif

#include "plat_win32_thread_context.cpp"
#include "stf0_memory.h"
#include "plat_win32_helpers.cpp"


#include "klocki_math.cpp"
#include "klocki_sorts.cpp"
#include "klocki_world.cpp"
#include "klocki_mesh.cpp"
#include "klocki_generate_world.cpp"
#include "klocki_render.cpp"
#include "klocki_world_render.cpp"



#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STBI_ONLY_PNG
#define STBI_ASSERT(x) assert(x)
//#define STBI_NO_STDIO
#include "stb_image.h"





static_function Debug_Chunk_Sort_Highlight *
push_debug_sort_highlight()
{
    Debug_Chunk_Sort_Highlight *highlight = plat->stats.sort_highlights + plat->stats.sort_highlight_one_past_last;
    
    plat->stats.sort_highlight_one_past_last = (plat->stats.sort_highlight_one_past_last + 1) % array_count(plat->stats.sort_highlights);
    if (plat->stats.sort_highlight_start == plat->stats.sort_highlight_one_past_last)
    {
        plat->stats.sort_highlight_start = (plat->stats.sort_highlight_start + 1) % array_count(plat->stats.sort_highlights);
    }
    
    return highlight;
}





static_function f32
exact_linear_to_srgb(f32 l)
{
    l = clamp01(l);
    f32 s = l*12.92f;
    
    if (l > 0.0031308f)
    {
        s = 1.055f*powf(l, 1.f/2.4f) - 0.055f;
    }
    
    return s;
}

static_function u32
color_v3_to_u32_bgra_exact(v3 color)
{
    u32 c255 = (((u32)(exact_linear_to_srgb(color.z)*255.f)) |
                ((u32)(exact_linear_to_srgb(color.y)*255.f) << 8) |
                ((u32)(exact_linear_to_srgb(color.x)*255.f) << 16) |
                (255 << 24));
    return c255;
}

static_function u32
color_v3_to_u32_bgra(v3 color)
{
    u32 c255 = (((u32)(color.z*255.f)) |
                ((u32)(color.y*255.f) << 8) |
                ((u32)(color.x*255.f) << 16) |
                (255 << 24));
    return c255;
}















static_function void
upload_mesh_to_gpu(Render_Chunk_Payload *data)
{
    profile_function();
    
    Chunk *chunk = data->chunk;
    if (chunk->early_free)
    {
        decrement_chunk_lock(chunk);
        return;
    }
    
    render_chunk(data);
    
    Fence_ReadWrite();
    chunk->mesh_uploaded_to_gpu = true;
    decrement_chunk_lock(chunk);
}



















static_function Worker_Message *
worker_get_new_message_slot(Worker *worker, u32 index = 0)
{
    u32 end_index = (worker->main_message_end + index) % array_count(worker->messages);
    Worker_Message *result = worker->messages + end_index;
    return result;
}

static_function void
worker_increment_message_end(Worker *worker, u32 count = 1)
{
#if Def_SuperSlow
    u32 old_end_index = worker->main_message_end;
#endif
    
    Fence_ReadWrite();
    assert(count >= 0);
    u32 new_end_index = (worker->main_message_end + count) % array_count(worker->messages);
    worker->main_message_end = new_end_index;
    
    
#if Def_SuperSlow
    // test for overflow
    assert(count == 0 ||
           new_end_index > old_end_index ||
           new_end_index < worker->message_start);
#endif
}



template <class T> static_function Worker_Task<T> *
worker_get_new_task_slot(Worker_Job<T> *job)
{
    u32 end_index = (job->main_task_end + job->main_added_tasks_count) % array_count(job->tasks);
    job->main_added_tasks_count += 1;
    
    auto result = job->tasks + end_index;
    return result;
}


template <class T> static_function b32
worker_send_message_increment_task_end(Worker *worker, Worker_Job<T> *job, Job_Type job_type)
{
    b32 result = false;
    
    if (job->main_added_tasks_count)
    {
        // @todo specifying both a job pointer and job type is messy and error prone. Simplify this system.
        assert(job->main_added_tasks_count < array_count(job->tasks));
        u32 new_end_index = (job->main_task_end + job->main_added_tasks_count) % array_count(job->tasks);
        job->main_task_end = new_end_index;
        job->main_added_tasks_count = 0;
        
        Worker_Message *message0 = worker_get_new_message_slot(worker);
        message0->type = WorkerMessage_SetTaskEnd;
        message0->job_type = job_type;
        message0->value = new_end_index;
        
        worker_increment_message_end(worker);
        result = true;
    }
    
    return result;
}






template <class T> static_function u32
worker_count_tasks(Worker_Job<T> *job)
{
    u32 task_start = job->task_start;
    u32 task_end = job->main_task_end;
    u32 count = 0;
    
    if (task_start < task_end)
    {
        count = task_end - task_start;
    }
    else if (task_start > task_end)
    {
        count = array_count(job->tasks) - task_start;
        count += task_end;
    }
    
    return count;
}











static_function void
worker_job_loop(Worker *worker)
{
    App_State *app = worker->app;
    World *world = &app->world;
    
    
    for (;;)
    {
        profile_scope("Worker loop");
        
        if (worker->message_start != worker->main_message_end)
        {
            Worker_Message *message = worker->messages + worker->message_start;
            worker->message_start = (worker->message_start + 1) % array_count(worker->messages);
            
            switch (message->type)
            {
                case WorkerMessage_Exit:
                {
                    worker->did_exit = true;
                    return;
                } break;
                
                case WorkerMessage_SetTaskStart:
                {
                    switch (message->job_type)
                    {
                        case Job_MeshResource:
                        {
                            auto job = &worker->job_mesh_resource;
                            job->task_start = message->value;
                        } break;
                        case Job_Mesh:
                        {
                            auto job = &worker->job_mesh;
                            job->task_start = message->value;
                        } break;
                        case Job_Generate:
                        {
                            auto job = &worker->job_generate;
                            job->task_start = message->value;
                        } break;
                        case Job_GpuUpload:
                        {
                            auto job = &worker->job_gpu_upload;
                            job->task_start = message->value;
                        } break;
                        case Job_PriorityMesh:
                        {
                            auto job = &worker->job_priority_mesh;
                            job->task_start = message->value;
                        } break;
                        case Job_PriorityGpuUpload:
                        {
                            auto job = &worker->job_priority_gpu_upload;
                            job->task_start = message->value;
                        } break;
                        Invalid_Default_Case;
                    }
                } break;
                
                case WorkerMessage_SetTaskEnd:
                {
                    switch (message->job_type)
                    {
                        case Job_MeshResource:
                        {
                            auto job = &worker->job_mesh_resource;
                            job->task_end = message->value;
                        } break;
                        case Job_Mesh:
                        {
                            auto job = &worker->job_mesh;
                            job->task_end = message->value;
                        } break;
                        case Job_Generate:
                        {
                            auto job = &worker->job_generate;
                            job->task_end = message->value;
                        } break;
                        case Job_GpuUpload:
                        {
                            auto job = &worker->job_gpu_upload;
                            job->task_end = message->value;
                        } break;
                        case Job_PriorityMesh:
                        {
                            auto job = &worker->job_priority_mesh;
                            job->task_end = message->value;
                        } break;
                        case Job_PriorityGpuUpload:
                        {
                            auto job = &worker->job_priority_gpu_upload;
                            job->task_end = message->value;
                        } break;
                        Invalid_Default_Case;
                    }
                } break;
                
                Invalid_Default_Case;
            }
        }
        else
        {
            b32 did_a_task = false;
            
            for_s32(job_loop, 2)
            {
                b32 did_task_this_loop = false;
                
                // priority jobs
                if (!did_task_this_loop)
                {
                    auto job = &worker->job_priority_gpu_upload;
                    if (job->task_start != job->task_end)
                    {
                        did_task_this_loop = true;
                        auto task = job->tasks + job->task_start;
                        job->task_start = (job->task_start + 1) % array_count(job->tasks);
                        
                        upload_mesh_to_gpu(&task->payload);
                    }
                }
                
                if (!did_task_this_loop)
                {
                    auto job = &worker->job_priority_mesh;
                    if (job->task_start != job->task_end)
                    {
                        did_task_this_loop = true;
                        auto task = job->tasks + job->task_start;
                        job->task_start = (job->task_start + 1) % array_count(job->tasks);
                        
                        Chunk_Neighborhood *neighborhood = &task->payload;
                        Chunk *center = get_center_chunk(neighborhood);
                        generate_chunk_mesh(neighborhood,
                                            &center->temp_opaque_mesh,
                                            &center->temp_translucent_mesh,
                                            &worker->job_mesh_resource);
                    }
                }
                
                
                
                // normal jobs
                if (!did_task_this_loop)
                {
                    auto job = &worker->job_gpu_upload;
                    if (job->task_start != job->task_end)
                    {
                        did_task_this_loop = true;
                        auto task = job->tasks + job->task_start;
                        job->task_start = (job->task_start + 1) % array_count(job->tasks);
                        
                        upload_mesh_to_gpu(&task->payload);
                    }
                }
                
                if (!did_task_this_loop)
                {
                    auto job = &worker->job_mesh;
                    if (job->task_start != job->task_end)
                    {
                        did_task_this_loop = true;
                        auto task = job->tasks + job->task_start;
                        job->task_start = (job->task_start + 1) % array_count(job->tasks);
                        
                        Chunk_Neighborhood *neighborhood = &task->payload;
                        Chunk *center = get_center_chunk(neighborhood);
                        generate_chunk_mesh(neighborhood,
                                            &center->temp_opaque_mesh,
                                            &center->translucent_mesh,
                                            &worker->job_mesh_resource);
                    }
                }
                
                if (!did_task_this_loop)
                {
                    auto job = &worker->job_generate;
                    if (job->task_start != job->task_end)
                    {
                        did_task_this_loop = true;
                        auto task = job->tasks + job->task_start;
                        job->task_start = (job->task_start + 1) % array_count(job->tasks);
                        generate_world_chunk(task->payload);
                        //generate_world_chunk_rng_test(task->payload);
                    }
                }
                
                
                
                did_a_task |= did_task_this_loop;
                
                if (!did_task_this_loop)
                {
                    break;
                }
            }
            
            
            
            if (!did_a_task)
            {
                WaitForSingleObjectEx(worker->semaphore, INFINITE, FALSE);
            }
        }
    }
}







static_function DWORD WINAPI
win32_worker_thread_func(LPVOID lp_param)
{
    Worker *worker = (Worker *)lp_param;
    worker_job_loop(worker);
    return 0;
}


static_function void
initialize_job_system(App_State *app, Job_System *job_system)
{
    s32 thread_count = pick_bigger(1, platform_get_core_count() - 1);
    job_system->workers.initialize(&plat->perm_arena, thread_count);
    job_system->handles.initialize(&plat->perm_arena, thread_count);
    
    job_system->semaphore = CreateSemaphoreExA(nullptr, 0, thread_count,
                                               nullptr, 0, SEMAPHORE_ALL_ACCESS);
    
    for_s32(thread_index, thread_count)
    {
        Worker *worker = job_system->workers.at(thread_index); // assumes zero initialized memory
        worker->app = app;
        worker->semaphore = job_system->semaphore;
    }
}



static_function void
start_job_system(Job_System *job_system)
{
    for_s32(thread_index, job_system->workers.count)
    {
        Worker *worker = job_system->workers.at(thread_index);
        worker->batch_number = job_system->restarts_count;
        worker->did_exit = false;
        
        HANDLE *handle = job_system->handles.at(thread_index);
        *handle = CreateThread(nullptr, 0, win32_worker_thread_func, worker, 0, 0/*thread id*/);
        assert((*handle) != INVALID_HANDLE_VALUE);
    }
    
    job_system->restarts_count += 1;
}


static_function void
stop_job_system(Job_System *job_system)
{
    if (!job_system->stop_messages_sent)
    {
        for_s32(thread_index, job_system->workers.count)
        {
            Worker *worker = job_system->workers.at(thread_index);
            Worker_Message *message = worker_get_new_message_slot(worker);
            message->type = WorkerMessage_Exit;
            worker_increment_message_end(worker);
        }
        
        job_system->stop_messages_sent = true;
    }
    
    
    u32 thread_count = safe_truncate_to_u32(job_system->handles.count);
    ReleaseSemaphore(job_system->semaphore, thread_count, nullptr);
    
    job_system->stopped_threads_count = 0;
    for_s32(thread_index, job_system->workers.count)
    {
        Worker *worker = job_system->workers.at(thread_index);
        if (worker->did_exit)
        {
            job_system->stopped_threads_count += 1;
        }
    }
    
    
    if (job_system->stopped_threads_count == thread_count)
    {
        plat->dll_reload_app_ready = true;
        
        job_system->stop_messages_sent = false;
        job_system->stopped_threads_count = 0;
        
        for_s32(thread_index, (s32)thread_count)
        {
            HANDLE *handle = job_system->handles.at(thread_index);
            b32 close_thread_res = CloseHandle(*handle);
            assert(close_thread_res);
        }
    }
}






static_function void
fast_text(App_State *app, String text,
          f32 scale = Default_Text_Scale, v4 color = Default_Text_Color)
{
    f32 glyph_side = (f32)Glyph_Side;
    
    for_u64(str_index, text.size)
    {
        u32 codepoint = text.str[str_index];
        if (codepoint >= 'a' && codepoint <= 'z') codepoint -= ('a' - 'A');
        
        if (is_whitespace(codepoint))
        {
            app->text_at.x += scale*glyph_side * (codepoint == '\t' ? 4.f : 1.f);
        }
        else if (is_end_of_line(codepoint))
        {
            app->text_at.x = 10.f;
            app->text_at.y += glyph_side*scale;
        }
        else
        {
            render_glyph(app->text_at, codepoint, scale, color);
            app->text_at.x += scale*glyph_side;
        }
    }
}

static_function void
fast_text_nl(App_State *app, String text,
             f32 scale = Default_Text_Scale, v4 color = Default_Text_Color)
{
    f32 glyph_side = (f32)Glyph_Side;
    
    fast_text(app, text, scale, color);
    app->text_at.y += glyph_side*scale;
    app->text_at.x = 10.f;
}


static_function void
fast_block_text(App_State *app, s32 block_type, f32 scale = Default_Text_Scale)
{
    f32 block_side = (f32)Block_Side;
    
    Block_Tex_Ids tex_ids = {};
    if (block_type >= 0 && block_type < array_count(map_block_to_tex_id))
    {
        tex_ids = map_block_to_tex_id[block_type];
    }
    else
    {
        assert(0);
    }
    
    v2 p = app->text_at;
    p.x = round_f32(p.x);
    p.y = round_f32(p.y);
    
    {
        Rect2 rect = rect_min_dim(p, V2(block_side*scale));
        Rect2 tex = rect_min_dim(V2(0), V2(1));
        
        Frame_Ui *frame = plat->frame_ui;
        Vert_Ui *verts = frame->verts + frame->face_index*Block_Verts_Per_Face;
        frame->face_index += 1;
        
        u32 color12 = pack_color12(get_white());
        u32 packed = (tex_ids.north << 12) | color12;
        
        verts[0] = {{rect.min.x, rect.min.y, 0}, {tex.min.x, tex.max.y}, packed};
        verts[1] = {{rect.min.x, rect.max.y, 0}, {tex.min.x, tex.min.y}, packed};
        verts[2] = {{rect.max.x, rect.min.y, 0}, {tex.max.x, tex.max.y}, packed};
        verts[3] = {{rect.max.x, rect.max.y, 0}, {tex.max.x, tex.min.y}, packed};
    }
    
    app->text_at.x += scale*block_side;
}








































static_function u32 *
crappy_image_load(Arena *a, char *path, s32 *out_cols, s32 *out_rows, b32 ignore_black = false)
{
    u32 *mem = nullptr;
    
    int rows, cols, comp;
    stbi_uc *stb_data = stbi_load(path, &cols, &rows, &comp, 4);
    *out_cols = cols;
    *out_rows = rows;
    
    if (stb_data)
    {
        if (rows && cols &&
            (comp == 4 || comp == 3))
        {
            mem = push_array(a, u32, rows*cols);
            
            if (comp == 4 || comp == 3)
            {
                u32 *target = mem;
                u32 *data = (u32 *)stb_data;
                
                for (s32 y = rows - 1;
                     y >= 0;
                     y -= 1)
                {
                    u32 *row = data + (y * cols);
                    
                    for_s32(x, cols)
                    {
                        u32 val = row[x];
                        if (ignore_black && val == 0xFF'00'00'00) { val = 0; }
                        
                        *target = ((val & ((255 << 8) | (255 << 24))) |
                                   ((val >> 16) & 255) |
                                   ((val & 255) << 16));
                        target += 1;
                    }
                }
            }
        }
        
        stbi_image_free(stb_data);
    }
    
    if (!mem)
    {
        char buffer[512];
        snprintf(buffer, sizeof(buffer), "Missing asset file: %s\n"
                 "Make sure to run the game from data/ directory", path);
        platform_throw_error_and_exit(buffer);
    }
    
    return mem;
}




static_function m4x4
rotation_from_direction(v3 dir, v3 up = V3(0, 1, 0))
{
    m4x4 result = identity();
    
    f32 dir_len_sq = get_length_sq(dir);
    if (dir_len_sq > 0.f)
    {
        dir *= (1.f / square_root(dir_len_sq));
        v3 X = normalize0(cross(up, dir));
        v3 Y = normalize0(cross(dir, X));
        
        result =
        {
            X.x, X.y, X.z, 0,
            Y.x, Y.y, Y.z, 0,
            dir.x, dir.y, dir.z, 0,
            0, 0, 0, 1,
        };
    }
    return result;
}







static_function void
debug_print(String text, v4 vec)
{
    Scratch scratch(0);
    char *str = cstrf(scratch, "%.*s: {%f, %f, %f, %f}\n",
                      string_expand(text), vec.x, vec.y, vec.z, vec.w);
    OutputDebugStringA(str);
}

static_function void
debug_print(String text, v3 vec)
{
    Scratch scratch(0);
    char *str = cstrf(scratch, "%.*s: {%f, %f, %f}\n",
                      string_expand(text), vec.x, vec.y, vec.z);
    OutputDebugStringA(str);
}

static_function void
debug_print(String text, f32 value)
{
    Scratch scratch(0);
    char *str = cstrf(scratch, "%.*s: %f\n", string_expand(text), value);
    OutputDebugStringA(str);
}

static_function void
debug_print(String text, m4x4 mat)
{
    Scratch scratch(0);
    char *str = cstrf(scratch, "%.*s: {\n"
                      "%f, %f, %f, %f\n"
                      "%f, %f, %f, %f\n"
                      "%f, %f, %f, %f\n"
                      "%f, %f, %f, %f}\n",
                      string_expand(text),
                      mat.e[0][0], mat.e[0][1], mat.e[0][2], mat.e[0][3],
                      mat.e[1][0], mat.e[1][1], mat.e[1][2], mat.e[1][3],
                      mat.e[2][0], mat.e[2][1], mat.e[2][2], mat.e[2][3],
                      mat.e[3][0], mat.e[3][1], mat.e[3][2], mat.e[3][3]);
    OutputDebugStringA(str);
}


static_function void
debug_app_tests(App_State *app)
{
#if Def_Internal
    assert(app);
    assert(is_non_zero_power_of_two(Chunk_Dim_X));
    assert(is_non_zero_power_of_two(Chunk_Dim_Y));
    assert(is_non_zero_power_of_two(Chunk_Dim_Z));
    runtime_assert((1 << Chunk_Dim_Power_X) == Chunk_Dim_X);
    runtime_assert((1 << Chunk_Dim_Power_Y) == Chunk_Dim_Y);
    runtime_assert((1 << Chunk_Dim_Power_Z) == Chunk_Dim_Z);
#endif
}











static_function b32
is_in_rectangle_epsilon(Rect3 rectangle, v3 test, f32 epsilon)
{
    b32 result = ((test.x >= rectangle.min.x - epsilon) && 
                  (test.y >= rectangle.min.y - epsilon) &&
                  (test.z >= rectangle.min.z - epsilon) &&
                  (test.x <= rectangle.max.x + epsilon) &&
                  (test.y <= rectangle.max.y + epsilon) &&
                  (test.z <= rectangle.max.z + epsilon));
    
    return result;
} 



struct World_Camera_Params
{
    v3 x, y, z;
    m4x4_inv mat;
};

static_function World_Camera_Params
get_world_camera_params(Camera *camera, f32 aspect_ratio)
{
    World_Camera_Params result = {};
    m4x4 camera_o = y_rotation(camera->angle.x)*x_rotation(camera->angle.y);
    result.x = get_column3(camera_o, 0);
    result.y = get_column3(camera_o, 1);
    result.z = get_column3(camera_o, 2);
    
    f32 focal_length = 0.78f;
    focal_length = 0.6f;
    m4x4_inv perspective_mat = perspective_projection(aspect_ratio, focal_length, 0.05f, 1000.f);
    m4x4_inv camera_mat = camera_transform(result.x, result.y, result.z, camera->p);
    result.mat.forward = perspective_mat.forward * camera_mat.forward;
    result.mat.inverse = camera_mat.inverse * perspective_mat.inverse;
    return result;
}

static_function m4x4_inv
get_world_camera_transform(Camera *camera, f32 aspect_ratio)
{
    return get_world_camera_params(camera, aspect_ratio).mat;
}




static_function void
normalize_world_position(Vec3_S64 *at_chunk, v3 *rel_p)
{
    s32 x = floor_f32_to_s32(rel_p->x);
    s32 y = floor_f32_to_s32(rel_p->y);
    s32 z = floor_f32_to_s32(rel_p->z);
    
    s32 chunk_x_delta = x / Chunk_Dim_X;
    s32 chunk_y_delta = y / Chunk_Dim_Y;
    s32 chunk_z_delta = z / Chunk_Dim_Z;
    
    if (rel_p->x < 0.f)
    {
        chunk_x_delta -= 1;
    }
    if (rel_p->y < 0.f)
    {
        chunk_y_delta -= 1;
    }
    if (rel_p->z < 0.f)
    {
        chunk_z_delta -= 1;
    }
    
    rel_p->x -= (f32)(chunk_x_delta*Chunk_Dim_X);
    rel_p->y -= (f32)(chunk_y_delta*Chunk_Dim_Y);
    rel_p->z -= (f32)(chunk_z_delta*Chunk_Dim_Z);
    
    at_chunk->x += chunk_x_delta;
    at_chunk->y += chunk_y_delta;
    at_chunk->z += chunk_z_delta;
}

static_function void
normalize_world_position(Entity *entity)
{
    normalize_world_position(&entity->at_chunk, &entity->p);
}

static_function void
normalize_world_position(Camera *camera)
{
    normalize_world_position(&camera->at_chunk, &camera->p);
}




struct Frustum_Value
{
    v3 normal;
    v4 color;
};

union Frustum
{
    struct
    {
        Frustum_Value top, bottom, left, right;
    };
    Frustum_Value e[4];
};



static_function void
render_world(App_State *app, World *world, f32 aspect_ratio)
{
    profile_function();
    
    Camera *camera = &world->camera;
    
    v3 chunk_dim = V3i(Chunk_Dim_X, Chunk_Dim_Y, Chunk_Dim_Z);
    v3 chunk_half_dim = 0.5f*chunk_dim;
    
    
    World_Camera_Params cam_params = get_world_camera_params(camera, aspect_ratio);
    f32 radius = square_root(3.f)*chunk_half_dim.x;
    
    Frustum frustum = {};
    {
        v3 row0 = cam_params.mat.forward.row[0].vec.xyz;
        v3 row1 = cam_params.mat.forward.row[1].vec.xyz;
        v3 row2 = cam_params.mat.forward.row[2].vec.xyz;
        v3 row3 = cam_params.mat.forward.row[3].vec.xyz;
        
        frustum.top.normal = row3 - row1;
        frustum.top.normal = normalize0(frustum.top.normal);
        frustum.top.color = get_white();
        
        frustum.bottom.normal = row3 + row1;
        frustum.bottom.normal = normalize0(frustum.bottom.normal);
        frustum.bottom.color = get_blue();
        
        frustum.left.normal = row3 + row0;
        frustum.left.normal = normalize0(frustum.left.normal);
        frustum.left.color = get_green();
        
        frustum.right.normal = row3 - row0;
        frustum.right.normal = normalize0(frustum.right.normal);
        frustum.right.color = get_red();
    }
    
    
    
    struct Defered_Mesh
    {
        u32 mesh_id;
        f32 mesh_age_t;
    };
    
    Scratch scratch(0);
    Checked_Array<Defered_Mesh> translucent_gpu_mesh_ids = create_checked_array<Defered_Mesh>(scratch);
    
    
#if Def_SuperSlow
    s64 debug_prev_dist_from_camera = 0;
#endif
    
    
    for_dll(chunk, &world->gpu_uploaded_sentinel)
    {
        assert(chunk);
#if Def_SuperSlow
        // test if chunks are sorted for front-to-back drawing
        assert(chunk->distance_from_camera >= debug_prev_dist_from_camera);
        debug_prev_dist_from_camera = chunk->distance_from_camera;
#endif
        
        Vec3_S64 at_chunk = chunk->at;
        v3 chunk_offset = get_world_to_camera_chunk_offset(world, at_chunk);
        v3 chunk_center = chunk_offset + chunk_half_dim;
        v3 chunk_center_rel_to_camera = chunk_center - camera->p;
        
        s32 in_frustum_count = 0;
        v4 color = V4(1);
        
        for_s32(frustum_index, array_count(frustum.e))
        {
            Frustum_Value fru = frustum.e[frustum_index];
            f32 dot = inner(chunk_center_rel_to_camera, fru.normal);
            
            f32 c = dot;
            if (c >= -radius)
            {
                in_frustum_count += 1;
            }
            else
            {
                break;
            }
        }
        
        
        if (in_frustum_count == 4)
        {
            f32 mesh_age_t = 2.f*time_elapsed(app->timestamp_now, chunk->initial_gpu_upload_timestamp);
            mesh_age_t = clamp01(mesh_age_t);
            
            
            assert(chunk->opaque_gpu_mesh.mesh_id || chunk->translucent_gpu_mesh.mesh_id);
            if (chunk->opaque_gpu_mesh.mesh_id)
            {
                plat->api.render_mesh_buffer(chunk->opaque_gpu_mesh.mesh_id, mesh_age_t);
            }
            
            if (chunk->translucent_gpu_mesh.mesh_id)
            {
                *translucent_gpu_mesh_ids.expand(scratch) = {
                    chunk->translucent_gpu_mesh.mesh_id,
                    mesh_age_t
                };
            }
        }
    }
    
    
    for (s64 translucent_id = (s64)translucent_gpu_mesh_ids.count - 1;
         translucent_id >= 0;
         translucent_id -= 1)
    {
        Defered_Mesh defered_mesh = *translucent_gpu_mesh_ids.at(translucent_id);
        plat->api.render_mesh_buffer(defered_mesh.mesh_id, defered_mesh.mesh_age_t);
    }
}



static_function void
free_cpu_mesh(World *world, Mesh **first_mesh)
{
    assert(first_mesh);
    
    if ((*first_mesh))
    {
        Mesh *last = (*first_mesh);
        while (last->next) { last = last->next; }
        
        last->next = world->first_free_mesh;
        world->first_free_mesh = (*first_mesh);
        (*first_mesh) = nullptr;
    }
}



static_function void
delete_gpu_mesh(Gpu_Mesh_Buffer *gpu_mesh)
{
    assert(gpu_mesh);
    plat->api.delete_mesh_buffer(gpu_mesh->mesh_id);
    *gpu_mesh = {};
}



static_function void
free_chunk_resources_and_initialize(World *world, Chunk *slot)
{
    slot->early_free = false;
    
    free_cpu_mesh(world, &slot->temp_opaque_mesh);
    delete_gpu_mesh(&slot->opaque_gpu_mesh);
    delete_gpu_mesh(&slot->temp_opaque_gpu_mesh);
    
    free_cpu_mesh(world, &slot->translucent_mesh);
    free_cpu_mesh(world, &slot->temp_translucent_mesh);
    delete_gpu_mesh(&slot->translucent_gpu_mesh);
    delete_gpu_mesh(&slot->temp_translucent_gpu_mesh);
    
    
    
    slot->blocks_generated = false;
    slot->can_contain_blocks = false;
    
    slot->mesh_is_valid = false;
    slot->mesh_is_empty = false;
    slot->mesh_uploaded_to_gpu = false;
    
    slot->blocks_version_frame = 0;
    slot->mesh_version_frame = 0;
}



static_function void
update_chunk_mesh_translate(World *world, Chunk *chunk)
{
    v3 chunk_offset = get_world_to_camera_chunk_offset(world, chunk->at);
    assert(chunk->opaque_gpu_mesh.mesh_id || chunk->translucent_gpu_mesh.mesh_id);
    assert(chunk->opaque_gpu_mesh.frame || chunk->translucent_gpu_mesh.frame);
    // @todo collapse this duplication!
    
    if (chunk->opaque_gpu_mesh.frame)
    {
        chunk->opaque_gpu_mesh.frame->mesh_translate = chunk_offset;
    }
    
    if (chunk->translucent_gpu_mesh.frame)
    {
        chunk->translucent_gpu_mesh.frame->mesh_translate = chunk_offset;
    }
}



static_function void
fill_voxel_texture_from_file(char *file_name, s32 depth_index)
{
    Scratch scratch(0);
    s32 cols, rows;
    u32 *mem = crappy_image_load(scratch, file_name, &cols, &rows);
    assert(cols == Block_Side);
    assert(rows == Block_Side);
    plat->api.fill_texture_3d(Atlas_Blocks, depth_index, mem);
}

static_function void
fill_voxel_texture_from_file(String file_name, s32 depth_index)
{
    Scratch scratch(0);
    char *cstr_file_name = to_cstr(scratch, file_name);
    fill_voxel_texture_from_file(cstr_file_name, depth_index);
}


static_function Chunk_Neighborhood
get_chunk_neighborhood(World *world, Vec3_S64 at_chunk)
{
    Chunk_Neighborhood result = {};
    
    for_s32(neighbor_index, Extended_Neighbors_Count)
    {
        Vec3_S64 offset = extended_offset_map[neighbor_index];
        Chunk *neighbor = get_chunk_slot(world, at_chunk + offset);
        
        if (neighbor && neighbor->blocks_generated)
        {
            result.all[neighbor_index] = neighbor;
        }
        else
        {
            return {};
        }
    }
    
    return result;
}


struct Collisions_Result
{
    Checked_Array<Collision_Entity> entities;
    b32 all_chunks_loaded;
};


static_function Collisions_Result
get_possible_block_collisions(Arena *collision_arena, App_State *app, World *world,
                              Vec3_S64 at_chunk, v3 chunk_relative_p,
                              v3 entity_half_dim, v3 move_vector)
{
    profile_function();
    
    Collisions_Result result = {};
    result.entities = create_checked_array<Collision_Entity>(collision_arena);
    
    Vec3_S64 move_direction = {
        move_vector.x >= 0.f ? 1 : -1,
        move_vector.y >= 0.f ? 1 : -1,
        move_vector.z >= 0.f ? 1 : -1
    };
    
    s64 fetch_start_x = -ceil_f32_to_s32(entity_half_dim.x);
    s64 fetch_start_y = -ceil_f32_to_s32(entity_half_dim.y);
    s64 fetch_start_z = -ceil_f32_to_s32(entity_half_dim.z);
    
    s64 fetch_end_x = ceil_f32_to_s32(entity_half_dim.x + absolute_value(move_vector.x));
    s64 fetch_end_y = ceil_f32_to_s32(entity_half_dim.y + absolute_value(move_vector.y));
    s64 fetch_end_z = ceil_f32_to_s32(entity_half_dim.z + absolute_value(move_vector.z));
    
    
    Vec3_S64 floor_chunk_relative_p = {
        floor_f32_to_s32(chunk_relative_p.x),
        floor_f32_to_s32(chunk_relative_p.y),
        floor_f32_to_s32(chunk_relative_p.z),
    };
    
    
    
    // collect possible collisions
    for (s64 offset_z = fetch_start_z;
         offset_z <= fetch_end_z;
         offset_z += 1)
    {
        for (s64 offset_y = fetch_start_y;
             offset_y <= fetch_end_y;
             offset_y += 1)
        {
            for (s64 offset_x = fetch_start_x;
                 offset_x <= fetch_end_x;
                 offset_x += 1)
            {
                Vec3_S64 offset = {offset_x, offset_y, offset_z};
                offset = hadamard(offset, move_direction);
                
                Vec3_S64 floor_p = floor_chunk_relative_p + offset;
                Vec3_S64 world_coord = combine_chunk_and_rel_coord_to_world_coord(at_chunk, floor_p);
                Block_Result block_res = get_block_result_at_world_coord(world, world_coord);
                
                
                
                if (!block_res.loaded_chunk)
                {
                    return result;
                }
                else
                {
                    if (is_solid(block_res.block.type))
                    {
                        v3 rel_p = v3_from_vec3_s64(floor_p) - chunk_relative_p + V3(0.5f);
                        Collision_Entity *sim = result.entities.expand(collision_arena);
                        sim->p = rel_p;
                        sim->half_dim = V3(0.5f);
                    }
                    else
                    {
#if Def_Internal
                        if (app->debug_collision_range)
                        {
                            v3 float_floor_p = v3_from_vec3_s64(floor_p);
                            render_cube_lines(plat->frame_line, world, at_chunk,
                                              float_floor_p, float_floor_p + V3(1),
                                              get_color(.0f, .95f, .6f));
                        }
#endif
                    }
                }
            }
        }
    }
    
    
    result.all_chunks_loaded = true;
    return result;
}



struct Closest_Collision
{
    b32 found_hit;
    f32 min_t;
    Collision_Entity *collision;
    Collision_Wall wall;
};

static_function Closest_Collision
find_closest_collision(Checked_Array<Collision_Entity> collisions,
                       v3 entity_half_dim,
                       v3 rel_p, v3 rel_move)
{
    Closest_Collision result = {};
    result.min_t = F32_Max;
    
    for_u64(collision_index, collisions.count)
    {
        Collision_Entity *collision = collisions.at(collision_index);
        v3 minkowski_half_dim = collision->half_dim + entity_half_dim;
        v3 min_corner = collision->p - minkowski_half_dim;
        v3 max_corner = collision->p + minkowski_half_dim;
        
        
        Collision_Wall walls[] =
        {
            // X west
            {
                min_corner.x,
                {rel_p.x, rel_p.y, rel_p.z},
                {rel_move.x, rel_move.y, rel_move.z},
                min_corner.y, max_corner.y,
                min_corner.z, max_corner.z,
                {-1, 0, 0}
            },
            // X east
            {
                max_corner.x,
                {rel_p.x, rel_p.y, rel_p.z},
                {rel_move.x, rel_move.y, rel_move.z},
                min_corner.y, max_corner.y,
                min_corner.z, max_corner.z,
                {1, 0, 0}
            },
            // Y down
            {
                min_corner.y,
                {rel_p.y, rel_p.x, rel_p.z},
                {rel_move.y, rel_move.x, rel_move.z},
                min_corner.x, max_corner.x,
                min_corner.z, max_corner.z,
                {0, -1, 0}
            },
            // Y up
            {
                max_corner.y,
                {rel_p.y, rel_p.x, rel_p.z},
                {rel_move.y, rel_move.x, rel_move.z},
                min_corner.x, max_corner.x,
                min_corner.z, max_corner.z,
                {0, 1, 0}
            },
            // Z south
            {
                min_corner.z,
                {rel_p.z, rel_p.y, rel_p.x},
                {rel_move.z, rel_move.y, rel_move.x},
                min_corner.y, max_corner.y,
                min_corner.x, max_corner.x,
                {0, 0, -1}
            },
            // Z south
            {
                max_corner.z,
                {rel_p.z, rel_p.y, rel_p.x},
                {rel_move.z, rel_move.y, rel_move.x},
                min_corner.y, max_corner.y,
                min_corner.x, max_corner.x,
                {0, 0, 1}
            },
        };
        
        
        
        for_s32(wall_index, array_count(walls))
        {
            Collision_Wall *wall = walls + wall_index;
            if (wall->entity_move.x != 0.f)
            {
                f32 t_result = (wall->wall_x - wall->entity_p.x) / wall->entity_move.x;
                if (t_result >= 0.f && result.min_t > t_result)
                {
                    f32 entity_y = wall->entity_p.y + t_result*wall->entity_move.y;
                    f32 entity_z = wall->entity_p.z + t_result*wall->entity_move.z;
                    
                    if (entity_y > wall->min_y && entity_y < wall->max_y &&
                        entity_z > wall->min_z && entity_z < wall->max_z)
                    {
                        result.found_hit = true;
                        result.min_t = t_result;
                        result.collision = collision;
                        result.wall = *wall;
                    }
                }
            }
        }
    }
    
    return result;
}



static_function b32
move_entity_parameters(App_State *app, World *world, Entity *entity, v3 accel)
{
    profile_function();
    
    f32 dt = app->dt;
    v3 current_dp = entity->dp + accel*dt;
    v3 current_move = current_dp*dt;
    
    
    f32 move_length = get_length(current_move);
    if (!move_length)
    {
        return true;
    }
    
    
    Scratch collision_scratch(0);
    Collisions_Result collisions = get_possible_block_collisions(collision_scratch, app, world,
                                                                 entity->at_chunk, entity->p,
                                                                 entity->half_dim, current_move);
    
    if (!collisions.all_chunks_loaded)
    {
        return false;
    }
    
    if (current_move.y)
    {
        entity->in_air = true;
    }
    
    
    
    
    // collide with entities
    {
        f32 epsilon_movement = 0.0001f;
        
        v3 rel_p = {};
        v3 rel_move = current_move;
        
        
#if Def_Internal
        if (app->debug_collision_visualization)
        {
            v3 debug_dir = normalize0(rel_move);
            render_line(plat->frame_line, world,
                        entity->at_chunk, entity->p + rel_p,
                        entity->p + rel_p + debug_dir*100.f);
        }
#endif
        
        
        
        for (s32 iteration = 0;
             iteration < 3;
             iteration += 1)
        {
            if (!rel_move.x && !rel_move.y && !rel_move.z)
            {
                break;
            }
            
            
            Closest_Collision closest = find_closest_collision(collisions.entities, entity->half_dim, rel_p, rel_move);
            
            
#if Def_Internal
            if (app->debug_collision_visualization && closest.collision)
            {
                v3 sim_p = entity->p + closest.collision->p;
                render_cube_lines(plat->frame_line, world, entity->at_chunk, 
                                  sim_p - closest.collision->half_dim,
                                  sim_p + closest.collision->half_dim,
                                  get_color(1, 1.f, 0));
                render_cube_lines(plat->frame_line, world, entity->at_chunk, 
                                  sim_p - closest.collision->half_dim - entity->half_dim,
                                  sim_p + closest.collision->half_dim + entity->half_dim,
                                  get_color(1, 0.1f, 0));
                render_cube_lines(plat->frame_line, world, entity->at_chunk, 
                                  entity->p - V3(0.05f),
                                  entity->p + V3(0.05f),
                                  get_color(0, 0, 1));
                assert(closest.wall.normal.x || closest.wall.normal.y || closest.wall.normal.z);
                {
                    v3 debug_origin = entity->p + rel_p + V3(0.2f+sin01(app->at))*0.2f;
                    v3 debug_dir = closest.wall.normal;
                    render_line(plat->frame_line, world,
                                entity->at_chunk, debug_origin,
                                debug_origin + debug_dir*50.f,
                                get_red(0.5f));
                }
                
            }
#endif
            
            
            if (closest.min_t > 1.f)
            {
                if (closest.found_hit)
                {
                    v3 test_rel_p = rel_p + 1.f*rel_move;
                    f32 test_axis = 0;
                    for_s32(i, 3)
                    {
                        if (closest.wall.normal.e[i])
                        {
                            test_axis = test_rel_p.e[i];
                            break;
                        }
                    }
                    
                    f32 distance_from_wall = absolute_value(closest.wall.wall_x - test_axis);
                    if (distance_from_wall > epsilon_movement)
                    {
                        closest = {};
                        closest.min_t = 1.f;
                    }
                }
                else
                {
                    closest.min_t = 1.f;
                }
            }
            
            
            rel_p += closest.min_t*rel_move;
            rel_move -= closest.min_t*rel_move;
            
            
            if (closest.found_hit)
            {
                if (closest.wall.normal.x == 1)
                {
                    rel_p.x = closest.wall.wall_x + epsilon_movement;
                }
                else if (closest.wall.normal.x == -1)
                {
                    rel_p.x = closest.wall.wall_x - epsilon_movement;
                }
                else if (closest.wall.normal.y == 1)
                {
                    rel_p.y = closest.wall.wall_x + epsilon_movement;
                    entity->in_air = false;
                }
                else if (closest.wall.normal.y == -1)
                {
                    rel_p.y = closest.wall.wall_x - epsilon_movement;
                }
                else if (closest.wall.normal.z == 1)
                {
                    rel_p.z = closest.wall.wall_x + epsilon_movement;
                }
                else if (closest.wall.normal.z == -1)
                {
                    rel_p.z = closest.wall.wall_x - epsilon_movement;
                }
                else
                {
                    assert(0);
                }
                
                
                for_s32(i, 3)
                {
                    if (closest.wall.normal.e[i])
                    {
                        rel_move.e[i] = 0.f;
                        current_dp.e[i] = 0.f;
                    }
                }
            }
            
            
            
#if Def_Internal
            if (app->debug_collision_visualization)
            {
                Color_Hsv hsv = {0.3f + 0.15f*iteration, 1.f, 1.f};
                v4 color = rgb_from_hsv(hsv);
                v3 debug_dir = normalize0(rel_move);
                render_line(plat->frame_line, world,
                            entity->at_chunk, entity->p + rel_p,
                            entity->p + rel_p + debug_dir*10.f,
                            color);
            }
#endif
        }
        
        
        
        entity->p += rel_p;
        entity->dp = current_dp;
        
        if (rel_p.x || rel_p.y || rel_p.z)
        {
            normalize_world_position(&entity->at_chunk, &entity->p);
        }
    }
    
    return true;
}


static_function v3
get_standard_acceleration(App_State *app, Entity *entity, v3 dir, f32 speed, b32 gravity)
{
    f32 dt = app->dt;
    dir = normalize0(dir);
    
    v3 accel = dir*speed;
    v3 drag = V3(7.5f, 0.1f, 7.5f);
    if (!entity->in_air)
    {
        drag += V3(.5f, 0, .5f);
    }
    
    if (!gravity)
    {
        drag.y = drag.x;
    }
    
    accel -= hadamard(drag, entity->dp);
    
    if (gravity)
    {
        accel += V3(0, -3.f*9.8f, 0);
    }
    
    return accel;
}


static_function b32
move_entity(App_State *app, World *world, Entity *entity, v3 dir, f32 speed, b32 gravity)
{
    v3 accel = get_standard_acceleration(app, entity, dir, speed, gravity);
    return move_entity_parameters(app, world, entity, accel);
}


static_function s64
get_chunk_to_camera_distance_sq(World *world, Chunk *chunk)
{
    assert(chunk);
    Vec3_S64 camera_at_chunk = world->camera.at_chunk;
    s64 dist_sq = get_length_sq(chunk->at - camera_at_chunk);
    return dist_sq;
}


static_function void
change_chunk_stage(World *world, Chunk *chunk, Chunk_Stage stage)
{
    profile_function();
    
    if (chunk->next && chunk->prev)
    {
        dll_remove(chunk);
        chunk->next = chunk->prev = nullptr;
    }
    else
    {
        assert(0);
    }
    
    
    switch (stage)
    {
        Invalid_Default_Case;
        case Chunk_Free: dll_insert_after(&world->free_sentinel, chunk); break;
        case Chunk_ToFree: dll_insert_after(&world->to_free_sentinel, chunk); break;
        
        case Chunk_InGeneration: dll_insert_after(&world->in_generation_sentinel, chunk); break;
        
        case Chunk_ToMesh: dll_insert_after(&world->to_mesh_sentinel, chunk); break;
        case Chunk_InMeshing: dll_insert_after(&world->in_meshing_sentinel, chunk); break;
        case Chunk_EmptyMesh: dll_insert_after(&world->empty_mesh_sentinel, chunk); break;
        
        case Chunk_InGpuUpload: dll_insert_after(&world->in_gpu_upload_sentinel, chunk); break;
        case Chunk_GpuUploaded:
        {
            // insert sort (by distance from camera)
            s64 this_dist_sq = get_chunk_to_camera_distance_sq(world, chunk);
            chunk->distance_from_camera = this_dist_sq;
            
            if (world->gpu_uploaded_sentinel.prev == &world->gpu_uploaded_sentinel ||
                world, world->gpu_uploaded_sentinel.prev->distance_from_camera <= this_dist_sq)
            {
                dll_insert_before(&world->gpu_uploaded_sentinel, chunk);
            }
            else
            {
                for_dll(search, &world->gpu_uploaded_sentinel)
                {
                    if (search->distance_from_camera >= this_dist_sq)
                    {
                        dll_insert_before(search, chunk);
                        break;
                    }
                }
            }
        }
        break;
    }
    
    
    assert(chunk->next && chunk->prev);
    chunk->stage = stage;
}


static_function void
change_chunk_priority_stage(World *world, Chunk *chunk, Chunk_Priority_Stage priority_stage)
{
    if (chunk->priority_stage)
    {
        assert(chunk->priority_next && chunk->priority_prev);
        dll_remove_NP(chunk, priority_next, priority_prev);
        chunk->priority_next = chunk->priority_prev = nullptr;
    }
    else
    {
        assert(!chunk->priority_next && !chunk->priority_prev);
    }
    
    
    switch (priority_stage)
    {
        Invalid_Default_Case;
        case ChunkPriority_None: break;
        
        case ChunkPriority_ToMesh: {
            dll_insert_after_NP(&world->priority_to_mesh_sentinel, chunk, priority_next, priority_prev);
        } break;
        case ChunkPriority_InMeshing: {
            dll_insert_after_NP(&world->priority_in_meshing_sentinel, chunk, priority_next, priority_prev);
        } break;
        case ChunkPriority_InGpuUpload: {
            dll_insert_after_NP(&world->priority_in_gpu_upload_sentinel, chunk, priority_next, priority_prev);
        } break;
    }
    
    chunk->priority_stage = priority_stage;
}





static_function void
mark_chunk_dirty(World *world, Chunk *chunk)
{
    assert(chunk->blocks_generated);
    
    chunk->can_contain_blocks = true;
    chunk->blocks_version_frame = plat->frame_number;
    
    if (!chunk->priority_stage)
    {
        change_chunk_priority_stage(world, chunk, ChunkPriority_ToMesh);
    }
}


enum Abort_Get_Block
{
    AbortGetBlock_Never,
    AbortGetBlock_OnMatching,
    AbortGetBlock_OnMissmatch,
};


static_function Block *
get_block_at_world_coord_and_mark_neighborhood_dirty(World *world, Vec3_S64 block_world_coord,
                                                     Abort_Get_Block abort_mode, Block_Type abort_type = Block_Air)
{
    Vec3_S64 chunk_dim = Chunk_Dim_Vec3_S64;
    
    Vec3_S64 at_chunk = {
        (block_world_coord.x >> Chunk_Dim_Power_X), 
        (block_world_coord.y >> Chunk_Dim_Power_Y), 
        (block_world_coord.z >> Chunk_Dim_Power_Z),
    };
    Vec3_S64 rel_p = {
        (block_world_coord.x & (Chunk_Dim_X - 1)),
        (block_world_coord.y & (Chunk_Dim_Y - 1)),
        (block_world_coord.z & (Chunk_Dim_Z - 1)),
    };
    
    Block *result = nullptr;
    
    Chunk *this_chunk = get_chunk_slot(world, at_chunk);
    if (this_chunk && this_chunk->blocks_generated)
    {
        result = &this_chunk->blocks[rel_p.z][rel_p.y][rel_p.x];
        
        switch (abort_mode)
        {
            Invalid_Default_Case;
            case AbortGetBlock_Never: break;
            
            case AbortGetBlock_OnMissmatch:
            {
                if (result->type != abort_type)
                {
                    return nullptr;
                };
            } break;
            
            case AbortGetBlock_OnMatching:
            {
                if (result->type == abort_type)
                {
                    return nullptr;
                }
            } break;
        }
        
        
        Vec3_S64 dirty_chunk_offset = {};
        
        // modify dirty_chunk_offset if block touches chunk border
        // to find neighbor chunks that were affected by this
        for_s32(i, 3)
        {
            s64 rel_axis = rel_p.e[i];
            if (rel_axis == 0)
            {
                dirty_chunk_offset.e[i] = -1;
            }
            else if (rel_axis == (chunk_dim.e[i] - 1))
            {
                dirty_chunk_offset.e[i] = +1;
            }
        }
        
        // mark neighbors along axis dirty (1 component)
        for_s32(i, 3)
        {
            Vec3_S64 chunk_offset = {};
            chunk_offset.e[i] = dirty_chunk_offset.e[i];
            Chunk *neighbor = get_chunk_slot(world, at_chunk + chunk_offset);
            if (neighbor && neighbor->blocks_generated)
            {
                mark_chunk_dirty(world, neighbor);
            }
        }
        
        // mark neighbors dirty (2 components)
        for_s32(i, 3)
        {
            Vec3_S64 chunk_offset = dirty_chunk_offset;
            chunk_offset.e[i] = 0;
            Chunk *neighbor = get_chunk_slot(world, at_chunk + chunk_offset);
            if (neighbor && neighbor->blocks_generated)
            {
                mark_chunk_dirty(world, neighbor);
            }
        }
        
        // mark diagonal neighbor dirty (3 components)
        {
            Chunk *neighbor = get_chunk_slot(world, at_chunk + dirty_chunk_offset);
            if (neighbor && neighbor->blocks_generated)
            {
                mark_chunk_dirty(world, neighbor);
            }
        }
        
        // mark main chunk dirty
        mark_chunk_dirty(world, this_chunk);
    }
    
    return result;
}

static_function void
place_block(World *world, Vec3_S64 block_world_coord, Block_Type type, b32 overwrite)
{
    Block *block = get_block_at_world_coord_and_mark_neighborhood_dirty(world, block_world_coord, (overwrite ? AbortGetBlock_Never : AbortGetBlock_OnMissmatch), Block_Air);
    
    if (block)
    {
        block->type = type;
    }
}

static_function void
destroy_block(World *world, Vec3_S64 block_world_coord)
{
    Block *block = get_block_at_world_coord_and_mark_neighborhood_dirty(world, block_world_coord, AbortGetBlock_OnMatching, Block_Air);
    
    if (block)
    {
        block->type = Block_Air;
    }
}




static_function v3
get_player_head_p(Entity *player)
{
    v3 p = player->p;
    p.y += player->half_dim.y*0.9f;
    return p;
}



static_function
APP_UPDATE(app_update)
{
    plat = interface;
    profile_function();
    
    Input *input = &plat->input;
    Rect2 inner_rect = rect_min_max(V2(0), V2i(plat->window_width, plat->window_height));
    
    b32 is_init_frame = false;
    b32 is_reload_frame = false;
    
    
    App_State *app = (App_State *)plat->app_state_initialized;
    if (!app)
    {
        profile_scope("First app init");
        
        is_init_frame = true;
        app = push_struct(&plat->perm_arena, App_State); // is zeroed
        plat->app_state_initialized = app;
        
        
        app->world.mesh_arena = create_virtual_arena();
        app->timestamp_now = time_perf();
        initialize_job_system(app, &app->job_system);
        
        
        World *world = &app->world;
        dll_initialize_sentinel(&world->in_generation_sentinel);
        dll_initialize_sentinel(&world->to_mesh_sentinel);
        dll_initialize_sentinel(&world->in_meshing_sentinel);
        dll_initialize_sentinel(&world->empty_mesh_sentinel);
        dll_initialize_sentinel(&world->in_gpu_upload_sentinel);
        dll_initialize_sentinel(&world->gpu_uploaded_sentinel);
        dll_initialize_sentinel(&world->to_free_sentinel);
        dll_initialize_sentinel(&world->free_sentinel);
        
        dll_initialize_sentinel_NP(&world->priority_to_mesh_sentinel, priority_next, priority_prev);
        dll_initialize_sentinel_NP(&world->priority_in_meshing_sentinel, priority_next, priority_prev);
        dll_initialize_sentinel_NP(&world->priority_in_gpu_upload_sentinel, priority_next, priority_prev);
        
        
        for_s32(chunk_slot_index, array_count(world->chunk_slots))
        {
            Chunk *slot = world->chunk_slots + chunk_slot_index;
            dll_insert_after(&world->free_sentinel, slot);
            assert(slot->stage == Chunk_Free);
        }
        
        
        
        {
            Scratch scratch(0);
            s32 cols, rows;
            u32 *mem = crappy_image_load(scratch, "font2.png", &cols, &rows, true);
            assert(cols == Glyph_Atlas_X);
            assert(rows == Glyph_Atlas_Y);
            plat->api.create_texture_2d(Atlas_Font, cols, rows, mem);
        }
        
        plat->api.create_texture_3d(Atlas_Blocks);
        
        
        
        
        //
        world->entity_count = 1;
        for_s32(entity_index, world->entity_count)
        {
            Entity *entity = world->entities + entity_index;
            *entity = {};
            entity->p = {rand_f32_11(&app->rng)*16.f, 40.f+rand_f32_11(&app->rng)*10.f, rand_f32_11(&app->rng)*16.f};
            entity->half_dim = V3(0.5f, 1.8f, 0.5f);
        }
        
        
        world->held_block_id = 1;
        world->player.p = {15, 31, 15};
        world->player.half_dim = {0.2f, 0.9f, 0.2f};
        normalize_world_position(&world->player);
        plat->mouse_in_locked_mode = true;
    }
    
    
    
    Job_System *job_system = &app->job_system;
    World *world = &app->world;
    app->text_at = {};
    
    
    if (plat->dll_reload_unhandled)
    {
        is_reload_frame = true;
        plat->dll_reload_unhandled = false;
        app->reload_counter += 1;
    }
    
    
    
    
    if (is_init_frame || is_reload_frame)
    {
        profile_scope("Dll init");
        
        app->rng = {10792348179023412113, 10987132423194123349};
        start_job_system(&app->job_system);
        
        
        
        // load textures
        {
            for_u64(load_texture_index, array_count(array_to_load_textures))
            {
                Scratch scratch(0);
                To_Load_Texture to_load = array_to_load_textures[load_texture_index];
                char *file_name = cstrf(scratch, "blocks16/%.*s", string_expand(to_load.file_name));
                fill_voxel_texture_from_file(file_name, to_load.texture_id);
            }
            
            plat->api.generate_mipmap_3d(Atlas_Blocks);
        }
    }
    
    
    
    
    if (plat->dll_reload_pending)
    {
        stop_job_system(&app->job_system);
    }
    
    
    
    if (!plat->is_window_active)
    {
        os_sleep_ms(33);
    }
    
    
    
    //~
    {
        s64 now = time_perf();
        app->true_dt = time_elapsed(now, app->timestamp_now);
        app->timestamp_now = now;
        
        app->dt = pick_smaller(app->true_dt, 1.f/16.f);
        app->at += app->dt;
        while (app->at > 1000.f) { app->at -= 1000.f; }
    }
    f32 dt = app->dt;
    f32 at = app->at;
    
    app->longest_true_dt = pick_bigger(app->longest_true_dt, app->true_dt);
    debug_app_tests(app);
    
    
    
    
    Vec3_S64 crosshair_block_hover_at_chunk = {};
    v3 crosshair_block_hover_rel_p = {};
    b32 crosshair_block_is_hovered = false;
    
    
    // controls
    {
        profile_scope("App user input");
        
        
        {
            world->held_block_id -= plat->input.mouse_wheel;
            
            world->held_block_id -= 1;
            world->held_block_id %= (Block_Count - 1);
            
            if (world->held_block_id < 0)
            {
                world->held_block_id += (Block_Count - 1);
            }
            world->held_block_id += 1;
        }
        
        
        
        Entity *player = &world->player;
        
        // move camera angle
        v2 cam_axis_coef = V2(1.f, 0.8f);
        v2 keyboard_cam_move = 3.5f*cam_axis_coef*dt;
        v2 mouse_cam_move = -0.002f*cam_axis_coef;
        
        if (plat->mouse_in_locked_mode && plat->is_window_active)
        {
            player->angle.x += mouse_cam_move.x * input->mouse_rel.x;
            player->angle.y += mouse_cam_move.y * input->mouse_rel.y;
        }
        
        if (input->cam_up.down)
        {
            player->angle.y += keyboard_cam_move.y;
        }
        if (input->cam_down.down)
        {
            player->angle.y -= keyboard_cam_move.y;
        }
        if (input->cam_left.down)
        {
            player->angle.x += keyboard_cam_move.x;
        }
        if (input->cam_right.down)
        {
            player->angle.x -= keyboard_cam_move.x;
        }
        
        f32 half_period = Pi32;
        player->angle.x = float_wrap(-half_period, player->angle.x, half_period);
        
        f32 quarter_period = 0.5f*half_period;
        player->angle.y = clamp(-quarter_period, player->angle.y, quarter_period);
        
        m4x4 player_angle_o = y_rotation(player->angle.x)*x_rotation(player->angle.y);
        v3 player_x_east_dir = get_column3(player_angle_o, 0);
        v3 player_z_north_dir = cross(player_x_east_dir, V3(0, -1, 0));
        v3 player_crosshair_dir = -get_column3(player_angle_o, 2);
        
        
        
        
        // move player
        v3 move_vector = {};
        if (input->forward.down || app->debug_go_forward)
        {
            move_vector += player_z_north_dir;
        }
        if (input->backward.down)
        {
            move_vector -= player_z_north_dir;
        }
        if (input->left.down)
        {
            move_vector -= player_x_east_dir;
        }
        if (input->right.down)
        {
            move_vector += player_x_east_dir;
        }
        if (input->sneak.down)
        {
            //move_vector -= V3(0, 1, 0);
        }
        
        
        if (just_pressed(input->f11))
        {
            world->fly_mode = !world->fly_mode;
        }
                         
        if (just_pressed(input->f1))
        {
            app->player_speed = 0;
        }
        if (just_pressed(input->f2))
        {
            app->player_speed += 1;
        }
        clamp_ref(1, &app->player_speed, 4);
        f32 player_speed = 40.f*square((f32)app->player_speed);
        
        
        
        if (world->fly_mode)
        {
            player_speed += 30.f;
            
            if (input->jump.down)
            {
                move_vector.y += 1.f;
            }
            if (input->sneak.down)
            {
                move_vector.y -= 1.f;
            }
        }
        else
        {
            if (just_pressed(input->jump))
            {
                world->timestamp_queued_jump = app->timestamp_now;
            }
            
            if (!player->in_air && world->timestamp_queued_jump)
            {
                f32 seconds_from_jump_press = time_elapsed(app->timestamp_now, world->timestamp_queued_jump);
                if (seconds_from_jump_press < 0.2f)
                {
                    world->timestamp_queued_jump = 0;
                    
                    f32 jump_speed = 50.f;
                    player->dp.y = 0.175f*jump_speed;
                }
            }
        }
        
        
        b32 gravity = !world->fly_mode;
        b32 player_updated = move_entity(app, world, player, move_vector, player_speed, gravity);
        if (!player_updated)
        {
            fast_text_nl(app, "loading chunks!"_f0, Default_Text_Scale, get_red());
        }
        
        
        
        
        
        
        
        
        // select block
        {
            v3 select_block_vector = player_crosshair_dir*8.f;
            v3 player_head_p = get_player_head_p(player);
            
            Scratch collision_scratch(0);
            // player_head_p can poke out of chunk but that shouldn't be a problem
            Collisions_Result collisions = get_possible_block_collisions(collision_scratch, app, world,
                                                                         player->at_chunk, player_head_p,
                                                                         {}, select_block_vector);
            if (collisions.all_chunks_loaded)
            {
                Closest_Collision closest = find_closest_collision(collisions.entities, {}, {}, select_block_vector);
                
                if (closest.found_hit && closest.min_t <= 1.f)
                {
                    assert(closest.collision);
                    Vec3_S64 block_at_chunk = player->at_chunk;
                    v3 block_rel_p = player_head_p + closest.collision->p;
                    normalize_world_position(&block_at_chunk, &block_rel_p);
                    
                    
                    crosshair_block_hover_at_chunk = block_at_chunk;
                    crosshair_block_hover_rel_p = block_rel_p;
                    crosshair_block_is_hovered = true;
                    
                    
                    Vec3_S64 block_world_coord = rel_coord_floor_to_world_coord(block_at_chunk, block_rel_p);
                    
                    
                    if (just_pressed(input->mouse_left) ||
                        (input->mouse_left.down && input->mouse_x2.down))
                    {
                        destroy_block(world, block_world_coord);
                    }
                    
                    // @todo reproject closest hit again for correctness? or queue right click for next frame?
                    if (just_pressed(input->mouse_right) ||
                        (input->mouse_right.down && input->mouse_x2.down))
                    {
                        Vec3_S64 normal_offset = floor_v3_to_vec3_s64(closest.wall.normal);
                        Vec3_S64 neighbor_world_coord = block_world_coord + normal_offset;
                        place_block(world, neighbor_world_coord, (Block_Type)world->held_block_id, false);
                    }
                }
            }
        }
        
        
        
        
        
        
        
        
        
        
        if (just_pressed(input->mouse_x1))
        {
            plat->mouse_in_locked_mode = !plat->mouse_in_locked_mode;
        }
        if (just_pressed(input->f3))
        {
            app->debug_text = !app->debug_text;
            app->debug_text_entity = false;
            app->debug_collision_range = false;
            app->debug_collision_visualization = false;
            app->debug_chunk_boundries = false;
            app->debug_sorts_highlight = false;
        }
        if (input->f3.down)
        {
            if (just_pressed(input->n1)) { app->debug_text_entity = true; }
            if (just_pressed(input->n2)) { app->debug_collision_visualization = true; }
            if (just_pressed(input->n3)) { app->debug_collision_range = true; }
            if (just_pressed(input->n4)) { app->debug_chunk_boundries = true; }
            if (just_pressed(input->n5)) { app->debug_sorts_highlight = true; }
        }
        if (just_pressed(input->f4))
        {
            plat->debug_show_wireframe = !plat->debug_show_wireframe;
        }
        if (just_pressed(input->f5))
        {
            world->debug_camera = world->camera;
            world->in_debug_camera = !world->in_debug_camera;
        }
        if (just_pressed(input->f9))
        {
            app->debug_go_forward = !app->debug_go_forward;
        }
        
        if (just_pressed(input->f8))
        {
            player->at_chunk = {};
            player->p = {0, 150, 0};
            player->angle = {-Pi32/2.f, 0};
            player->dp = {0, -2000.f, 0};
            normalize_world_position(player);
        }
        
        
        
        
        
        
        
        
        // player to camera
        {
            Camera *camera = &world->camera;
            camera->at_chunk = player->at_chunk;
            camera->p = get_player_head_p(player);
            camera->angle = player->angle;
        }
    }
    
    
    
    
    
    f32 aspect_ratio = safe_ratio_1(get_width(inner_rect), get_height(inner_rect));
    
    
    
    
    
    
    
    
    // cross chunk camera movement
    {
        profile_scope("App cross chunk movement");
        
        normalize_world_position(&world->camera);
        normalize_world_position(&world->debug_camera);
        
        
        
        Camera *camera = &world->camera;
        
        if (world->world_at_camera_chunk != camera->at_chunk ||
            !world->initialized)
        {
            world->initialized = true;
            
            
            Vec3_S64 chunk_index_offset = world->world_at_camera_chunk - camera->at_chunk;
            Vec3_S64 new_chunks_min_corner = camera->at_chunk + vec3_s64(Chunk_Min_X, Chunk_Min_Y, Chunk_Min_Z);
            Vec3_S64 new_chunks_one_past_corner = camera->at_chunk + vec3_s64(Chunk_OnePast_X, Chunk_OnePast_Y, Chunk_OnePast_Z);
            
            {
                Scratch scratch(0);
                Chunk_Handles *new_handles = push_struct_clear(scratch, Chunk_Handles);
                world->waiting_for_generation_count = 0;
                
                
                
                for_s32(index_z, Chunk_Count_Z)
                {
                    for_s32(index_y, Chunk_Count_Y)
                    {
                        for_s32(index_x, Chunk_Count_X)
                        {
                            Vec3_S64 old_chunk_index = vec3_s64(index_x, index_y, index_z);
                            u32 old_index = chunk_handle_1d_index_from_3d_index(old_chunk_index);
                            Chunk *old = world->chunk_handles.h[old_index];
                            
                            if (old)
                            {
                                Vec3_S64 new_chunk_index = old_chunk_index + chunk_index_offset;
                                
                                if (new_chunk_index.x >= 0 && new_chunk_index.x < Chunk_Count_X &&
                                    new_chunk_index.y >= 0 && new_chunk_index.y < Chunk_Count_Y &&
                                    new_chunk_index.z >= 0 && new_chunk_index.z < Chunk_Count_Z)
                                {
                                    u32 new_index = chunk_handle_1d_index_from_3d_index(new_chunk_index);
                                    new_handles->h[new_index] = old;
                                }
                                else
                                {
                                    change_chunk_priority_stage(world, old, ChunkPriority_None);
                                    
                                    if (old->lock_counter == 0)
                                    {
                                        free_chunk_resources_and_initialize(world, old);
                                        change_chunk_stage(world, old, Chunk_Free);
                                    }
                                    else
                                    {
                                        old->early_free = true;
                                        change_chunk_stage(world, old, Chunk_ToFree);
                                    }
                                }
                            }
                        }
                    }
                }
                
                
                copy_array(&world->chunk_handles, new_handles, Chunk_Handles, 1);
                world->world_at_camera_chunk = camera->at_chunk;
                world->chunks_handles_min_corner = new_chunks_min_corner;
            }
            
            
            
            {
                for_s32(index_z, Chunk_Count_Z)
                {
                    for_s32(index_y, Chunk_Count_Y)
                    {
                        for_s32(index_x, Chunk_Count_X)
                        {
                            Vec3_S64 index3d = vec3_s64(index_x, index_y, index_z);
                            u32 index1d = chunk_handle_1d_index_from_3d_index(index3d);
                            
                            if (!world->chunk_handles.h[index1d])
                            {
                                Vec3_S64 *waiting = &world->waiting_for_generation[world->waiting_for_generation_count];
                                *waiting = new_chunks_min_corner + index3d;
                                world->waiting_for_generation_count += 1;
                            }
                        }
                    }
                }
                
                merge_sort_array_by_distance_from_p(world->waiting_for_generation,
                                                    world->waiting_for_generation_count,
                                                    world->camera.at_chunk);
            }
            
            
            
            
            
            {
                profile_scope("Merge sort GPU uploaded chunks");
                // @todo right now we unload from dll (doubly linked list) to array to sort
                // and then load chunks into dll again. Maybe sort on dll directly?
                
                Scratch scratch(0);
                Checked_Array<Chunk *> gpu_chunks = create_checked_array<Chunk *>(scratch);
                
                for (Chunk *chunk = world->gpu_uploaded_sentinel.next;
                     chunk != &world->gpu_uploaded_sentinel;
                     )
                {
                    Chunk *next = chunk->next;
                    
                    dll_remove(chunk);
                    update_chunk_mesh_translate(world, chunk);
                    chunk->distance_from_camera = get_chunk_to_camera_distance_sq(world, chunk);
                    *gpu_chunks.expand(scratch) = chunk;
                    
                    chunk = next;
                }
                
                merge_sort_array(gpu_chunks.array, gpu_chunks.count);
                
                for_u64(gpu_chunk_index, gpu_chunks.count)
                {
                    Chunk *chunk = *gpu_chunks.at(gpu_chunk_index);
                    dll_insert_after(&world->gpu_uploaded_sentinel, chunk);
                }
            }
        }
    }
    
    
    
    
    
    
    // update entities
    {
        profile_scope("Update entities");
        
        
        static_local s32 mode = 0; // @delete debug code
        if (just_pressed(input->f10))
        {
            mode += 1;
            mode %= 3;
        }
        
        
        for_s32(entity_index, world->entity_count)
        {
            Entity *entity = world->entities + entity_index;
            
            f32 speed = 400.f;
            v3 dir = V3(0);
            
            v3 mode_dir = V3(1,0,0);
            if (mode == 1)
            {
                mode_dir = V3(0,1,0);
            }
            else if (mode == 2)
            {
                mode_dir = V3(0,0,1);
            }
            
            
            if (input->f6.down)
            {
                dir += -mode_dir;
            }
            if (input->f7.down)
            {
                dir += mode_dir;
            }
            
            
            entity->updated = move_entity(app, world, entity, dir, speed, true);
        }
    }
    
    
    
    
    
    
    
    
    
    
    plat->api.sync_frame_rendering(plat->render_buffer_index); // @todo If we are gpu bound this function will wait. We could skip rendering for this frame instead
    
    
    
    {
        Camera *real_camera = &world->camera;
        Camera camera_temp;
        
        if (world->in_debug_camera)
        {
            camera_temp = world->debug_camera;
            Vec3_S64 chunk_offset = camera_temp.at_chunk - real_camera->at_chunk;
            camera_temp.p += hadamard(v3_from_vec3_s64(chunk_offset), Chunk_Dim_Vec3_S64);
            
            fast_text_nl(app, "debug_cam"_f0, Default_Text_Scale, get_red(.4f + .2f*sin(4.1f*app->at)));
        }
        else
        {
            camera_temp = *real_camera;
        }
        
        m4x4_inv world_transform = get_world_camera_transform(&camera_temp, aspect_ratio);
        plat->api.set_world_transform(&world_transform.forward, camera_temp.p);
    }
    
    
    render_world(app, world, aspect_ratio);
    
    
    for_s32(entity_index, world->entity_count)
    {
        Entity *entity = world->entities + entity_index;
        v4 color = (entity->updated ? get_yellow() : get_red());
        
        render_entity(plat->frame_debug, world, entity, color);
    }
    
    if (world->in_debug_camera)
    {
        render_entity(plat->frame_debug, world, &world->player, get_blue(0.7f));
    }
    
    
    
    
    if (crosshair_block_is_hovered)
    {
        v3 r = V3(0.505f);
        render_cube_lines(plat->frame_line, world, crosshair_block_hover_at_chunk,
                          crosshair_block_hover_rel_p - r, crosshair_block_hover_rel_p + r,
                          get_white(0.9f));
    }
    
    
    
    
    
    
    
    
#if 1
    if (app->debug_chunk_boundries)
    {
        Camera *camera = &world->camera;
        v3 max = v3_from_vec3_s64(Chunk_Dim_Vec3_S64);
        render_cube_lines(plat->frame_line, world, camera->at_chunk + vec3_s64(0,0,0), {}, max, get_green());
        render_cube_lines(plat->frame_line, world, camera->at_chunk + vec3_s64(+1,0,0), {}, max, get_blue(0.2f));
        render_cube_lines(plat->frame_line, world, camera->at_chunk + vec3_s64(-1,0,0), {}, max, get_blue(0.2f));
        render_cube_lines(plat->frame_line, world, camera->at_chunk + vec3_s64(0,+1,0), {}, max, get_blue(0.2f));
        render_cube_lines(plat->frame_line, world, camera->at_chunk + vec3_s64(0,-1,0), {}, max, get_blue(0.2f));
        render_cube_lines(plat->frame_line, world, camera->at_chunk + vec3_s64(0,0,+1), {}, max, get_blue(0.2f));
        render_cube_lines(plat->frame_line, world, camera->at_chunk + vec3_s64(0,0,-1), {}, max, get_blue(0.2f));
#if 0
        render_cube_lines(plat->frame_line, world, camera->at_chunk + vec3_s64(0,-1,-1), {}, max, get_gray(0.2f));
        render_cube_lines(plat->frame_line, world, camera->at_chunk + vec3_s64(0,-1,+1), {}, max, get_gray(0.2f));
        render_cube_lines(plat->frame_line, world, camera->at_chunk + vec3_s64(0,+1,-1), {}, max, get_gray(0.2f));
        render_cube_lines(plat->frame_line, world, camera->at_chunk + vec3_s64(0,+1,+1), {}, max, get_gray(0.2f));
        render_cube_lines(plat->frame_line, world, camera->at_chunk + vec3_s64(-1,+1,0), {}, max, get_gray(0.2f));
        render_cube_lines(plat->frame_line, world, camera->at_chunk + vec3_s64(+1,+1,0), {}, max, get_gray(0.2f));
        render_cube_lines(plat->frame_line, world, camera->at_chunk + vec3_s64(+1,-1,0), {}, max, get_gray(0.2f));
        render_cube_lines(plat->frame_line, world, camera->at_chunk + vec3_s64(-1,-1,0), {}, max, get_gray(0.2f));
#endif
    }
#endif
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    {
        profile_scope("App schedule worker tasks");
        
        s32 worker_index = job_system->next_worker_to_be_used; // @todo do something smarter to pick thread without jobs?
        s32 worker_count = safe_truncate_to_s32(job_system->workers.count);
        b32 release_semaphore = false;
        
        
        // move unlocked chunks from to_free_sentinel to free_sentinel
        {
            s32 locked_chunks_counter = 0;
            
            for (Chunk *chunk = world->to_free_sentinel.next;;)
            {
                if (chunk == &world->to_free_sentinel)
                {
                    break;
                }
                Chunk *next = chunk->next;
                
                
                if (chunk->lock_counter)
                {
                    locked_chunks_counter += 1;
                    if (locked_chunks_counter > 3) // @todo arbitrary - measure if that "optimization" makes sense
                    {
                        break;
                    }
                }
                else
                {
                    free_chunk_resources_and_initialize(world, chunk);
                    change_chunk_stage(world, chunk, Chunk_Free);
                }
                
                
                chunk = next;
            }
        }
        
        
        
        
        // resupply meshes
        for_s32(index, worker_count)
        {
            Worker *worker = job_system->workers.at(index);
            u32 mesh_resource_count = worker_count_tasks(&worker->job_mesh_resource);
            
            u32 target_count = kilobytes(4);
            s32 part = target_count/4;
            
            if (mesh_resource_count < target_count)
            {
                for_s32(i, part)
                {
                    auto task = worker_get_new_task_slot(&worker->job_mesh_resource);
                    Mesh *mesh = get_new_mesh(world);
                    task->payload = mesh;
                }
                
                worker_send_message_increment_task_end(worker, &worker->job_mesh_resource, Job_MeshResource);
            }
        }
        
        
        
        
        
        
        
        
        
        
        
        
        //
        //~ PRIORITY JOBS
        // C++ is a crappy language so for now make sure to keep it in sync with normal version far below
        
        if (world->priority_in_gpu_upload_sentinel.priority_prev != &world->priority_in_gpu_upload_sentinel)
        {
            for (Chunk *chunk = world->priority_in_gpu_upload_sentinel.priority_prev;;)
            {
                if (chunk == &world->priority_in_gpu_upload_sentinel)
                {
                    break;
                }
                Chunk *prev = chunk->priority_prev;
                
                
                if (chunk->mesh_uploaded_to_gpu)
                {
                    assert(chunk->temp_opaque_mesh ||
                           chunk->temp_translucent_mesh ||
                           chunk->translucent_mesh);
                    
                    if (chunk->temp_opaque_mesh)
                    {
                        free_cpu_mesh(world, &chunk->temp_opaque_mesh);
                        plat->api.finalize_mesh_buffer(chunk->temp_opaque_gpu_mesh.mesh_id, true);
                        delete_gpu_mesh(&chunk->opaque_gpu_mesh);
                        chunk->opaque_gpu_mesh = chunk->temp_opaque_gpu_mesh;
                        chunk->temp_opaque_gpu_mesh = {};
                    }
                    
                    
                    if (chunk->temp_translucent_mesh)
                    {
                        free_cpu_mesh(world, &chunk->translucent_mesh);
                        chunk->translucent_mesh = chunk->temp_translucent_mesh;
                        chunk->temp_translucent_mesh = {};
                        
                        plat->api.finalize_mesh_buffer(chunk->temp_translucent_gpu_mesh.mesh_id, false);
                        delete_gpu_mesh(&chunk->translucent_gpu_mesh);
                        chunk->translucent_gpu_mesh = chunk->temp_translucent_gpu_mesh;
                        chunk->temp_translucent_gpu_mesh = {};
                    }
                    
                    
                    update_chunk_mesh_translate(world, chunk);
                    change_chunk_stage(world, chunk, Chunk_GpuUploaded);
                    
                    if (chunk->mesh_version_frame == chunk->blocks_version_frame)
                    {
                        change_chunk_priority_stage(world, chunk, ChunkPriority_None);
                    }
                    else
                    {
                        change_chunk_priority_stage(world, chunk, ChunkPriority_ToMesh);
                    }
                }
                
                
                chunk = prev;
            }
        }
        
        
        
        
        if (world->priority_in_meshing_sentinel.priority_prev != &world->priority_in_meshing_sentinel)
        {
            b32 jobs_added = false;
            
            for (Chunk *chunk = world->priority_in_meshing_sentinel.priority_prev;;)
            {
                if (chunk == &world->priority_in_meshing_sentinel)
                {
                    break;
                }
                Chunk *prev = chunk->priority_prev;
                
                
                if (chunk->mesh_is_valid)
                {
                    chunk->mesh_uploaded_to_gpu = false;
                    Mesh **opaque_mesh = &chunk->temp_opaque_mesh;
                    Gpu_Mesh_Buffer *opaque_gpu_mesh = &chunk->temp_opaque_gpu_mesh;
                    Mesh **translucent_mesh = &chunk->temp_translucent_mesh;
                    Gpu_Mesh_Buffer *translucent_gpu_mesh = &chunk->temp_translucent_gpu_mesh;
                    
                    
                    if (!(*opaque_mesh))
                    {
                        delete_gpu_mesh(opaque_gpu_mesh);
                    }
                    if (!(*translucent_mesh))
                    {
                        delete_gpu_mesh(translucent_gpu_mesh);
                    }
                    
                    
                    
                    if (chunk->mesh_is_empty)
                    {
                        change_chunk_stage(world, chunk, Chunk_EmptyMesh);
                        
                        if (chunk->mesh_version_frame == chunk->blocks_version_frame)
                        {
                            change_chunk_priority_stage(world, chunk, ChunkPriority_None);
                        }
                        else
                        {
                            change_chunk_priority_stage(world, chunk, ChunkPriority_ToMesh);
                        }
                    }
                    else
                    {
                        assert(!opaque_gpu_mesh->mesh_id);
                        assert(!translucent_gpu_mesh->mesh_id);
                        
                        Render_Chunk_Payload render_data = {};
                        render_data.chunk = chunk;
                        
                        
                        s32 opaque_quad_count = count_meshes_in_linked_list((*opaque_mesh));
                        if (opaque_quad_count)
                        {
                            Gpu_Mesh_Buffer *gpu_mesh = opaque_gpu_mesh;
                            *gpu_mesh = plat->api.create_mesh_buffer(opaque_quad_count);
                            render_data.opaque_mesh = (*opaque_mesh);
                            render_data.opaque_verts = gpu_mesh->frame->verts;
                            render_data.opaque_face_array = gpu_mesh->frame->face_array;
                        }
                        
                        s32 translucent_quad_count = count_meshes_in_linked_list((*translucent_mesh));
                        if (translucent_quad_count)
                        {
                            Gpu_Mesh_Buffer *gpu_mesh = translucent_gpu_mesh;
                            sort_mesh_faces_back_to_front(world, chunk, (*translucent_mesh));
                            *gpu_mesh = plat->api.create_mesh_buffer(translucent_quad_count);
                            render_data.translucent_mesh = (*translucent_mesh);
                            render_data.translucent_verts = gpu_mesh->frame->verts;
                            render_data.translucent_face_array = gpu_mesh->frame->face_array;
                        }
                        
                        
                        increment_chunk_lock(chunk);
                        change_chunk_priority_stage(world, chunk, ChunkPriority_InGpuUpload);
                        jobs_added = true;
                        
                        Worker *worker = job_system->workers.at(worker_index);
                        worker_index = (worker_index + 1) % worker_count;
                        auto task = worker_get_new_task_slot(&worker->job_priority_gpu_upload);
                        task->payload = render_data;
                    }
                }
                
                
                chunk = prev;
            }
            
            
            
            if (jobs_added)
            {
                for_s32(index, worker_count)
                {
                    Worker *worker = job_system->workers.at(index);
                    worker_send_message_increment_task_end(worker, &worker->job_priority_gpu_upload, Job_PriorityGpuUpload);
                }
                
                release_semaphore |= true;
            }
        }
        
        
        
        
        
        
        if (world->priority_to_mesh_sentinel.priority_prev != &world->priority_to_mesh_sentinel)
        {
            b32 jobs_added = false;
            
            for (Chunk *chunk = world->priority_to_mesh_sentinel.priority_prev;;)
            {
                if (chunk == &world->priority_to_mesh_sentinel)
                {
                    break;
                }
                Chunk *prev = chunk->priority_prev;
                
                
                if (chunk->stage == Chunk_EmptyMesh ||
                    chunk->stage == Chunk_GpuUploaded)
                {
                    Chunk_Neighborhood neighborhood = get_chunk_neighborhood(world, chunk->at);
                    if (neighborhood.all[0])
                    {
                        chunk->mesh_is_valid = false;
                        chunk->mesh_is_empty = false;
                        chunk->mesh_version_frame = chunk->blocks_version_frame;
                        
                        
                        Worker *worker = job_system->workers.at(worker_index);
                        worker_index = (worker_index + 1) % worker_count;
                        auto task = worker_get_new_task_slot(&worker->job_priority_mesh);
                        task->payload = neighborhood;
                        
                        for_s32(hood_index, array_count(neighborhood.all))
                        {
                            increment_chunk_lock(neighborhood.all[hood_index]);
                        }
                        change_chunk_priority_stage(world, chunk, ChunkPriority_InMeshing);
                        jobs_added = true;
                    }
                }
                
                chunk = prev;
            }
            
            
            
            if (jobs_added)
            {
                for_s32(index, worker_count)
                {
                    Worker *worker = job_system->workers.at(index);
                    worker_send_message_increment_task_end(worker, &worker->job_priority_mesh, Job_PriorityMesh);
                }
                
                release_semaphore |= true;
            }
        }
        
        
        
        
        
        // sort translucent blocks & refresh the gpu upload
        {
            profile_scope("Scan & sort translucent blocks");
            b32 jobs_added = false;
            
            for_dll(chunk, &world->gpu_uploaded_sentinel)
            {
                if (!chunk->priority_stage)
                {
                    if (chunk->translucent_mesh)
                    {
                        Camera *camera = &world->camera;
                        
                        b32 do_sorting = false;
                        
                        Vec3_S64 current_chunk_offset = camera->at_chunk - chunk->at;
                        s64 current_chunk_dist_sq = get_length_sq(current_chunk_offset);
                        
                        Vec3_S64 last_chunk_delta = chunk->transparecy_sorted_at_chunk - camera->at_chunk;
                        v3 last_rel_delta = chunk->transparency_sorted_at_rel_p - camera->p;
                        v3 last_delta = v3_from_vec3_s64(hadamard(last_chunk_delta, Chunk_Dim_Vec3_S64)) + last_rel_delta;
                        f32 last_delta_len = get_length(last_delta);
                        
                        
                        if (last_delta_len > current_chunk_dist_sq*16 + 0.5)
                        {
                            sort_mesh_faces_back_to_front(world, chunk, chunk->translucent_mesh);
                            
                            assert(chunk->translucent_gpu_mesh.frame);
                            Render_Chunk_Payload render_data = {};
                            render_data.chunk = chunk;
                            render_data.translucent_mesh = chunk->translucent_mesh;
                            render_data.translucent_verts = chunk->translucent_gpu_mesh.frame->verts;
                            render_data.translucent_face_array = chunk->translucent_gpu_mesh.frame->face_array;
                            
                            
                            increment_chunk_lock(chunk);
                            change_chunk_priority_stage(world, chunk, ChunkPriority_InGpuUpload);
                            jobs_added = true;
                            
                            Worker *worker = job_system->workers.at(worker_index);
                            worker_index = (worker_index + 1) % worker_count;
                            auto task = worker_get_new_task_slot(&worker->job_priority_gpu_upload);
                            task->payload = render_data;
                            
                            plat->stats.total_chunk_sorts_count += 1;
                            if (app->debug_sorts_highlight)
                            {
                                Debug_Chunk_Sort_Highlight *highlight = push_debug_sort_highlight();
                                highlight->timestamp = app->timestamp_now;
                                highlight->at_chunk = chunk->at;
                            }
                        }
                    }
                }
            }
            
            
            if (jobs_added)
            {
                for_s32(index, worker_count)
                {
                    Worker *worker = job_system->workers.at(index);
                    worker_send_message_increment_task_end(worker, &worker->job_priority_gpu_upload, Job_PriorityGpuUpload);
                }
                
                release_semaphore |= true;
            }
        }
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        //
        //~ NORMAL JOBS
        //
        
        
        // check generated chunks
        for (Chunk *chunk = world->in_generation_sentinel.next;;)
        {
            if (chunk == &world->in_generation_sentinel)
            {
                break;
            }
            Chunk *next = chunk->next;
            
            if (chunk->blocks_generated)
            {
                if (chunk->can_contain_blocks)
                {
                    change_chunk_stage(world, chunk, Chunk_ToMesh);
                }
                else
                {
                    change_chunk_stage(world, chunk, Chunk_EmptyMesh);
                }
            }
            
            chunk = next;
        }
        
        
        
        // generate chunks
        {
            b32 added_jobs = false;
            
            while (world->waiting_for_generation_count)
            {
                if (world->free_sentinel.prev != &world->free_sentinel)
                {
                    Chunk *free_slot = world->free_sentinel.prev;
                    free_slot->at = world->waiting_for_generation[world->waiting_for_generation_count-1];
                    world->waiting_for_generation_count -= 1;
                    
                    Chunk **slot_address = get_chunk_slot_address(world, free_slot->at);
                    assert(!(*slot_address));
                    *slot_address = free_slot;
                    
                    
                    Worker *worker = job_system->workers.at(worker_index);
                    worker_index = (worker_index + 1) % worker_count;
                    auto task = worker_get_new_task_slot(&worker->job_generate);
                    task->payload = free_slot;
                    
                    increment_chunk_lock(free_slot);
                    change_chunk_stage(world, free_slot, Chunk_InGeneration);
                    added_jobs = true;
                }
                else
                {
                    break;
                }
            }
            
            
            if (added_jobs)
            {
                for_s32(index, worker_count)
                {
                    Worker *worker = job_system->workers.at(index);
                    worker_send_message_increment_task_end(worker, &worker->job_generate, Job_Generate);
                }
                
                release_semaphore |= true;
            }
        }
        
        
        
        
        
        
        
        
        
        
        if (world->in_gpu_upload_sentinel.next != &world->in_gpu_upload_sentinel)
        {
            for (Chunk *chunk = world->in_gpu_upload_sentinel.next;;)
            {
                if (chunk == &world->in_gpu_upload_sentinel)
                {
                    break;
                }
                Chunk *next = chunk->next;
                
                
                if (chunk->mesh_uploaded_to_gpu)
                {
                    free_cpu_mesh(world, &chunk->temp_opaque_mesh);
                    plat->api.finalize_mesh_buffer(chunk->opaque_gpu_mesh.mesh_id, true);
                    plat->api.finalize_mesh_buffer(chunk->translucent_gpu_mesh.mesh_id, false);
                    change_chunk_stage(world, chunk, Chunk_GpuUploaded);
                    update_chunk_mesh_translate(world, chunk);
                    chunk->initial_gpu_upload_timestamp = app->timestamp_now;
                }
                
                
                chunk = next;
            }
        }
        
        
        
        
        
        
        if (world->in_meshing_sentinel.next != &world->in_meshing_sentinel)
        {
            b32 jobs_added = false;
            
            for (Chunk *chunk = world->in_meshing_sentinel.next;;)
            {
                if (chunk == &world->in_meshing_sentinel)
                {
                    break;
                }
                Chunk *next = chunk->next;
                
                
                if (chunk->mesh_is_valid)
                {
                    Mesh **opaque_mesh = &chunk->temp_opaque_mesh;
                    Gpu_Mesh_Buffer *opaque_gpu_mesh = &chunk->opaque_gpu_mesh;
                    Mesh **translucent_mesh = &chunk->translucent_mesh;
                    Gpu_Mesh_Buffer *translucent_gpu_mesh = &chunk->translucent_gpu_mesh;
                    
                    
                    if (chunk->mesh_is_empty)
                    {
                        change_chunk_stage(world, chunk, Chunk_EmptyMesh);
                    }
                    else
                    {
                        assert(!opaque_gpu_mesh->mesh_id);
                        assert(!translucent_gpu_mesh->mesh_id);
                        
                        Render_Chunk_Payload render_data = {};
                        render_data.chunk = chunk;
                        
                        
                        s32 opaque_quad_count = count_meshes_in_linked_list((*opaque_mesh));
                        if (opaque_quad_count)
                        {
                            Gpu_Mesh_Buffer *gpu_mesh = opaque_gpu_mesh;
                            *gpu_mesh = plat->api.create_mesh_buffer(opaque_quad_count);
                            render_data.opaque_mesh = (*opaque_mesh);
                            render_data.opaque_verts = gpu_mesh->frame->verts;
                            render_data.opaque_face_array = gpu_mesh->frame->face_array;
                        }
                        
                        s32 translucent_quad_count = count_meshes_in_linked_list((*translucent_mesh));
                        if (translucent_quad_count)
                        {
                            Gpu_Mesh_Buffer *gpu_mesh = translucent_gpu_mesh;
                            sort_mesh_faces_back_to_front(world, chunk, (*translucent_mesh));
                            *gpu_mesh = plat->api.create_mesh_buffer(translucent_quad_count);
                            render_data.translucent_mesh = (*translucent_mesh);
                            render_data.translucent_verts = gpu_mesh->frame->verts;
                            render_data.translucent_face_array = gpu_mesh->frame->face_array;
                        }
                        
                        
                        
                        Worker *worker = job_system->workers.at(worker_index);
                        worker_index = (worker_index + 1) % worker_count;
                        auto task = worker_get_new_task_slot(&worker->job_gpu_upload);
                        task->payload = render_data;
                        
                        increment_chunk_lock(chunk);
                        change_chunk_stage(world, chunk, Chunk_InGpuUpload);
                        jobs_added = true;
                    }
                }
                
                
                chunk = next;
            }
            
            
            
            if (jobs_added)
            {
                for_s32(index, worker_count)
                {
                    Worker *worker = job_system->workers.at(index);
                    worker_send_message_increment_task_end(worker, &worker->job_gpu_upload, Job_GpuUpload);
                }
                
                release_semaphore |= true;
            }
        }
        
        
        
        
        // @todo do not iterate over chunks in to_mesh that will never have neighbors somehow?
        if (world->to_mesh_sentinel.next != &world->to_mesh_sentinel)
        {
            b32 jobs_added = false;
            
            for (Chunk *chunk = world->to_mesh_sentinel.next;;)
            {
                if (chunk == &world->to_mesh_sentinel)
                {
                    break;
                }
                Chunk *next = chunk->next;
                
                
                Chunk_Neighborhood neighborhood = get_chunk_neighborhood(world, chunk->at);
                if (neighborhood.all[0])
                {
                    chunk->mesh_version_frame = chunk->blocks_version_frame;
                    
                    Worker *worker = job_system->workers.at(worker_index);
                    worker_index = (worker_index + 1) % worker_count;
                    auto task = worker_get_new_task_slot(&worker->job_mesh);
                    task->payload = neighborhood;
                    
                    for_s32(hood_index, array_count(neighborhood.all))
                    {
                        increment_chunk_lock(neighborhood.all[hood_index]);
                    }
                    change_chunk_stage(world, chunk, Chunk_InMeshing);
                    jobs_added = true;
                }
                
                
                chunk = next;
            }
            
            
            if (jobs_added)
            {
                for_s32(index, worker_count)
                {
                    Worker *worker = job_system->workers.at(index);
                    worker_send_message_increment_task_end(worker, &worker->job_mesh, Job_Mesh);
                }
                
                release_semaphore |= true;
            }
        }
        
        
        
        
        
        
        
        if (release_semaphore)
        {
            ReleaseSemaphore(job_system->semaphore, worker_count, nullptr);
        }
        
        
        job_system->next_worker_to_be_used = worker_index;
    }
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    //
    //
    //
    
    if (app->debug_sorts_highlight)
    {
        f32 max_time = 1.f;
        
        auto process_highlight = [&](s32 index)
        {
            Debug_Chunk_Sort_Highlight *highlight = plat->stats.sort_highlights + index;
            f32 delta_time = time_elapsed(app->timestamp_now, highlight->timestamp);
            if (delta_time > max_time)
            {
                plat->stats.sort_highlight_start = (index + 1) % array_count(plat->stats.sort_highlights);
            }
            else
            {
                f32 t = delta_time / max_time;
                v4 color = get_red(t);
                f32 td = clamp(0.01f, t, 0.99f);
                v3 dim = v3_from_vec3_s64(Chunk_Dim_Vec3_S64)*td;
                v3 p = v3_from_vec3_s64(Chunk_Dim_Vec3_S64) - dim;
                
                render_cube_lines(plat->frame_line, world, highlight->at_chunk, p, dim, color);
            }
        };
        
        
        if (plat->stats.sort_highlight_start <= plat->stats.sort_highlight_one_past_last)
        {
            for (s32 index = plat->stats.sort_highlight_start;
                 index < plat->stats.sort_highlight_one_past_last;
                 index += 1)
            {
                process_highlight(index);
            }
        }
        else
        {
            for (s32 index = plat->stats.sort_highlight_start;
                 index < array_count(plat->stats.sort_highlights);
                 index += 1)
            {
                process_highlight(index);
            }
            
            for (s32 index = 0;
                 index < plat->stats.sort_highlight_one_past_last;
                 index += 1)
            {
                process_highlight(index);
            }
        }
    }
    
    
    
    
    
    
    // ui
    {
        f32 s = Default_Text_Scale;
        fast_text_nl(app, ""_f0, s*0.25f);
        
        fast_text(app, "held block: "_f0, s, get_white());
        String block_text = get_block_pretty_name_string(world->held_block_id);
        fast_block_text(app, world->held_block_id, s*2.f);
        fast_text(app, " "_f0, s);
        fast_text_nl(app, block_text, s, get_color(.9f, .85f, .3f));
    }
    
    
    
    
    
    if (app->debug_text)
    {
        profile_scope("App debug text");
        
        
        Scratch scratch(0);
        
        fast_text(app, "\n\n"_f0);
        
        
        if (app->debug_text_entity)
        {
            f32 s = Default_Text_Scale;
            v4 c = get_white();
            
            Entity *player = &world->player;
            String first_entity_str = stringf(scratch, "player dp x: %f y: %f z: %f",
                                              player->dp.x, player->dp.y, player->dp.z);
            fast_text_nl(app, first_entity_str, s, c);
        }
        
        
        
        
        
        String fps = stringf(scratch, "%.2fms  %.0ffps (gameplay dt: %.2fms)",
                             app->true_dt*1000.f, 1.f/app->true_dt, dt*1000.f);
        fast_text(app, fps);
        
        
        static_local f32 last_avg = 0;
        static_local f32 avg_accum = 0;
        static_local s32 avg_frame_count = 0;
        
        avg_accum += dt;
        avg_frame_count += 1;
        if (avg_frame_count > 33)
        {
            last_avg = avg_accum / (f32)avg_frame_count;
            avg_accum = 0;
            avg_frame_count = 0;
        }
        
        String avg_fps = stringf(scratch, " avg: %.2fms  %.0ffps", last_avg*1000.f, safe_ratio_0(1.f, last_avg));
        fast_text_nl(app, avg_fps, Default_Text_Scale, get_color(1, 0.7f, 0.92f));
        
        
        
        Camera *camera = (world->in_debug_camera ? &world->debug_camera : &world->camera);
        
        String pos_text = stringf(scratch,
                                  "chunk  x: %05lld  y: %05lld  z: %05lld\n"
                                  "offset x: %f  y: %f  z: %f",
                                  camera->at_chunk.x, camera->at_chunk.y, camera->at_chunk.z,
                                  camera->p.x, camera->p.y, camera->p.z);
        fast_text_nl(app, pos_text, Default_Text_Scale, get_cyan(0.2f, 0.9f));
        
        
        String angle_text = stringf(scratch, "ax: %.2f  ay: %.2f",
                                    camera->angle.x*(180/Pi32), camera->angle.y*(180/Pi32));
        fast_text_nl(app, angle_text);
        
        
        Vec3_S64 world_p = rel_coord_floor_to_world_coord(camera->at_chunk, camera->p);
        String world_pos = stringf(scratch, "world  x: %05lld  y: %05lld  z: %05lld",
                                   world_p.x, world_p.y, world_p.z);
        fast_text_nl(app, world_pos, Default_Text_Scale, get_blue(0.6f, 0.9f));
                                   
        
        
        v4 cm = get_red(0.8f);
        v4 cv = get_red(0.5f);
        f32 s = Default_Text_Scale;
        
        u64 quads = plat->stats.game_quads_last_frame;
        u64 triangles = quads*2;
        String triangle_text = stringf(scratch, "world triangles: %03lluk (%05llu)",
                                       triangles/1000, triangles);
        fast_text_nl(app, triangle_text, Default_Text_Scale, get_red(0.8f));
        
        fast_text(app, " mem: "_f0, s, cm);
        u64 current_mem = quads * (sizeof(Vert_Voxel) + sizeof(u32));
        fast_text(app, to_short_bytes_string(scratch, current_mem, 3), s, cv);
        fast_text(app, "/"_f0, s, cm);
        
        {
            u64 max_mem = plat->stats.total_vram_buffers_size;
            
            fast_text(app, to_short_bytes_string(scratch, max_mem, 3), s, cv);
            
            fast_text(app, " ("_f0, s, cm);
            fast_text(app, to_short_bytes_string(scratch, max_mem, 3), s, cv);
            fast_text_nl(app, ")"_f0, s, cm);
        }
        
        
        String render_text = stringf(scratch, "frame number: %09llu    render index: %02u / %02u",
                                     plat->frame_number, plat->render_buffer_index, Render_Buffered_Frames);
        fast_text_nl(app, render_text, s, get_cyan(0.6f));
        
        
        String longest_dt = stringf(scratch, "longest frame dt: %.2fms", app->longest_true_dt*1000.f);
        fast_text_nl(app, longest_dt, s, get_yellow(0, 0.7f));
        
        
        String sorts_string = stringf(scratch, "sorts: %lld", plat->stats.total_chunk_sorts_count);
        fast_text_nl(app, sorts_string, s, get_red());
        
        
        
        
        
        {
            s = 0.5f;
            cm = get_blue(0.9f);
            cv = get_blue(0.5f);
            
            
            
            Debug_Timing_Frame *frame = plat->stats.timing_frames + plat->stats.active_timing_frame;
            for (u64 timing_index = 1;
                 timing_index < frame->timing_count;
                 timing_index += 1)
            {
                Debug_Timing prev_timing = frame->timings[timing_index - 1];
                Debug_Timing timing = frame->timings[timing_index];
                
                f32 ms = 1000.f*time_elapsed(timing.time, prev_timing.time);
                
                fast_text(app, timing.text, s, cm);
                fast_text(app, ": "_f0, s, cm);
                fast_text(app, to_string(scratch, ms), s, cv);
                fast_text_nl(app, "ms"_f0, s, cm);
            }
        }
        
        
        
        
        
        
        String mouse_rel_text = stringf(scratch, "mouse rel x: %.1f    y: %.1f",
                                        input->mouse_rel.x, input->mouse_rel.y);
        fast_text_nl(app, mouse_rel_text, 0.5f, get_green(0.5f));
        
        
        if (input->mouse_rel.x || input->mouse_rel.y)
        {
            app->debug_last_mouse_move_frame = plat->frame_number + (u64)(1.f/dt);
            fast_text_nl(app, "MOUSE IS MOVING"_f0, 0.25f, V4(1,0.2f,0,0.3f));
        }
        
        if (app->debug_last_mouse_move_frame > plat->frame_number)
        {
            fast_text_nl(app, "MOUSE was moving recently!"_f0, 0.5f, V4(0.5f,1,0.4f,0.6f));
        }
    }
    
    
    
    
    
    
    {
        v2 crosshair_center = get_center(inner_rect);
        crosshair_center.x = round_f32(crosshair_center.x);
        crosshair_center.y = round_f32(crosshair_center.y);
        v2 dim = V2(Glyph_Side);
        Rect2 rect = rect_center_dim(crosshair_center, dim);
        Rect2 tex = glyph_tex_from_xy(2, 3);
        render_glyph_tex(rect, tex);
    }
    
}

















#if Def_Ship
static_function
#else
extern "C" __declspec(dllexport)
#endif
EXPORT_APP_API(export_app_api)
{
    App_Exports result = {};
    result.app_update = app_update;
    return result;
}
