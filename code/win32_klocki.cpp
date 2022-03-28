#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "opengl32.lib")
#include "win32_klocki.h"
#include "hidusage.h"


static_global Win32_State state32 = {};
static_global Platform_Interface *plat = {};

#include "klocki_shared.cpp"
#include "plat_win32_thread_context.cpp" // TODO: This too
#include "stf0_memory.h"
#include "plat_win32_helpers.cpp"
#include "klocki_render.cpp"
#include "win32_hot_reloading.cpp"



#include "win32_klocki_opengl.cpp"
#include "opengl_klocki.cpp"








//~
static_function void
win32_init_input()
{
    Input *input = &plat->input;
    dll_initialize_sentinel(&input->event_sentinel);
    input->event_count = 0;
}

static_function Input_Event *
win32_push_event()
{
    Input *input = &plat->input;
    
    Input_Event *result = push_struct_clear(get_frame_arena(), Input_Event);
    input->event_count += 1;
    break_at(input->event_count > 128); // <- just for debugging
    
    dll_insert_before(&input->event_sentinel, result);
    return result;
}

static_function Input_Event *
win32_push_event(b32 mouse_left)
{
    Input_Event *event = win32_push_event();
    event->mouse_left = mouse_left;
    return event;
}




//~
static_function LRESULT
win32_window_proc_message_callback(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    switch (message)
    {
        case WM_CLOSE: {
            os_exit_process(0);
            //plat->set_try_program_exit = true;
        } break;
        
        case WM_PAINT:
        {
            
            PAINTSTRUCT ps;
            BeginPaint(window, &ps);
#if 0
            plat->embedded_widget_count = 0;
            
            RECT window_rect;
            GetClientRect(window, &window_rect);
            s32 window_width = window_rect.right - window_rect.left;
            s32 window_height = window_rect.bottom - window_rect.top;
            
            do_app_layer_frame_work(window, window_width, window_height, state32.window_dc, nullptr);
#endif
            EndPaint(window, &ps);
        } break;
        
        
        case WM_ACTIVATEAPP:
        {
            plat->is_window_active = !!wParam;
        } break;
        
        
        case WM_SYSKEYDOWN:
        {
            b32 alt_down = lParam & 29;
            if (alt_down && wParam == VK_F4)
            {
                platform_exit_process(0);
            }
            
            return true;
        } break;
        
        case WM_SYSKEYUP:
        case WM_SYSCHAR:
        {
            return true;
        } break;
        
        
        case WM_INPUT:
        {
            RAWINPUT raw = {};
            UINT data_size = sizeof(raw); // in/out
            UINT res = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &raw, &data_size, sizeof(RAWINPUTHEADER));
            
            if (res != -1)
            {
                if (raw.header.dwType == RIM_TYPEMOUSE)
                {
                    f32 rel_x = (f32)raw.data.mouse.lLastX;
                    f32 rel_y = (f32)raw.data.mouse.lLastY;
                    
                    {
                        Input *input = &plat->input;
                        input->mouse_rel.x += rel_x;
                        input->mouse_rel.y += rel_y;
                    }
                }
            }
            else
            {
                db;
            }
            
            result = DefWindowProcW(window, message, wParam, lParam);
        } break;
        
        case WM_LBUTTONDOWN:
        {
            win32_push_event(true);
        } break;
        
        case WM_LBUTTONUP:
        {;
            win32_push_event(false);
        } break;
        
        case WM_MOUSEWHEEL:
        {
            s32 z_delta = GET_WHEEL_DELTA_WPARAM(wParam);
            plat->input.mouse_wheel += (z_delta / WHEEL_DELTA);
        } break;
        
        
        //~
        default:
        {
            result = DefWindowProcW(window, message, wParam, lParam);
        } break;
    }
    
    return result;
}









static_function void
win32_debug_section_timing(String text)
{
    Fence_ReadWrite();
    
    u32 frame_index = (plat->stats.active_timing_frame + 1) % array_count(plat->stats.timing_frames);
    Debug_Timing_Frame *frame = plat->stats.timing_frames + frame_index;
    
    assert(frame->timing_count < array_count(frame->timings));
    Debug_Timing *timing = frame->timings + frame->timing_count;
    frame->timing_count += 1;
    
    timing->time = time_perf();
    timing->text = text;
}

