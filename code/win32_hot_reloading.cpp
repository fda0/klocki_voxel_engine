#if Def_Ship
// single exe, no hot reloading path

// forward declaration
static_function EXPORT_APP_API(export_app_api);

static_function void
win32_load_app_code()
{
    state32.app_code = {};
    state32.app_code.api = export_app_api();
}

#define win32_check_for_dll_update()



#else
// hot dll reloading path



static_function b32
win32_is_file_unlocked(Platform_String path)
{
    b32 output = false;
    
    HANDLE file_handle = CreateFileW(path.wstr, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (file_handle != INVALID_HANDLE_VALUE)
    {
        if (ReadFile(file_handle, 0, 0, 0, 0)) {
            output = true;
        }
        CloseHandle(file_handle);
    }
    return output;
}

static_function FILETIME
win32_get_last_write_time(char *file_name)
{
    FILETIME lastWriteTime = {};
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (GetFileAttributesEx(file_name, GetFileExInfoStandard, &data)) {
        lastWriteTime = data.ftLastWriteTime;
    }
    return lastWriteTime;
}

static_function Win32_App_Code
win32_load_app_code_(Platform_String source_dll, Platform_String temp_dll_target)
{
    Win32_App_Code result = {};
    result.last_dll_write_time = platform_get_file_mod_time(source_dll);
    
    b32 copy_result = platform_copy_file(source_dll, temp_dll_target, true);
    if (copy_result)
    {
        result.app_dll = LoadLibraryW(temp_dll_target.wstr);
        if (result.app_dll)
        {
            result.export_app_api = (Export_App_Api *)GetProcAddress(result.app_dll, "export_app_api");
            if (result.export_app_api)
            {
                result.api = result.export_app_api();
            }
        }
    }
    
    return result;
}

static_function void
win32_load_app_code()
{
    state32.app_code = win32_load_app_code_(state32.source_dll_path, state32.temp_dll_path);
    
    if (!state32.app_code.api.app_update)
    {
        Scratch scratch(0);
        String source_dll = win32_push_platform_string_to_string8(scratch, state32.source_dll_path);
        char *message = cstrf(scratch, "Failed to load DLL %.*s", string_expand(source_dll));
        platform_throw_message(message, NMB_TypeOk);
        os_exit_process(1);
    }
}

//-
static_function void
win32_unload_app_code_(Win32_App_Code *code)
{
    if (code->app_dll)
    {
        FreeLibrary(code->app_dll);
        code->app_dll = nullptr;
    }
    
    code->export_app_api = nullptr;
    code->api = {};
}

static_function void
win32_unload_app_code()
{
    win32_unload_app_code_(&state32.app_code);
}



//-
static_function void
win32_check_for_dll_update()
{
    File_Mod_Time new_dll_write_time = platform_get_file_mod_time(state32.source_dll_path);
    if (CompareFileTime(&new_dll_write_time, &state32.app_code.last_dll_write_time))
    {
        if (win32_is_file_unlocked(state32.source_dll_path))
        {
            plat->dll_reload_pending = true;
            
            if (plat->dll_reload_app_ready)
            {
                win32_unload_app_code();
                //win32_set_worker_job_function(nullptr);
                win32_load_app_code();
                
                if (!state32.app_code.export_app_api)
                {
                    platform_throw_error_and_exit("Failed to load app code from dll");
                }
                
                
                plat->dll_reload_pending = false;
                plat->dll_reload_app_ready = false;
                plat->dll_reload_unhandled = true;
            }
        }
    }
}

#endif