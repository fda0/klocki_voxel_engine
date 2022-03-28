#if Def_Profile // Optional setup code for Superluminal profiler

#define profile_block_start_color(BlockName, Color) PerformanceAPI_BeginEvent(BlockName, nullptr, Color);
#define profile_block_end() PerformanceAPI_EndEvent();
#define profile_scope_color(ScopeName, Color) auto glue(profile_scope_var__, This_Line_S32) = Profile_Scope(ScopeName, Color)


#define profile_block_start(BlockName) profile_block_start_color(BlockName,\
(((This_Line_S32*Counter_Macro*2'073'742'267U)&0xff'3f'3f'00) | 0x40'00'00'ff))

#define profile_function() profile_scope_color(This_Function,\
(((This_Line_S32*Counter_Macro*2'073'742'267U)&0x3f'ff'3f'00) | 0x00'40'00'ff))

#define profile_scope(ScopeName) profile_scope_color(ScopeName,\
(((This_Line_S32*Counter_Macro*2'103'742'009U)&0x3f'3f'ff'00) | 0x00'00'40'ff))


struct Profile_Scope
{
    Profile_Scope(const char *scope_name, u32 color = PERFORMANCEAPI_DEFAULT_COLOR)
    {
        profile_block_start_color(scope_name, color);
    }
    
    ~Profile_Scope()
    {
        profile_block_end();
    }
};


#else
#define profile_function()
#define profile_scope(...)
#endif
