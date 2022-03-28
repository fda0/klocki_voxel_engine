#include "klocki_platform_interface.h"


struct Win32_App_Code
{
    HMODULE app_dll;
    FILETIME last_dll_write_time;
    
    Export_App_Api *export_app_api;
    App_Exports api;
};

struct Win32_State
{
    Platform_Interface interface;
    HWND window;
    HDC window_dc;
    
    b32 mouse_is_locked;
    
    Win32_App_Code app_code;
    Platform_String exe_path;
    Platform_String exe_dir;
    Platform_String source_dll_path;
    Platform_String temp_dll_path;
};
