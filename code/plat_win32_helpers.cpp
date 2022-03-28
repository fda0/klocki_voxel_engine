#pragma once


enum Native_Message_Box_Type
{
    NMB_TypeOk = MB_OK,
    NMB_TypeYesNo = MB_YESNO,
    NMB_TypeCancelTryAgainContinue = MB_CANCELTRYCONTINUE,
    NMB_TypeYesNoCancel = MB_YESNOCANCEL,
};
enum Native_Message_Box_Result
{
    NMB_None = 0,
    NMB_Ok = IDOK,
    NMB_Yes = IDYES,
    NMB_No = IDNO,
    NMB_Cancel = IDCANCEL,
    NMB_TryAgain = IDTRYAGAIN,
    NMB_Continue = IDCONTINUE,
};
static_function s32
platform_throw_message(char *message, Native_Message_Box_Type box_type)
{
    MSGBOXPARAMSA params = {};
    params.cbSize = sizeof(params);
    params.lpszText = message;
    
    params.dwStyle = MB_ICONERROR | MB_TASKMODAL;
    params.dwStyle |= box_type;
    if (box_type == NMB_TypeYesNo) { params.dwStyle |= MB_DEFBUTTON2; }
    
    s32 message_res = MessageBoxIndirectA(&params);
    assert(message_res);
    return message_res;
}

static_function void
platform_exit_process(s32 code)
{
    ExitProcess(code);
}

static_function void
platform_throw_error_and_exit(char *message)
{
    debug_break();
    platform_throw_message(message, NMB_TypeOk);
    platform_exit_process(1);
}










//~ utf8 -> utf16
static_function Platform_String
win32_push_string8_to_platform_string(Arena *a, String source)
{
    Platform_String result = {};
    
    char *source_str = (char*)source.str;
    s32 source_size = safe_truncate_to_s32(source.size);
    
    s32 required_size = MultiByteToWideChar(CP_UTF8, 0,
                                            source_str, source_size,
                                            0, 0);
    
    if (required_size > 0)
    {
        wchar_t *result_buffer = push_array(a, wchar_t, required_size + 1);
        s32 result_size = MultiByteToWideChar(CP_UTF8, 0,
                                              source_str, source_size,
                                              result_buffer, required_size);
        
        if (result_size > 0)
        {
            result = {result_buffer, safe_truncate_to_u64(result_size)};
            result.wstr[result_size] = 0;
        }
    }
    
    return result;
}


static_function String
win32_push_platform_string_to_string8(Arena *a, Platform_String source)
{
    String result = {};
    
    s32 source_size = safe_truncate_to_s32(source.size);
    
    s32 required_size = WideCharToMultiByte(CP_UTF8, 0,
                                            source.wstr, source_size,
                                            0, 0,
                                            nullptr, false);
    
    if (required_size > 0)
    {
        char *result_buffer = push_array(a, char, required_size);
        s32 result_size = WideCharToMultiByte(CP_UTF8, 0,
                                              source.wstr, source_size,
                                              result_buffer, required_size,
                                              nullptr, false);
        
        if (result_size > 0)
        {
            result = get_string(result_buffer, safe_truncate_to_u64(result_size));
        }
    }
    
    return result;
}





static_function s32
win32_string8_to_string16_count_characters(String str8)
{
    s32 count = MultiByteToWideChar(CP_UTF8, 0,
                                    (char *)str8.str, safe_truncate_to_s32(str8.size),
                                    0, 0);
    
    assert(count > 0);
    return count;
}

static_function s32
win32_string8_to_string16(String source_str8, wchar_t *memory, s32 memory_size)
{
    s32 count = MultiByteToWideChar(CP_UTF8, 0,
                                    (char *)source_str8.str, safe_truncate_to_s32(source_str8.size),
                                    memory, memory_size);
    
    assert(count > 0);
    return count;
}

static_function wchar_t *
win32_push_string8_to_cstr16(Arena *a, String source_utf8)
{
    s32 required_count = win32_string8_to_string16_count_characters(source_utf8);
    required_count += 1;
    
    wchar_t *memory = push_array(a, wchar_t, required_count);
    
    s32 written_count = win32_string8_to_string16(source_utf8, memory, required_count);
    assert(required_count == written_count+1);
    memory[required_count-1] = 0;
    
    return memory;
}