static_function void
win32_debug_section_timing_clear()
{
    u32 frame_index = (plat->stats.active_timing_frame + 1) % array_count(plat->stats.timing_frames);
    Debug_Timing_Frame *frame = plat->stats.timing_frames + frame_index;
    frame->timing_count = 0;
}



static_function void
win32_update_platform_interface()
{
    plat->frame_number += 1;
    plat->render_buffer_index = plat->frame_number % Render_Buffered_Frames;
    plat->stats.active_timing_frame = plat->frame_number % array_count(plat->stats.timing_frames);
    
    win32_debug_section_timing_clear();
    win32_debug_section_timing("init"_f0);
    
    
    assert(plat->render_buffer_index < array_count(gl_state.debug_buffers));
    assert(plat->render_buffer_index < array_count(gl_state.ui_buffers));
    plat->frame_ui = &gl_state.ui_buffers[plat->render_buffer_index].frame;
    plat->frame_debug = &gl_state.debug_buffers[plat->render_buffer_index].frame;
    plat->frame_line = &gl_state.line_buffers[plat->render_buffer_index].frame;
    
    
    RECT window_rect;
    GetClientRect(state32.window, &window_rect);
    plat->window_width = window_rect.right - window_rect.left;
    plat->window_height = window_rect.bottom - window_rect.top;
    
    dll_initialize_sentinel(&plat->input.event_sentinel);
}


static_function void
win32_update_button(Button *button, int virtual_key, b32 force_clear)
{
    button->was_down = button->down;
    
    if (force_clear)
    {
        button->down = false;
    }
    else
    {
        SHORT state = GetKeyState(virtual_key);
        button->down = ((state & Bit_15) != 0);
    }
}



