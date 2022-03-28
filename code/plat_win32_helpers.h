typedef FILETIME File_Mod_Time;

struct File_Write_Operation
{
    b32 success;
    File_Mod_Time time;
    String normalized_path;
};

struct File_Read_Operation
{
    b32 open_succcess;
    b32 read_success;
    String content;
    File_Mod_Time time;
};


struct Platform_String
{
    wchar_t *wstr; // always null terminated
    u64 size;
};






static_function Platform_String
operator "" _fplat(const wchar_t *str, size_t size)
{
    Platform_String result = {(wchar_t *)str, (u64)size};
    return result;
}


static_function String16
get_string16(Platform_String platform_string)
{
    // NOTE(f0): On windows Platform_String are String16;
    // But win32 Platform_String needs to be zero terminated after the size
    // while String16 do not have such requirement.
    String16 result = {
        (u16 *)platform_string.wstr,
        platform_string.size
    };
    return result;
}

static_function Platform_String
get_platform_string(wchar_t *wstr)
{
    Platform_String result = {wstr, 0};
    while (*wstr)
    {
        wstr += 1;
        result.size += 1;
    }
    return result;
}

static_function Platform_String
get_platform_string(wchar_t *wstr, u64 len)
{
    Platform_String result = {wstr, len};
    return result;
}





//~
static_function b32
equals(File_Mod_Time a, File_Mod_Time b)
{
    b32 result = ((a.dwLowDateTime == b.dwLowDateTime) && (a.dwHighDateTime == b.dwHighDateTime));
    return result;
}






//~
static_function Platform_String
push_copy(Arena *a, Platform_String source)
{
    Platform_String result = {};
    result.wstr = push_array(a, wchar_t, source.size + 1);
    result.size = source.size;
    if (source.size)
    {
        copy_array(result.wstr, source.wstr, wchar_t, source.size);
    }
    
    result.wstr[result.size] = 0;
    return result;
}


static_function b32
equals(Platform_String a, Platform_String b)
{
    b32 result = (a.size == b.size);
    if (result)
    {
        for_u64(index, a.size)
        {
            if (a.wstr[index] != b.wstr[index])
            {
                result = false;
                break;
            }
        }
    }
    return result;
}

static_function b32
str_starts_with(Platform_String haystack, Platform_String needle)
{
    b32 result = false;
    if (haystack.size >= needle.size && needle.size)
    {
        u64 index = 0;
        for (;;)
        {
            if (index == needle.size)
            {
                result = true;
                break;
            }
            
            if (haystack.wstr[index] != needle.wstr[index])
            {
                break;
            }
            
            index += 1;
        }
    }
    
    return result;
}


static_function b32
str_ends_with(Platform_String haystack, Platform_String needle)
{
    b32 result = false;
    if (haystack.size >= needle.size && needle.size)
    {
        u64 index = 0;
        for (;;)
        {
            if (index == needle.size)
            {
                result = true;
                break;
            }
            
            if (haystack.wstr[haystack.size - index - 1] != needle.wstr[needle.size - index - 1])
            {
                break;
            }
            
            index += 1;
        }
    }
    
    return result;
}



static_function Platform_String
str_concatenate(Arena *a, Platform_String first, Platform_String second)
{
    Platform_String result = {};
    result.size = first.size + second.size;
    result.wstr = push_array(a, wchar_t, result.size + 1);
    
    copy_array(result.wstr,              first.wstr,  wchar_t, first.size);
    copy_array(result.wstr + first.size, second.wstr, wchar_t, second.size);
    result.wstr[result.size] = 0;
    
    return result;
}

static_function Platform_String
str_concatenate(Arena *a, Platform_String first, Platform_String second, Platform_String third)
{
    Platform_String result = {};
    result.size = first.size + second.size + third.size;
    result.wstr = push_array(a, wchar_t, result.size + third.size + 1);
    
    copy_array(result.wstr,                            first.wstr,  wchar_t, first.size);
    copy_array(result.wstr + first.size,               second.wstr, wchar_t, second.size);
    copy_array(result.wstr + first.size + second.size, third.wstr,  wchar_t, third.size);
    result.wstr[result.size] = 0;
    
    return result;
}



static_function Find_Index
str_index_of_reverse_from_table(Platform_String haystack, Platform_String table)
{
    Find_Index result = {};
    
    for_s64_rev(haystack_index, haystack.size)
    {
        wchar_t h = haystack.wstr[haystack_index];
        
        for_u64(table_index, table.size)
        {
            wchar_t t = table.wstr[table_index];
            if (h == t)
            {
                result.index = haystack_index;
                result.found = true;
                return result;
            }
        }
    }
    
    return result;
}