//~ utf16 -> utf8
static_function s32
win32_cstr16_to_cstr8_count_characters(wchar_t *wstr)
{
    s32 count = WideCharToMultiByte(CP_UTF8, 0,
                                    wstr, -1,
                                    0, 0,
                                    nullptr, false);
    assert(count > 0);
    return count;
}

static_function s32
win32_cstr16_to_cstr8(wchar_t *source_wstr,
                      char *output, u64 output_size)
{
    s32 count = WideCharToMultiByte(CP_UTF8, 0,
                                    source_wstr, -1,
                                    output, safe_truncate_to_s32(output_size),
                                    nullptr, false);
    
    assert(count > 0);
    return count;
}

static_function char *
win32_push_cstr16_to_cstr8(Arena *a, wchar_t *wstr_source)
{
    s32 required_count = win32_cstr16_to_cstr8_count_characters(wstr_source);
    char *result = push_array(a, char, required_count);
    s32 result_count = win32_cstr16_to_cstr8(wstr_source, result, required_count);
    
    assert(required_count == result_count);
    assert(required_count && result[required_count-1] == 0);
    return result;
}


//-
static_function s32
win32_string16_to_string8_count_characters(String16 string16)
{
    s32 count = 0;
    if (string16.size > 0)
    {
        count = WideCharToMultiByte(CP_UTF8, 0,
                                    (wchar_t *)string16.str, safe_truncate_to_s32(string16.size),
                                    0, 0,
                                    nullptr, false);
        assert(count > 0);
    }
    return count;
}

static_function s32
win32_string16_to_string8(String16 source16,
                          u8 *output_str, u64 output_size)
{
    s32 count = WideCharToMultiByte(CP_UTF8, 0,
                                    (wchar_t *)source16.str, safe_truncate_to_s32(source16.size),
                                    (char *)output_str, safe_truncate_to_s32(output_size),
                                    nullptr, false);
    assert(count > 0);
    return count;
}

static_function String
win32_push_string16_to_string8(Arena *a, String16 source16)
{
    String result = {};
    if (source16.size > 0)
    {
        s32 required_count = win32_string16_to_string8_count_characters(source16);
        if (required_count > 0)
        {
            result = push_string(a, required_count);
            s32 result_count = win32_string16_to_string8(source16, result.str, result.size);
            
            assert(required_count == result_count);
            assert(result.size && result.str[result.size-1] != 0);
        }
        else
        {
            assert(0);
        }
    }
    return result;
}























//~
static_function Platform_String
platform_get_current_directory(Arena *a)
{
    u32 buffer_size = GetCurrentDirectoryW(0, nullptr);
    wchar_t *buffer = push_array(a, wchar_t, buffer_size);
    u32 length = GetCurrentDirectoryW(buffer_size, buffer);
    
    Platform_String result = {buffer, (u64)length};
    return result;
}

static_function void
platform_set_current_directory(Platform_String path)
{
    SetCurrentDirectoryW(path.wstr);
}

static_function void
platform_set_current_directory(String path)
{
    Scratch scratch(0);
    Platform_String plat_string = win32_push_string8_to_platform_string(scratch, path);
    SetCurrentDirectoryW(plat_string.wstr);
}


struct Platform_Change_Current_Directory_Scope_
{
    Platform_String previous_directory;
    
    Platform_Change_Current_Directory_Scope_(String new_directory)
    {
        previous_directory = platform_get_current_directory(get_frame_arena());
        platform_set_current_directory(new_directory);
    }
    
    ~Platform_Change_Current_Directory_Scope_()
    {
        platform_set_current_directory(previous_directory);
    }
};

#define platform_change_current_directory_scope(NewDirectory) \
Platform_Change_Current_Directory_Scope_ glue(change_cwd_scope_, This_Line_S32)(NewDirectory)







//~
static_function File_Mod_Time
platform_get_file_mod_time(Platform_String path)
{
    HANDLE file_handle = CreateFileW(path.wstr, GENERIC_READ,
                                     (FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE),
                                     nullptr, OPEN_EXISTING, 0,  nullptr);
    
    File_Mod_Time mod_time = {};
    
    if (file_handle != INVALID_HANDLE_VALUE)
    {
        b32 get_time_res = GetFileTime(file_handle, nullptr, nullptr, &mod_time);
        break_at(!get_time_res);
        CloseHandle(file_handle);
    }
    
    return mod_time;
}


static_function File_Mod_Time
platform_get_file_mod_time(String path)
{
    Scratch scratch(0);
    Platform_String path_wide = win32_push_string8_to_platform_string(scratch, path);
    File_Mod_Time result = platform_get_file_mod_time(path_wide);
    return result;
}