static_function void
win32_update_and_render()
{
    profile_function();
    
    win32_update_platform_interface();
    
    {
        profile_scope("Win32 Poll input");
        Input *input = &plat->input;
        b32 clear = !plat->is_window_active;
        
        //POINT mpos;
        //GetCursorPos(&mpos);
        //ScreenToClient(state32.window, &mpos);
        //plat->input.mouse_abs = V2i(mpos.x, mpos.y);
        win32_update_button(&input->mouse_left, VK_LBUTTON, clear);
        win32_update_button(&input->mouse_right, VK_RBUTTON, clear);
        win32_update_button(&input->mouse_middle, VK_MBUTTON, clear);
        win32_update_button(&input->mouse_x1, VK_XBUTTON1, clear);
        win32_update_button(&input->mouse_x2, VK_XBUTTON2, clear);
        
        
        win32_update_button(&input->forward, 'W', clear);
        win32_update_button(&input->backward, 'S', clear);
        win32_update_button(&input->left, 'A', clear);
        win32_update_button(&input->right, 'D', clear);
        
        win32_update_button(&input->cam_up, VK_UP, clear);
        win32_update_button(&input->cam_down, VK_DOWN, clear);
        win32_update_button(&input->cam_left, VK_LEFT, clear);
        win32_update_button(&input->cam_right, VK_RIGHT, clear);
        
        win32_update_button(&input->jump, VK_SPACE, clear);
        win32_update_button(&input->sneak, VK_SHIFT, clear);
        
        win32_update_button(&input->f1, VK_F1, clear);
        win32_update_button(&input->f2, VK_F2, clear);
        win32_update_button(&input->f3, VK_F3, clear);
        win32_update_button(&input->f4, VK_F4, clear);
        win32_update_button(&input->f5, VK_F5, clear);
        win32_update_button(&input->f6, VK_F6, clear);
        win32_update_button(&input->f7, VK_F7, clear);
        win32_update_button(&input->f8, VK_F8, clear);
        win32_update_button(&input->f9, VK_F9, clear);
        win32_update_button(&input->f10, VK_F10, clear);
        win32_update_button(&input->f11, VK_F11, clear);
        
        win32_update_button(&input->n1, '1', clear);
        win32_update_button(&input->n2, '2', clear);
        win32_update_button(&input->n3, '3', clear);
        win32_update_button(&input->n4, '4', clear);
        win32_update_button(&input->n5, '5', clear);
        win32_update_button(&input->n6, '6', clear);
        win32_update_button(&input->n7, '7', clear);
        win32_update_button(&input->n8, '8', clear);
        win32_update_button(&input->n9, '9', clear);
        win32_update_button(&input->n0, '0', clear);
    }
    
    
    if (plat->mouse_in_locked_mode && plat->is_window_active)
    {
        RECT client_rect;
        GetClientRect(state32.window, &client_rect);
        MapWindowPoints(state32.window, nullptr, (POINT*)&client_rect, 2);
        
        b32 clip_res = ClipCursor(&client_rect);
        assert(clip_res);
        
        if (!state32.mouse_is_locked)
        {
            ShowCursor(false);
        }
        
        state32.mouse_is_locked = true;
    }
    else if (state32.mouse_is_locked)
    {
        ClipCursor(nullptr);
        ShowCursor(true);
        state32.mouse_is_locked = false;
    }
    
    
    
    
    win32_debug_section_timing("interface update"_f0);
    
    
    //os_sleep_ms(5);
    gl_setup_world_settings();
    win32_debug_section_timing("sleep"_f0);
    
    
    if (state32.app_code.api.app_update)
    {
        state32.app_code.api.app_update(&state32.interface);
    }
    win32_debug_section_timing("app update"_f0);
    
    
    
    
    gl_render_all(plat->render_buffer_index);
    win32_debug_section_timing("render all"_f0);
    
    
    
    
    //-
    Thread_Context *thread = platform_get_thread_context();
    
    for_u64(scratch_index, array_count(thread->scratch_arenas))
    {
        Arena *scratch = thread->scratch_arenas + scratch_index;
        assert(scratch->base);
        assert(scratch->position == 0);
    }
    
    reset_arena_position(get_frame_arena());
    win32_init_input();
    plat->input.mouse_rel = {};
    plat->input.mouse_wheel = 0;
    win32_debug_section_timing("clear stuff"_f0);
}






