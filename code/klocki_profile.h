#if !defined(Def_Profile)
#  define Def_Profile 0
#endif

#if Def_Profile
#  if Def_Internal
#    error "Def_Profile + Def_Internal?"
#  endif
#  if Def_Slow
#    error "Def_Profile + Def_Slow?"
#  endif
#  if Def_SuperSlow
#    error "Def_Profile + Def_SuperSlow?"
#  endif
#endif


#if Def_Profile // Optional setup code for Superluminal profiler
#define PERFORMANCEAPI_ENABLED 1
#pragma comment(lib, "../libs/PerformanceAPI_MT.lib")
#pragma comment(lib, "advapi32.lib")
#include "PerformanceAPI_capi.h"
#endif