//~
static_function File_Write_Operation
platform_write_to_file(String path_string, String content)
{
    profile_function();
    
    Scratch scratch(0);
    wchar_t *wstr_path = win32_push_string8_to_cstr16(scratch, path_string);
    
    retry_file_write_label:;
    HANDLE file_handle = CreateFileW(wstr_path, GENERIC_WRITE, (FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE),
                                     nullptr, CREATE_ALWAYS, 0, nullptr);
    
    File_Write_Operation result = {};
    
    if (file_handle != INVALID_HANDLE_VALUE)
    {
        while (content.size)
        {
            u64 this_part_size = gigabytes(1);
            this_part_size = pick_smaller(this_part_size, content.size);
            
            DWORD to_write = safe_truncate_to_u32(this_part_size);
            DWORD bytes_written = 0;
            
            b32 write_res = WriteFile(file_handle, content.str, to_write, &bytes_written, nullptr);
            
            if (!write_res || (bytes_written != this_part_size))
            {
                assert(0);
                CloseHandle(file_handle);
                goto report_file_save_error_label;
            }
            
            content = str_skip(content, this_part_size);
        }
        
        
        b32 flush_res = FlushFileBuffers(file_handle);
        assert(flush_res);
        
        b32 get_time_res = GetFileTime(file_handle, nullptr, nullptr, &result.time);
        assert(get_time_res);
        
        b32 close_res = CloseHandle(file_handle);
        assert(close_res);
        
        result.success = true;
    }
    else
    {
        report_file_save_error_label:;
        
        // TODO: delete this; temporary retry policy (for testing)
        s32 message_res = platform_throw_message("Failed to save to file, try again?", NMB_TypeYesNo);
        switch (message_res)
        {
            default: assert(0); // fallthrough
            case NMB_No: break;
            case NMB_Yes: goto retry_file_write_label; break;
        }
    }
    
    
    return result;
}