int WinMain(HINSTANCE instance, HINSTANCE prev_instance,
            LPSTR lpCmdLine, int nShowCmd)
{
    // init
    {
        plat = &state32.interface;
        plat->perm_arena = create_virtual_arena();
        
        plat->platform_data = platform_setup_data();
        Thread_Context *main_thread_ctx = push_thread_context(&plat->perm_arena);
        platform_set_thread_context(main_thread_ctx);
        
        
        LARGE_INTEGER large_perfomance_freq;
        b32 return_code_test = QueryPerformanceFrequency(&large_perfomance_freq);
        assert(return_code_test);
        plat->inv_perf_freq = 1.f / (f32)large_perfomance_freq.QuadPart;
        
        
        
        // set pointers to exported platform api
        {
            plat->api.create_mesh_buffer = gl_create_mesh_buffer;
            plat->api.finalize_mesh_buffer = gl_finalize_mesh_buffer;
            plat->api.delete_mesh_buffer = gl_delete_mesh_buffer;
            plat->api.set_world_transform = gl_set_world_transform;
            plat->api.render_mesh_buffer = gl_render_mesh_buffer;
            
            plat->api.create_texture_2d = gl_create_texture_2d;
            plat->api.create_texture_3d = gl_create_texture_3d;
            plat->api.fill_texture_3d = gl_fill_texture_3d;
            plat->api.generate_mipmap_3d = gl_generate_mipmap_3d;
            
            plat->api.sync_frame_rendering = gl_sync_frame_rendering;
        }
        
        
        
        win32_init_input();
        
        {
            state32.exe_path = platform_get_this_exe_path(&plat->perm_arena);
            Find_Index find_dir = str_index_of_reverse_from_table(state32.exe_path, L"/\\"_fplat);
            if (find_dir.found)
            {
                state32.exe_dir = state32.exe_path;
                state32.exe_dir.size = find_dir.index + 1;
            }
            else
            {
                state32.exe_dir = L"./"_fplat;
                assert(0);
            }
            
            
            state32.source_dll_path = str_concatenate(&plat->perm_arena, state32.exe_dir, L"klocki.dll"_fplat);
            state32.temp_dll_path = str_concatenate(&plat->perm_arena, state32.exe_dir, L"klocki_temp.dll"_fplat);
            win32_load_app_code();
        }
        
    }
    
    
    
    
#if 1 // @temp change current directory to data/ if user runs it from build/ by mistake
    {
        Scratch scratch(0);
        Platform_String cwd = platform_get_current_directory(scratch);
        String cwd8 = win32_push_platform_string_to_string8(scratch, cwd);
        Directory d = directory_from_string(scratch, cwd8);
        if (filesystem_str_equals(d.last(), "build"_f0))
        {
            d.name_count -= 1;
        }
        if (!filesystem_str_equals(d.last(), "data"_f0))
        {
            d = directory_append(scratch, d, "data"_f0);
        }
        String new_dir_str = to_string(scratch, d);
        Platform_String new_cwd = win32_push_string8_to_platform_string(scratch, new_dir_str);
        platform_set_current_directory(new_cwd);
    }
#endif
    
    
    
    
    
    
    
    
    // create window
#define WINDOW_CLASS L"Original Block Game"
    
    WNDCLASSW window_class = {0};
    window_class.style         = 0;
    window_class.lpfnWndProc   = win32_window_proc_message_callback;
    window_class.hInstance     = instance;
    //window_class.hIcon = LoadIconW(instance, MAKEINTRESOURCEW(1));
    window_class.hCursor       = LoadCursorA(0, IDC_ARROW);
    window_class.lpszClassName = WINDOW_CLASS;
    
    
    ATOM atom = RegisterClassW(&window_class);
    if (!atom)
    {
        platform_throw_error_and_exit("RegisterClassW failed");
    }
    
    
#if 0
    int win_width = CW_USEDEFAULT;
    int win_height = CW_USEDEFAULT;
#else
    int win_width = 1920;
    int win_height = 1080;
#endif
    
    DWORD style = WS_OVERLAPPEDWINDOW; // NOTE(f0): needed for many default window behaviors even with custom window
    state32.window = CreateWindowW(WINDOW_CLASS, L"Original Block Game", style,
                                   CW_USEDEFAULT, CW_USEDEFAULT,
                                   win_width, win_height,
                                   0, 0, instance, 0);
    if (!state32.window)
    {
        platform_throw_error_and_exit("CreateWindowExW failed");
    }
    
    
    
    // register mouse to get WM_INPUT messages
    {
        RAWINPUTDEVICE rid[1];
        rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC; 
        rid[0].usUsage = HID_USAGE_GENERIC_MOUSE; 
        rid[0].dwFlags = RIDEV_INPUTSINK;
        rid[0].hwndTarget = state32.window;
        RegisterRawInputDevices(rid, 1, sizeof(rid[0]));
    }
    
    
    state32.window_dc = GetDC(state32.window);
    HGLRC opengl_rc = win32_gl_initialize(state32.window_dc);
    if (!opengl_rc)
    {
        platform_throw_error_and_exit("Opengl_rc initialization failed");
    }
    gl_setup_shaders();
    
    
    ShowWindow(state32.window, SW_SHOW);
    
    
    
    
    for(;;)
    {
        win32_check_for_dll_update();
        
        
#if 0
        if (plat->set_minimize)
        {
            ShowWindow(state32.window, SW_MINIMIZE);
        }
        else if (plat->set_toggle_maximize)
        {
            if (IsZoomed(state32.window))
            {
                ShowWindow(state32.window, SW_RESTORE);
            }
            else
            {
                ShowWindow(state32.window, SW_MAXIMIZE);
            }
        }
#endif
        
        {
            profile_scope("Win32 process Windows messages");
            
            MSG msg;
            while (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
        }
        
        win32_update_and_render();
    }
}


#if Def_Ship
#include "klocki.cpp"
#endif