//~
static_function b32
platform_check_if_file_exists(String path_string, b32 check_if_directory_exists_instead = false)
{
    profile_function();
    
    Scratch scratch(0);
    wchar_t *wstr_path = win32_push_string8_to_cstr16(scratch, path_string);
    
    DWORD attributes = GetFileAttributesW(wstr_path);
    b32 result = ((attributes != INVALID_FILE_ATTRIBUTES) &&
                  ((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0));
    return result;
}


static_function b32
platform_check_if_directory_exists(String path_string)
{
    profile_function();
    
    Scratch scratch(0);
    wchar_t *wstr_path = win32_push_string8_to_cstr16(scratch, path_string);
    
    DWORD attributes = GetFileAttributesW(wstr_path);
    b32 result = ((attributes != INVALID_FILE_ATTRIBUTES) &&
                  ((attributes & FILE_ATTRIBUTE_DIRECTORY) != 0));
    return result;
}


static_function b32
platform_check_if_directory_exists(Directory directory)
{
    Scratch scratch(0);
    String directory_string = to_string(scratch, directory);
    return platform_check_if_directory_exists(directory_string);
}




static_function File_Read_Operation
platform_push_entire_file_read(Arena *data_arena, String path_string)
{
    profile_function();
    
    Scratch scratch(data_arena);
    wchar_t *wstr_path = win32_push_string8_to_cstr16(scratch, path_string);
    HANDLE file_handle = CreateFileW(wstr_path, GENERIC_READ, (FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE),
                                     nullptr, OPEN_EXISTING, 0, nullptr);
    
    File_Read_Operation result = {};
    
    if (file_handle != INVALID_HANDLE_VALUE)
    {
        result.open_succcess = true;
        
        LARGE_INTEGER large_int_size = {};
        b32 get_size_res = GetFileSizeEx(file_handle, &large_int_size);
        assert(get_size_res);
        
        if (get_size_res)
        {
            u64 size = large_int_size.QuadPart;
            result.content = push_string(data_arena, size);
            u64 read_bytes_sum = 0;
            
            while (read_bytes_sum < size)
            {
                u64 this_part_size = gigabytes(1);
                this_part_size = pick_smaller(this_part_size, (size - read_bytes_sum));
                
                DWORD to_read = safe_truncate_to_u32(this_part_size);
                DWORD bytes_read = 0;
                b32 read_res = ReadFile(file_handle, (result.content.str + read_bytes_sum), to_read, &bytes_read, nullptr);
                
                if (!read_res || (bytes_read != this_part_size))
                {
                    result.content = {};
                    CloseHandle(file_handle);
                    assert(0);
                    goto end_of_function_label;
                }
                
                read_bytes_sum += this_part_size;
            }
            
            result.read_success = true;
        }
        
        
        b32 get_time_res = GetFileTime(file_handle, nullptr, nullptr, &result.time);
        assert(get_time_res);
        
        b32 close_res = CloseHandle(file_handle);
        assert(close_res);
    }
    
    
    end_of_function_label:;
    return result;
}





//~
static_function File_Write_Operation
platform_create_file(Arena *data_arena, String path_string)
{
    profile_function();
    
    Scratch scratch(data_arena);
    wchar_t *wstr_path = win32_push_string8_to_cstr16(scratch, path_string);
    
    HANDLE file_handle = CreateFileW(wstr_path, GENERIC_WRITE, (FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE),
                                     nullptr, CREATE_ALWAYS, 0, nullptr);
    
    File_Write_Operation result = {};
    
    if (file_handle != INVALID_HANDLE_VALUE)
    {
        b32 get_time_res = GetFileTime(file_handle, nullptr, nullptr, &result.time);
        assert(get_time_res);
        
        // get normalized file path
        {
            DWORD required_length = GetFinalPathNameByHandleW(file_handle, nullptr, 0, 0);
            if (required_length)
            {
                wchar_t *wchar_buffer = push_array(scratch, wchar_t, required_length);
                DWORD res_length = GetFinalPathNameByHandleW(file_handle, wchar_buffer, required_length, 0);
                
                if (res_length > 0)
                {
                    String normalized_path = win32_push_string16_to_string8(data_arena, get_string16(wchar_buffer));
                    assert(normalized_path.size > 0 && normalized_path.str[normalized_path.size - 1] != 0);
                    result.normalized_path = normalized_path;
                }
            }
            
            if (!result.normalized_path.size)
            {
                result.normalized_path = path_string;
                assert(0);
            }
        }
        
        
        b32 close_res = CloseHandle(file_handle);
        assert(close_res);
        
        result.success = true;
    }
    
    return result;
}




//~
static_function b32
platform_delete_file(String path_string)
{
    profile_function();
    
    Scratch scratch(0);
    wchar_t *wstr_path = win32_push_string8_to_cstr16(scratch, path_string);
    
    b32 result = DeleteFileW(wstr_path);
    return result;
}


//~
struct Platform_Normalized_Path_Result
{
    String path;
    b32 success;
};

static_function Platform_Normalized_Path_Result
platform_get_normalized_path(Arena *data_arena, Platform_String platform_path_string)
{
    // TODO(f0): compress with below or delete
    profile_function();
    Platform_Normalized_Path_Result result = {};
    
    HANDLE file = CreateFileW(platform_path_string.wstr, 0, FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,
                              nullptr, OPEN_EXISTING, 0, nullptr);
    
    if (file != INVALID_HANDLE_VALUE)
    {
        DWORD required_length = GetFinalPathNameByHandleW(file, nullptr, 0, 0);
        if (required_length)
        {
            Scratch scratch(data_arena);
            wchar_t *wchar_buffer = push_array(scratch, wchar_t, required_length);
            DWORD res_length = GetFinalPathNameByHandleW(file, wchar_buffer, required_length, 0);
            
            if (res_length > 0)
            {
                result.path = win32_push_string16_to_string8(data_arena, get_string16(wchar_buffer));
                assert(result.path.size > 0 &&
                       result.path.str[result.path.size - 1] != 0);
                result.success = true;
            }
        }
        
        CloseHandle(file);
    }
    
    return result;
}

static_function Platform_Normalized_Path_Result
platform_get_normalized_path(Arena *data_arena, String path_string)
{
    Scratch scratch(data_arena);
    Platform_String plat_path_string = win32_push_string8_to_platform_string(scratch, path_string);
    
    Platform_Normalized_Path_Result result = platform_get_normalized_path(data_arena, plat_path_string);
    return result;
}





//~
static_function void
platform_list_all_files(Arena *data_arena, String_Dll_Node *output_sentinel,
                        Platform_String directory_wide, b32 recurse_into_directories)
{
    Scratch scratch(data_arena);
    
    assert(!directory_wide.size || directory_wide.wstr[directory_wide.size-1] == '\\');
    Platform_String wildcard_path = str_concatenate(scratch, directory_wide, L"*"_fplat);
    
    
    WIN32_FIND_DATAW data = {};
    HANDLE find_handle = FindFirstFileW(wildcard_path.wstr, &data);
    if (find_handle != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            {
                //arena_scope(scratch); // TODO(f0): enable when scratch != data_arena
                Platform_String file_name = get_platform_string(data.cFileName);
                Platform_String full_path = str_concatenate(scratch, directory_wide, file_name);
                
                String_Dll_Node *node = push_struct(data_arena, String_Dll_Node);
                auto normalized = platform_get_normalized_path(data_arena, full_path);
                if (normalized.success)
                {
                    node->string = normalized.path;
                    assert(node->string.size);
                    dll_insert_before(output_sentinel, node);
                }
            }
            else
            {
                if (recurse_into_directories)
                {
                    Platform_String next_directory = get_platform_string(data.cFileName);
                    
                    //if (!equals(next_directory, L"."_fplat) && !equals(next_directory, L".."_fplat))
                    if (!str_starts_with(next_directory, L"."_fplat))
                    {
                        assert(next_directory.size &&
                               next_directory.wstr[next_directory.size-1] != '\\' &&
                               next_directory.wstr[next_directory.size-1] != '/');
                        
                        Platform_String next_wildcard_directory = str_concatenate(scratch, directory_wide, next_directory, L"\\"_fplat);
                        platform_list_all_files(data_arena, output_sentinel,
                                                next_wildcard_directory, recurse_into_directories);
                    }
                }
            }
            
        } while (FindNextFileW(find_handle, &data) != 0);
        FindClose(find_handle);
    }
}


static_function String_Dll_Node *
platform_list_all_files(Arena *data_arena, Directory directory, b32 recurse_into_directories)
{
    String_Dll_Node *result_sentinel = push_struct_clear(data_arena, String_Dll_Node);
    dll_initialize_sentinel(result_sentinel);
    
    Scratch scratch(data_arena);
    Platform_String directory_wide = win32_push_string8_to_platform_string(scratch, to_string(scratch, directory));
    platform_list_all_files(data_arena, result_sentinel, directory_wide, recurse_into_directories);
    
    return result_sentinel;
}






//~
static_function b32
platform_directory_exists(Directory directory)
{
    Scratch scratch(0);
    Platform_String directory_wide = win32_push_string8_to_platform_string(scratch, to_string(scratch, directory));
    
    DWORD attributes = GetFileAttributesW(directory_wide.wstr);
    b32 result = ((attributes != INVALID_FILE_ATTRIBUTES) &&
                  ((attributes & FILE_ATTRIBUTE_DIRECTORY) != 0));
    return result;
}


//~
static_function s32
platform_get_core_count()
{
    SYSTEM_INFO info = {};
    GetSystemInfo(&info);
    s32 result = info.dwNumberOfProcessors;
    return result;
}



//~
static_function b32
platform_copy_file(Platform_String source, Platform_String destination, b32 overwrite)
{
    b32 fail_if_exists = !overwrite;
    b32 result = CopyFileW(source.wstr, destination.wstr, fail_if_exists);
    return result;
}

static_function b32
platform_copy_file(String source, String destination, b32 overwrite)
{
    Scratch scratch(0);
    
    Platform_String source_wide = win32_push_string8_to_platform_string(scratch, source);
    Platform_String destination_wide = win32_push_string8_to_platform_string(scratch, destination);
    b32 result = platform_copy_file(source_wide, destination_wide, overwrite);
    return result;
}



//~
static_function Platform_String
platform_get_this_exe_path(Arena *arena)
{
    Scratch scratch(arena);
    DWORD buffer_count = 0;
    DWORD buffer_alloc_count = kilobytes(1);
    wchar_t *buffer = push_array(scratch, wchar_t, buffer_alloc_count);
    
    Platform_String result = {};
    
    for_u64(retry_index, 2)
    {
        buffer_count += buffer_alloc_count;
        
        DWORD len = GetModuleFileNameW(nullptr, buffer, buffer_count);
        
        auto err = GetLastError();
        if (len == 0 || err == ERROR_INSUFFICIENT_BUFFER)
        {
            debug_break();
        }
        else
        {
            result = push_copy(arena, get_platform_string(buffer, len));
            break;
        }
        
        buffer_alloc_count = kilobytes(32); // this will result in 33k character buffer
        // 32,767 characters is "approximate max windows path" according to msdn
        push_array(scratch, wchar_t, buffer_alloc_count);
    }
    
    return result;
}

