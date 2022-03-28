#pragma once


/*
    This file is a big blob of code that I share across my codebases.
    The plan was to replace Clib with it - and its not far off for my uses.
    It has proven to be very useful for starting new projects quickly but it became a little bloated with time.
    I'm hoping to retire it and leave only the best parts of it for my future codebases.
*/


















/* ======================= #define guide ======================
Switches that can be defined manually:
	> Def_Internal:
		0 - Build for public relrease
		1 - Build for developers only
	> Def_Slow:
		0 - No slow code allowed!
		1 - Slow code is welcomed (example: additional asserts)

Semi automatic switches (but can be specified manually):
    > Def_Windows; Def_Linux - target platforms
    > Def_Msvc; Dev_Llvm     - target compiler
*/



// ================================================================
// @Section_source_context_cracking.h
// ================================================================

//#pragma warning(push, 0)
// =========================== Types ==========================
#include <inttypes.h>
#include <stdint.h>
#include <limits.h>
#include <float.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>

// =========================== Basic ==========================
#if !defined(Def_Internal)
#  define Def_Internal 0
#endif
#if !defined(Def_Slow)
#  define Def_Slow 0
#endif
#if !defined(Def_More_Breakpoints)
#  define Def_More_Breakpoints 0
#endif
// ======== Detect platform =======
#if !defined(Def_Windows)
#  define Def_Windows 0
#endif
#if !defined(Def_Linux)
#  define Def_Linux 0
#endif
// ====== Platform not found ======
#if !Def_Windows && !Def_Linux
// TODO(f0): Check standard switches to automatically and deduce them and make compiling easier if possible
//           And support different compilers (cl, clang, gcc)
#  error "Define Def_Windows or Def_Linux"
#endif



// ======== Detect compiler =======
#if !defined(Def_Compiler_Msvc)
#  define Def_Compiler_Msvc 0
#endif
#if !defined(Def_Compiler_Llvm)
#  define Def_Compiler_Llvm 0
#endif
// ====== Compiler not found ======
#if !Def_Compiler_Msvc && !Def_Compiler_Llvm
#  if _MSC_VER
#    undef Def_Compiler_Msvc
#    define Def_Compiler_Msvc 1
#  else
// TODO(f0): More compilers
#    undef Def_Compiler_Llvm
#    define Def_Compiler_Llvm 1
#  endif
#endif



//=============================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//=============================
#if Def_Compiler_Msvc
#  include <intrin.h>
#elif Def_Compiler_Llvm
#  include <x86intrin.h>
#else
#  error "not impl; SSE/NEON optimizations?"
#endif



// ========================== Memory ==========================
#if Def_Windows
#  define WIN32_LEAN_AND_MEAN
#  include <Windows.h>
#  include <timeapi.h>
#  include <shellapi.h>
//#    include <debugapi.h>

// Microsoft's wall of shame:
#  undef far
#  undef near
#  undef interface
#  undef RELATIVE
#  undef ABSOLUTE
#endif



// =========================== Alloc ==========================
#include <stdarg.h>




// ========================= Platform =========================
#if Def_Windows
#  pragma comment(lib, "winmm.lib")
#  pragma comment(lib, "shell32.lib")
#endif

//#pragma warning(pop)




// ================================================================
// @Section_source_types.h
// ================================================================
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef s8 b8;
typedef s16 b16;
typedef s64 b64;
typedef s32 b32;

typedef u8  bit8;
typedef u16 bit16;
typedef u32 bit32;
typedef u64 bit64;

typedef float f32;
typedef double f64;

typedef __m128 Max_Aligment_Type; // TODO(f0): is there a type with bigger aligment? maybe create custom struct
//=============
#define S8_Min (-0x80)
#define S8_Max ( 0x7f)
#define S16_Min (-0x8000)
#define S16_Max ( 0x7fff)
#define S32_Min (-0x80000000LL)
#define S32_Max ( 0x7fffffffLL)
#define S64_Min (-0x8000000000000000LL)
#define S64_Max ( 0x7fffffffffffffffLL)
//
#define U8_Max (0xffU)
#define U16_Max (0xffffU)
#define U32_Max (0xffffffffULL)
#define U64_Max (0xffffffffffffffffULL)
//
#define F32_Max (3.402823466e+38f)
#define F64_Max (1.7976931348623158e+308)
//
#define Pi32  (3.14159265359f)
#define Tau32 (6.2831853071795864769f)
//=============
#define static_function static
#define static_local static
#define static_global static
#define static_const static const
#define force_inline __forceinline
//=============
#define array_count(a) ((sizeof(a))/(sizeof(*a)))
#define pick_smaller(a, b) (((a) > (b)) ? (b) : (a))
#define pick_bigger(a, b) (((a) > (b)) ? (a) : (b))
//
#define offset_of(Type, Member) ((s64)&(((Type *)0)->Member))
//
#define kilobytes(b) (1024*(b))
#define megabytes(b) (1024*kilobytes(b))
#define gigabytes(b) ((s64)1024*megabytes(b))
#define terabytes(b) ((s64)1024*gigabytes(b))
#define petabytes(b) ((s64)1024*terabytes(b))
#define exabytes(b) ((s64)1024*petabytes(b))
//
#define kilo(n) (1000*(n))
#define mega(n) (1000*kilo(n))
#define giga(n) (1000ULL*mega(n))
#define tera(n) (1000ULL*giga(n))
#define peta(n) (1000ULL*tera(n))
#define exa(n) (1000ULL*peta(n))
//
#define glue_(a, b) a ## b
#define glue(a, b) glue_(a, b)
#define stringify(a) #a
#define stringify2(a) stringify(a)
#define stringify_macro(a) stringify(a)
//
#define for_range(Type, I, Range) for (Type I = 0; I < (Range); ++I)
#define for_u64(I, Range) for_range(u64, I, Range)
#define for_u32(I, Range) for_range(u32, I, Range)
#define for_s64(I, Range) for_range(s64, I, Range)
#define for_s32(I, Range) for_range(s32, I, Range)
#define for_range_rev(Type, I, Range) for (Type I = (Range)-1; I >= 0; --I)
#define for_s64_rev(I, Range) for_range_rev(s64, I, Range)
#define for_s32_rev(I, Range) for_range_rev(s32, I, Range)
//
#define for_array(I, Array) for_s64(I, (s64)array_count(Array))
#define for_array_rev(I, Array) for_s64_rev(I, (s64)array_count(Array))
#define for_linked_list(Node, List) for (auto Node = (List).first; Node; Node = Node->next)
#define for_linked_list_ptr(Node, List) for (auto Node = (List)->first; Node; Node = Node->next)
//
#define u32_from_pointer(Pointer) ((u32)(u64)(Pointer))
#define pointer_from_u32(Type, Value) ((Type *)((u64)Value))
//
// NOTE(f0): Align bits needs to be power of 2
#define align_bin_to(Value, AlignBits) ((Value + (AlignBits-1)) & ~(AlignBits-1))
#define align4(Value) align_bin_to(Value, 4)
#define align8(Value) align_bin_to(Value, 8)
#define align16(Value) align_bin_to(Value, 16)
//=============
#define square_m128(value) _mm_mul_ps((value), (value))
#define f32_from_m128(wide, index) ((f32 *)&(wide))[index]
#define u32_from_m128i(wide, index) ((u32 *)&(wide))[index]
#define s32_from_m128i(wide, index) ((s32 *)&(wide))[index]




//================= Doubly linked list macros =================
#define for_dll_NP(Item, SentinelPtr, NextName) \
for(auto Item = (SentinelPtr)->NextName; Item != (SentinelPtr); Item = Item->NextName)

#define for_dll(Item, SentinelPtr) for_dll_NP(Item, SentinelPtr, next)
#define for_dll_Panel(Item, SentinelPtr) for_dll_NP(Item, SentinelPtr, panel_next)

//-
#define dll_initialize_sentinel_NP(SentinelPtr, NextName, PrevName) (SentinelPtr)->NextName = (SentinelPtr)->PrevName = (SentinelPtr)
#define dll_initialize_sentinel(SentinelPtr) dll_initialize_sentinel_NP(SentinelPtr, next, prev)


//-
#define dll_insert_after_NP(ParentPtr, NewChildPtr, NextName, PrevName) do{ \
(NewChildPtr)->NextName = (ParentPtr)->NextName;                            \
(NewChildPtr)->PrevName = (ParentPtr);                                      \
(NewChildPtr)->NextName->PrevName = (NewChildPtr);                          \
(NewChildPtr)->PrevName->NextName = (NewChildPtr);                          \
}while(0)

#define dll_insert_after(ParentPtr, NewChildPtr) dll_insert_after_NP(ParentPtr, NewChildPtr, next, prev)

//-
#define dll_insert_before_NP(ParentPtr, NewChildPtr, NextName, PrevName) do{ \
(NewChildPtr)->NextName = (ParentPtr);                                       \
(NewChildPtr)->PrevName = (ParentPtr)->PrevName;                             \
(NewChildPtr)->NextName->PrevName = (NewChildPtr);                           \
(NewChildPtr)->prev->NextName = (NewChildPtr);                               \
}while(0)

#define dll_insert_before(ParentPtr, NewChildPtr) dll_insert_before_NP(ParentPtr, NewChildPtr, next, prev)

//-
#define dll_remove_NP(ToRemovePtr, NextName, PrevName) do{   \
(ToRemovePtr)->NextName->PrevName = (ToRemovePtr)->PrevName; \
(ToRemovePtr)->PrevName->NextName = (ToRemovePtr)->NextName; \
}while(0)

#define dll_remove(ToRemovePtr) dll_remove_NP(ToRemovePtr, next, prev)




// ================================================================
// @Section_source_constants.h
// ================================================================
#define Bitmask_1 0x00000001
#define Bitmask_2 0x00000003
#define Bitmask_3 0x00000007
#define Bitmask_4 0x0000000f
#define Bitmask_5 0x0000001f
#define Bitmask_6 0x0000003f
#define Bitmask_7 0x0000007f
#define Bitmask_8 0x000000ff
#define Bitmask_9 0x000001ff
#define Bitmask_10 0x000003ff
#define Bitmask_11 0x000007ff
#define Bitmask_12 0x00000fff
#define Bitmask_13 0x00001fff
#define Bitmask_14 0x00003fff
#define Bitmask_15 0x00007fff
#define Bitmask_16 0x0000ffff
#define Bitmask_17 0x0001ffff
#define Bitmask_18 0x0003ffff
#define Bitmask_19 0x0007ffff
#define Bitmask_20 0x000fffff
#define Bitmask_21 0x001fffff
#define Bitmask_22 0x003fffff
#define Bitmask_23 0x007fffff
#define Bitmask_24 0x00ffffff
#define Bitmask_25 0x01ffffff
#define Bitmask_26 0x03ffffff
#define Bitmask_27 0x07ffffff
#define Bitmask_28 0x0fffffff
#define Bitmask_29 0x1fffffff
#define Bitmask_30 0x3fffffff
#define Bitmask_31 0x7fffffff

#define Bitmask_32 0x00000000ffffffff
#define Bitmask_33 0x00000001ffffffff
#define Bitmask_34 0x00000003ffffffff
#define Bitmask_35 0x00000007ffffffff
#define Bitmask_36 0x0000000fffffffff
#define Bitmask_37 0x0000001fffffffff
#define Bitmask_38 0x0000003fffffffff
#define Bitmask_39 0x0000007fffffffff
#define Bitmask_40 0x000000ffffffffff
#define Bitmask_41 0x000001ffffffffff
#define Bitmask_42 0x000003ffffffffff
#define Bitmask_43 0x000007ffffffffff
#define Bitmask_44 0x00000fffffffffff
#define Bitmask_45 0x00001fffffffffff
#define Bitmask_46 0x00003fffffffffff
#define Bitmask_47 0x00007fffffffffff
#define Bitmask_48 0x0000ffffffffffff
#define Bitmask_49 0x0001ffffffffffff
#define Bitmask_50 0x0003ffffffffffff
#define Bitmask_51 0x0007ffffffffffff
#define Bitmask_52 0x000fffffffffffff
#define Bitmask_53 0x001fffffffffffff
#define Bitmask_54 0x003fffffffffffff
#define Bitmask_55 0x007fffffffffffff
#define Bitmask_56 0x00ffffffffffffff
#define Bitmask_57 0x01ffffffffffffff
#define Bitmask_58 0x03ffffffffffffff
#define Bitmask_59 0x07ffffffffffffff
#define Bitmask_60 0x0fffffffffffffff
#define Bitmask_61 0x1fffffffffffffff
#define Bitmask_62 0x3fffffffffffffff
#define Bitmask_63 0x7fffffffffffffff

//-
#define Bit_1  0x00000001
#define Bit_2  0x00000002
#define Bit_3  0x00000004
#define Bit_4  0x00000008
#define Bit_5  0x00000010
#define Bit_6  0x00000020
#define Bit_7  0x00000040
#define Bit_8  0x00000080
#define Bit_9  0x00000100
#define Bit_10 0x00000200
#define Bit_11 0x00000400
#define Bit_12 0x00000800
#define Bit_13 0x00001000
#define Bit_14 0x00002000
#define Bit_15 0x00004000
#define Bit_16 0x00008000
#define Bit_17 0x00010000
#define Bit_18 0x00020000
#define Bit_19 0x00040000
#define Bit_20 0x00080000
#define Bit_21 0x00100000
#define Bit_22 0x00200000
#define Bit_23 0x00400000
#define Bit_24 0x00800000
#define Bit_25 0x01000000
#define Bit_26 0x02000000
#define Bit_27 0x04000000
#define Bit_28 0x08000000
#define Bit_29 0x10000000
#define Bit_30 0x20000000
#define Bit_31 0x40000000
#define Bit_32 0x80000000

#define Bit_33 0x0000000100000000
#define Bit_34 0x0000000200000000
#define Bit_35 0x0000000400000000
#define Bit_36 0x0000000800000000
#define Bit_37 0x0000001000000000
#define Bit_38 0x0000002000000000
#define Bit_39 0x0000004000000000
#define Bit_40 0x0000008000000000
#define Bit_41 0x0000010000000000
#define Bit_42 0x0000020000000000
#define Bit_43 0x0000040000000000
#define Bit_44 0x0000080000000000
#define Bit_45 0x0000100000000000
#define Bit_46 0x0000200000000000
#define Bit_47 0x0000400000000000
#define Bit_48 0x0000800000000000
#define Bit_49 0x0001000000000000
#define Bit_50 0x0002000000000000
#define Bit_51 0x0004000000000000
#define Bit_52 0x0008000000000000
#define Bit_53 0x0010000000000000
#define Bit_54 0x0020000000000000
#define Bit_55 0x0040000000000000
#define Bit_56 0x0080000000000000
#define Bit_57 0x0100000000000000
#define Bit_58 0x0200000000000000
#define Bit_59 0x0400000000000000
#define Bit_60 0x0800000000000000
#define Bit_61 0x1000000000000000
#define Bit_62 0x2000000000000000
#define Bit_63 0x4000000000000000
#define Bit_64 0x8000000000000000





// ================================================================
// @Section_source_basic.h
// ================================================================



//~ Compile time helper constants
#if Def_Windows
#define Native_Slash_Char '\\'
#define Native_Slash_Str "\\"
#elif Def_Linux
#define Native_Slash_Char '/'
#define Native_Slash_Str "/"
#endif


//~
#define This_Function __func__
#define This_Function_Sig __FUNCSIG__
#define This_Line_S32 __LINE__
#define This_File     __FILE__
#define Counter_Macro __COUNTER__

#define File_Line          This_File "(" stringify2(This_Line_S32) ")"
#define File_Line_Function File_Line ": " This_Function



//~
#define force_halt() do{ fflush(stdout); *((s32 volatile*)0) = 1; }while(0)

#define assert_always(Expression) do{ if(!(Expression)) {\
printf("\n" File_Line ": RUNTIME error: assert(%s) in function %s\n",\
stringify(Expression), This_Function);\
fflush(stdout);\
debug_break(); force_halt(); exit(1);\
}}while(0)



//~
#if Def_Slow
#  if Def_Windows

#    ifdef WIN32_LEAN_AND_MEAN
#      define debug_break() do{if(IsDebuggerPresent()) {fflush(stdout); __debugbreak();}}while(0)
#    else
#      define debug_break() do{fflush(stdout); __debugbreak();}while(0)
#    endif

#  elif Def_Linux
#    define debug_break() do{fflush(stdout); __builtin_debugtrap();}while(0)
#  endif



#  define assert(Expression) assert_always(Expression)
#  define break_at(Expression) do{if((Expression)){debug_break();}}while(0)

#else
#  define debug_break()
#  define assert(Expression)
#  define break_at(Expression)
#endif



//-
static_function b32 static_expression_wrapper_(b32 a) { return a; }
#define runtime_assert(ExpressionMakeNotStatic) assert(static_expression_wrapper_(ExpressionMakeNotStatic))
#define assert_bounds(Index, Array) assert((Index) >= 0 && (Index) < array_count(Array))
#define exit_error() do{ fflush(stdout); debug_break(); exit(1);}while(0)

#define Invalid_Default_Case default:{assert(0);}break




//~ Hacky C++11 defer
template <typename F>
struct Private_Defer {
	F f;
	Private_Defer(F f) : f(f) {}
	~Private_Defer() { f(); }
};

template <typename F>
Private_Defer<F> private_defer_static_function(F f) {
	return Private_Defer<F>(f);
}

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x)    DEFER_2(x, __COUNTER__)
#define defer(code)   auto DEFER_3(_defer_) = private_defer_static_function([&](){code;})

//~ Hacky execute in ctor (for executing things in global scope right after dll is loaded)
template <typename F>
struct Private_Execute_In_Ctor {
	Private_Execute_In_Ctor(F f) { f(); }
};

template <typename F>
Private_Execute_In_Ctor<F> private_execute_in_ctor_static_function(F f) {
	return Private_Execute_In_Ctor<F>(f);
}

#define EXECUTE_IN_CTOR_1(x, y) x##y
#define EXECUTE_IN_CTOR_2(x, y) EXECUTE_IN_CTOR_1(x, y)
#define EXECUTE_IN_CTOR_3(x)    EXECUTE_IN_CTOR_2(x, __COUNTER__)
#define execute_in_ctor(code)   auto EXECUTE_IN_CTOR_3(_execute_in_ctor_) = private_execute_in_ctor_static_function([&](){code;})




//~ Checked truncate
static_function u16 safe_truncate_to_u16(s32 value)
{
    assert(value <= (s32)U16_Max);
    assert(value >= 0);
    u16 result = (u16)value;
    return result;
}
static_function u16 safe_truncate_to_u16(u32 value)
{
    assert(value <= U16_Max);
    u16 result = (u16)value;
    return result;
}
static_function u16 safe_truncate_to_u16(u64 value)
{
    assert(value <= U16_Max);
    u16 result = (u16)value;
    return result;
}
static_function u16 safe_truncate_to_u16(s64 value)
{
    assert(value <= U16_Max);
    assert(value >= 0);
    u16 result = (u16)value;
    return result;
}

static_function s32 safe_truncate_to_s32(s64 value)
{
	assert(value <= (s64)S32_Max);
    assert(value >= (s64)S32_Min);
    u32 result = (s32)value;
	return result;
}
static_function s32 safe_truncate_to_s32(u64 value)
{
	assert(value <= (u64)S32_Max);
    u32 result = (s32)value;
	return result;
}

static_function u32 safe_truncate_to_u32(s32 value)
{
    assert(value >= 0);
    u32 result = (u32)value;
	return result;
}
static_function u32 safe_truncate_to_u32(s64 value)
{
	assert(value <= (s64)U32_Max);
    assert(value >= 0);
    u32 result = (u32)value;
	return result;
}
static_function u32 safe_truncate_to_u32(u64 value)
{
	assert(value <= U32_Max);
    u32 result = (u32)value;
	return result;
}

static_function u64 safe_truncate_to_u64(s64 value)
{
    assert(value >= 0);
    u64 result = (u64)value;
    return result;
}








//~ Handy memory copy
#define copy_bytes(DestinationPtr, SourcePtr, ByteCount) memcpy(DestinationPtr, SourcePtr, ByteCount)

#define copy_array(DestinationPtr, SourcePtr, Type, Count) copy_bytes(DestinationPtr, SourcePtr, sizeof(Type)*(Count))


//-
#define clear_bytes(DestinationPtr, ByteCount) memset(DestinationPtr, 0, ByteCount)

#define clear_array(DestinationPtr, Type, Count) clear_bytes(DestinationPtr, sizeof(Type)*(Count))
#define clear_struct_value(DestinationValue) clear_bytes(&DestinationValue, sizeof(DestinationValue))
#define clear_struct_pointer(DestinationPtr) clear_bytes(DestinationPtr, sizeof(*DestinationPtr))


//-
#define are_bytes_equal(APtr, BPtr, ByteCount) (memcmp(APtr, BPtr, ByteCount) == 0)
#define are_structs_equal(APtr, BPtr) (sizeof(*APtr) == sizeof(*BPtr) && are_bytes_equal(APtr, BPtr, sizeof(*APtr)))


// ================================================================
// @Section_source_math_types.h
// ================================================================

//~
#define Def_SSE4_Rounding_Floor    (_MM_FROUND_TO_NEG_INF|_MM_FROUND_NO_EXC)
#define Def_SSE4_Rounding_Ceil     (_MM_FROUND_TO_POS_INF|_MM_FROUND_NO_EXC)
#define Def_SSE4_Rounding_Nearest  (_MM_FROUND_TO_NEAREST_INT|_MM_FROUND_NO_EXC)
#define Def_SSE4_Rounding_Truncate (_MM_FROUND_TO_ZERO|_MM_FROUND_NO_EXC)


//~ Vectors (float)
union v2
{
    struct {
        f32 x, y;
    };
    f32 e[2];
};
union v3
{
    struct {
        f32 x, y, z;
    };
    struct {
        v2 xy;
    };
    struct {
        f32 _x_;
        v2 yz;
    };
    f32 e[3];
};
union v4
{
    struct {
        f32 x, y, z, w;
    };
    struct {
        v3 xyz;
    };
    struct {
        v2 xy, zw;
    };
    struct {
        f32 r, g, b, a;
    };
    struct {
        v3 rgb;
    };
    f32 e[4];
};

union f32_4x
{
    __m128 p;
    f32 e[4];
    u32 u[4];
    v4 vec;
};

union f32_8x
{
    __m256 p;
    f32 e[8];
    u32 u[8];
    v4 vec[2];
};


//~ Matrices (float)
union m4x4
{
    f32 e[4][4]; // NOTE(f0): e[ROW][COLUMN]
    f32 flat[16];
    f32_4x row[4];
};


//~ Rectangles (float)
struct Rect2
{
    v2 min, max;
};
struct Rect3
{
    v3 min, max;
};


//~ Vectors (int)
union v2s
{
    struct {
        s32 x, y;
    };
    s32 e[2];
};

//~ Rectangles (int)
struct Rect2s
{
    v2s min, max;
};



// ================================================================
// @Section_source_math_scalar.h
// ================================================================

//~

static_function u8
clamp(u8 min, u8 value, u8 max)
{
    u8 result = value;
    if (result < min) { result = min; }
    if (result > max) { result = max; }
    return result;
}
static_function u16
clamp(u16 min, u16 value, u16 max)
{
    u16 result = value;
    if (result < min) { result = min; }
    if (result > max) { result = max; }
    return result;
}
static_function u32
clamp(u32 min, u32 value, u32 max)
{
    u32 result = value;
    if (result < min) { result = min; }
    if (result > max) { result = max; }
    return result;
}
static_function u64
clamp(u64 min, u64 value, u64 max)
{
    u64 result = value;
    if (result < min) { result = min; }
    if (result > max) { result = max; }
    return result;
}
static_function s8
clamp(s8 min, s8 value, s8 max)
{
    s8 result = value;
    if (result < min) { result = min; }
    if (result > max) { result = max; }
    return result;
}
static_function s16
clamp(s16 min, s16 value, s16 max)
{
    s16 result = value;
    if (result < min) { result = min; }
    if (result > max) { result = max; }
    return result;
}
static_function s32
clamp(s32 min, s32 value, s32 max)
{
    s32 result = value;
    if (result < min) { result = min; }
    if (result > max) { result = max; }
    return result;
}
static_function s64
clamp(s64 min, s64 value, s64 max)
{
    s64 result = value;
    if (result < min) { result = min; }
    if (result > max) { result = max; }
    return result;
}
static_function f32
clamp(f32 min, f32 value, f32 max)
{
    f32 result = value;
    if (result < min) { result = min; }
    if (result > max) { result = max; }
    return result;
}
static_function f64
clamp(f64 min, f64 value, f64 max)
{
    f64 result = value;
    if (result < min) { result = min; }
    if (result > max) { result = max; }
    return result;
}


static_function void
clamp_ref_bot(u8 min, u8 *value)
{
    if (*value < min) { *value = min; }
}
static_function void
clamp_ref_bot(u16 min, u16 *value)
{
    if (*value < min) { *value = min; }
}
static_function void
clamp_ref_bot(u32 min, u32 *value)
{
    if (*value < min) { *value = min; }
}
static_function void
clamp_ref_bot(u64 min, u64 *value)
{
    if (*value < min) { *value = min; }
}
static_function void
clamp_ref_bot(s8 min, s8 *value)
{
    if (*value < min) { *value = min; }
}
static_function void
clamp_ref_bot(s16 min, s16 *value)
{
    if (*value < min) { *value = min; }
}
static_function void
clamp_ref_bot(s32 min, s32 *value)
{
    if (*value < min) { *value = min; }
}
static_function void
clamp_ref_bot(s64 min, s64 *value)
{
    if (*value < min) { *value = min; }
}
static_function void
clamp_ref_bot(f32 min, f32 *value)
{
    if (*value < min) { *value = min; }
}
static_function void
clamp_ref_bot(f64 min, f64 *value)
{
    if (*value < min) { *value = min; }
}


static_function void
clamp_ref_top(u8 *value, u8 max)
{
    if (*value > max) { *value = max; }
}
static_function void
clamp_ref_top(u16 *value, u16 max)
{
    if (*value > max) { *value = max; }
}
static_function void
clamp_ref_top(u32 *value, u32 max)
{
    if (*value > max) { *value = max; }
}
static_function void
clamp_ref_top(u64 *value, u64 max)
{
    if (*value > max) { *value = max; }
}
static_function void
clamp_ref_top(s8 *value, s8 max)
{
    if (*value > max) { *value = max; }
}
static_function void
clamp_ref_top(s16 *value, s16 max)
{
    if (*value > max) { *value = max; }
}
static_function void
clamp_ref_top(s32 *value, s32 max)
{
    if (*value > max) { *value = max; }
}
static_function void
clamp_ref_top(s64 *value, s64 max)
{
    if (*value > max) { *value = max; }
}
static_function void
clamp_ref_top(f32 *value, f32 max)
{
    if (*value > max) { *value = max; }
}
static_function void
clamp_ref_top(f64 *value, f64 max)
{
    if (*value > max) { *value = max; }
}


static_function void
clamp_ref(u8 min, u8 *value, u8 max)
{
    if (*value < min) { *value = min; }
    if (*value > max) { *value = max; }
}
static_function void
clamp_ref(u16 min, u16 *value, u16 max)
{
    if (*value < min) { *value = min; }
    if (*value > max) { *value = max; }
}
static_function void
clamp_ref(u32 min, u32 *value, u32 max)
{
    if (*value < min) { *value = min; }
    if (*value > max) { *value = max; }
}
static_function void
clamp_ref(u64 min, u64 *value, u64 max)
{
    if (*value < min) { *value = min; }
    if (*value > max) { *value = max; }
}
static_function void
clamp_ref(s8 min, s8 *value, s8 max)
{
    if (*value < min) { *value = min; }
    if (*value > max) { *value = max; }
}
static_function void
clamp_ref(s16 min, s16 *value, s16 max)
{
    if (*value < min) { *value = min; }
    if (*value > max) { *value = max; }
}
static_function void
clamp_ref(s32 min, s32 *value, s32 max)
{
    if (*value < min) { *value = min; }
    if (*value > max) { *value = max; }
}
static_function void
clamp_ref(s64 min, s64 *value, s64 max)
{
    if (*value < min) { *value = min; }
    if (*value > max) { *value = max; }
}
static_function void
clamp_ref(f32 min, f32 *value, f32 max)
{
    if (*value < min) { *value = min; }
    if (*value > max) { *value = max; }
}
static_function void
clamp_ref(f64 min, f64 *value, f64 max)
{
    if (*value < min) { *value = min; }
    if (*value > max) { *value = max; }
}



static_function u8
safe_ratio_n(u8 numerator, u8 divisor, u8 n)
{
    u8 result = n;
    if (divisor != (u8)0)
    {
        result = numerator / divisor;
    }
    return result;
}
static_function u16
safe_ratio_n(u16 numerator, u16 divisor, u16 n)
{
    u16 result = n;
    if (divisor != (u16)0)
    {
        result = numerator / divisor;
    }
    return result;
}
static_function u32
safe_ratio_n(u32 numerator, u32 divisor, u32 n)
{
    u32 result = n;
    if (divisor != (u32)0)
    {
        result = numerator / divisor;
    }
    return result;
}
static_function u64
safe_ratio_n(u64 numerator, u64 divisor, u64 n)
{
    u64 result = n;
    if (divisor != (u64)0)
    {
        result = numerator / divisor;
    }
    return result;
}
static_function s8
safe_ratio_n(s8 numerator, s8 divisor, s8 n)
{
    s8 result = n;
    if (divisor != (s8)0)
    {
        result = numerator / divisor;
    }
    return result;
}
static_function s16
safe_ratio_n(s16 numerator, s16 divisor, s16 n)
{
    s16 result = n;
    if (divisor != (s16)0)
    {
        result = numerator / divisor;
    }
    return result;
}
static_function s32
safe_ratio_n(s32 numerator, s32 divisor, s32 n)
{
    s32 result = n;
    if (divisor != (s32)0)
    {
        result = numerator / divisor;
    }
    return result;
}
static_function s64
safe_ratio_n(s64 numerator, s64 divisor, s64 n)
{
    s64 result = n;
    if (divisor != (s64)0)
    {
        result = numerator / divisor;
    }
    return result;
}
static_function f32
safe_ratio_n(f32 numerator, f32 divisor, f32 n)
{
    f32 result = n;
    if (divisor != (f32)0)
    {
        result = numerator / divisor;
    }
    return result;
}
static_function f64
safe_ratio_n(f64 numerator, f64 divisor, f64 n)
{
    f64 result = n;
    if (divisor != (f64)0)
    {
        result = numerator / divisor;
    }
    return result;
}


static_function u8
safe_ratio_0(u8 numerator, u8 divisor)
{
    u8 result = safe_ratio_n(numerator, divisor, (u8)0);
    return result;
}
static_function u16
safe_ratio_0(u16 numerator, u16 divisor)
{
    u16 result = safe_ratio_n(numerator, divisor, (u16)0);
    return result;
}
static_function u32
safe_ratio_0(u32 numerator, u32 divisor)
{
    u32 result = safe_ratio_n(numerator, divisor, (u32)0);
    return result;
}
static_function u64
safe_ratio_0(u64 numerator, u64 divisor)
{
    u64 result = safe_ratio_n(numerator, divisor, (u64)0);
    return result;
}
static_function s8
safe_ratio_0(s8 numerator, s8 divisor)
{
    s8 result = safe_ratio_n(numerator, divisor, (s8)0);
    return result;
}
static_function s16
safe_ratio_0(s16 numerator, s16 divisor)
{
    s16 result = safe_ratio_n(numerator, divisor, (s16)0);
    return result;
}
static_function s32
safe_ratio_0(s32 numerator, s32 divisor)
{
    s32 result = safe_ratio_n(numerator, divisor, (s32)0);
    return result;
}
static_function s64
safe_ratio_0(s64 numerator, s64 divisor)
{
    s64 result = safe_ratio_n(numerator, divisor, (s64)0);
    return result;
}
static_function f32
safe_ratio_0(f32 numerator, f32 divisor)
{
    f32 result = safe_ratio_n(numerator, divisor, (f32)0);
    return result;
}
static_function f64
safe_ratio_0(f64 numerator, f64 divisor)
{
    f64 result = safe_ratio_n(numerator, divisor, (f64)0);
    return result;
}


static_function u8
safe_ratio_1(u8 numerator, u8 divisor)
{
    u8 result = safe_ratio_n(numerator, divisor, (u8)1);
    return result;
}
static_function u16
safe_ratio_1(u16 numerator, u16 divisor)
{
    u16 result = safe_ratio_n(numerator, divisor, (u16)1);
    return result;
}
static_function u32
safe_ratio_1(u32 numerator, u32 divisor)
{
    u32 result = safe_ratio_n(numerator, divisor, (u32)1);
    return result;
}
static_function u64
safe_ratio_1(u64 numerator, u64 divisor)
{
    u64 result = safe_ratio_n(numerator, divisor, (u64)1);
    return result;
}
static_function s8
safe_ratio_1(s8 numerator, s8 divisor)
{
    s8 result = safe_ratio_n(numerator, divisor, (s8)1);
    return result;
}
static_function s16
safe_ratio_1(s16 numerator, s16 divisor)
{
    s16 result = safe_ratio_n(numerator, divisor, (s16)1);
    return result;
}
static_function s32
safe_ratio_1(s32 numerator, s32 divisor)
{
    s32 result = safe_ratio_n(numerator, divisor, (s32)1);
    return result;
}
static_function s64
safe_ratio_1(s64 numerator, s64 divisor)
{
    s64 result = safe_ratio_n(numerator, divisor, (s64)1);
    return result;
}
static_function f32
safe_ratio_1(f32 numerator, f32 divisor)
{
    f32 result = safe_ratio_n(numerator, divisor, (f32)1);
    return result;
}
static_function f64
safe_ratio_1(f64 numerator, f64 divisor)
{
    f64 result = safe_ratio_n(numerator, divisor, (f64)1);
    return result;
}









//~

static_function s8
sign_of(s8 value)
{
    s8 result = (value >= (s8)0) ? (s8)1 : (s8)-1;
    return result;
}
static_function s16
sign_of(s16 value)
{
    s16 result = (value >= (s16)0) ? (s16)1 : (s16)-1;
    return result;
}
static_function s32
sign_of(s32 value)
{
    s32 result = (value >= (s32)0) ? (s32)1 : (s32)-1;
    return result;
}
static_function s64
sign_of(s64 value)
{
    s64 result = (value >= (s64)0) ? (s64)1 : (s64)-1;
    return result;
}
static_function f32
sign_of(f32 value)
{
    f32 result = (value >= (f32)0) ? (f32)1 : (f32)-1;
    return result;
}
static_function f64
sign_of(f64 value)
{
    f64 result = (value >= (f64)0) ? (f64)1 : (f64)-1;
    return result;
}



static_function s8
absolute_value(s8 value)
{
    if (value < (s8)0)
    {
        value = -value;
    }
    return value;
}
static_function s16
absolute_value(s16 value)
{
    if (value < (s16)0)
    {
        value = -value;
    }
    return value;
}
static_function s32
absolute_value(s32 value)
{
    if (value < (s32)0)
    {
        value = -value;
    }
    return value;
}
static_function s64
absolute_value(s64 value)
{
    if (value < (s64)0)
    {
        value = -value;
    }
    return value;
}
static_function f32
absolute_value(f32 value)
{
    if (value < (f32)0)
    {
        value = -value;
    }
    return value;
}
static_function f64
absolute_value(f64 value)
{
    if (value < (f64)0)
    {
        value = -value;
    }
    return value;
}









//~
// ============================ f32 ===========================
static_function f32
square(f32 a)
{
    f32 result = a * a;
    return result;
}

static_function f32
lerp(f32 a, f32 t, f32 b)
{
    f32 result = (1.0f - t)*a + t*b;
    return result;
}

static_function f32
clamp01(f32 value)
{
    f32 result = clamp(0.0f, value, 1.0f);
    return result;
}

static_function f32
clamp01_map_to_range(f32 min, f32 t, f32 max)
{
    f32 result = 0.f;
    
    f32 range = max - min;
    if (range != 0.0f)
    {
        result = clamp01((t - min) / range);
    }
    
    return result;
}





// ================================================================
// @Section_source_string_common.h
// ================================================================
struct String
{
    u8 *str;
    u64 size;
};
struct String16
{
    u16 *str;
    u64 size;
};

static_function String
operator "" _f0(const char *str, size_t size)
{
    String result = {(u8 *)str, (u64)size};
    return result;
}

static_function String16
operator "" _f16(const wchar_t *str, size_t size)
{
    String16 result = {(u16 *)str, (u64)size};
    return result;
}


struct Find_Index
{
    u64 index; // Index defaults to 0 when not found
    b32 found;
};

struct String_Dll_Node
{
    String string;
    String_Dll_Node *next;
    String_Dll_Node *prev;
};


// Macros
#define string_expand(Str) ((s32)((Str).size)), ((char *)((Str).str))
#define string_expand_rev(Str) ((char *)((Str).str)), ((Str).size)
#define lit2str(Literal) get_string(Literal, array_count(Literal)-1)
#define l2s(Literal) lit2str(Literal)





static_function u64
zero_terminated_length(char *text)
{
    u64 length = 0;
    while (*text++) {
        ++length;
    }
    return length;
}
static_function u64
zero_terminated_length(const char *text)
{
    u64 length = 0;
    while (*text++) {
        ++length;
    }
    return length;
}
static_function u64
zero_terminated_length(u8 *text)
{
    u64 length = 0;
    while (*text++) {
        ++length;
    }
    return length;
}



// Constructors
static_function String
get_string(u8 *str, u64 size)
{
    String result = {str, size};
    return result;
}

static_function String
get_string(const char *str, u64 size)
{
    return get_string((u8 *)str, size);
}

static_function String
get_string(const char *cstr)
{
    String result = {
        (u8 *)cstr,
        zero_terminated_length(cstr)
    };
    return result;
}




static_function String16
get_string16(wchar_t *wstr)
{
    String16 result = {(u16 *)wstr, 0};
    while (*wstr)
    {
        wstr += 1;
        result.size += 1;
    }
    return result;
}

static_function String16
get_string16(wchar_t *wstr, u64 length)
{
    String16 result = {(u16 *)wstr, length};
    return result;
}





// ======================== @Basic_Char =======================
static_function b32 is_slash(u8 c) { return (c == '\\' || c == '/'); }
static_function b32 is_slash(char c) { return (c == '\\' || c == '/'); }
static_function b32 is_slash(u16 c) { return (c == '\\' || c == '/'); }
static_function b32 is_slash(u32 c) { return (c == '\\' || c == '/'); }

static_function b32 is_end_of_line(u8 c) { return ((c == '\n') || (c == '\r')); }
static_function b32 is_end_of_line(char c) { return ((c == '\n') || (c == '\r')); }
static_function b32 is_end_of_line(u16 c) { return ((c == '\n') || (c == '\r')); }
static_function b32 is_end_of_line(u32 c) { return ((c == '\n') || (c == '\r')); }

static_function b32 is_whitespace(u8 c) { return ((c == ' ') || (c == '\t') || (c == '\v') || (c == '\f')); }
static_function b32 is_whitespace(char c) { return ((c == ' ') || (c == '\t') || (c == '\v') || (c == '\f')); }
static_function b32 is_whitespace(u16 c) { return ((c == ' ') || (c == '\t') || (c == '\v') || (c == '\f')); }
static_function b32 is_whitespace(u32 c) { return ((c == ' ') || (c == '\t') || (c == '\v') || (c == '\f')); }

static_function b32 is_white(u8 c) { return (is_whitespace(c) || is_end_of_line(c)); }
static_function b32 is_white(char c) { return (is_whitespace(c) || is_end_of_line(c)); }
static_function b32 is_white(u16 c) { return (is_whitespace(c) || is_end_of_line(c)); }
static_function b32 is_white(u32 c) { return (is_whitespace(c) || is_end_of_line(c)); }

static_function b32 is_alpha(u8 c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
static_function b32 is_alpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
static_function b32 is_alpha(u16 c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
static_function b32 is_alpha(u32 c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }

static_function b32 is_number(u8 c) { return (c >= '0' && c <= '9'); }
static_function b32 is_number(char c) { return (c >= '0' && c <= '9'); }
static_function b32 is_number(u16 c) { return (c >= '0' && c <= '9'); }
static_function b32 is_number(u32 c) { return (c >= '0' && c <= '9'); }

static_function b32 is_lower(u8 c) { return (c >= 'a' && c <= 'z'); }
static_function b32 is_lower(char c) { return (c >= 'a' && c <= 'z'); }
static_function b32 is_lower(u16 c) { return (c >= 'a' && c <= 'z'); }
static_function b32 is_lower(u32 c) { return (c >= 'a' && c <= 'z'); }

static_function b32 is_upper(u8 c) { return (c >= 'A' && c <= 'Z'); }
static_function b32 is_upper(char c) { return (c >= 'A' && c <= 'Z'); }
static_function b32 is_upper(u16 c) { return (c >= 'A' && c <= 'Z'); }
static_function b32 is_upper(u32 c) { return (c >= 'A' && c <= 'Z'); }



static_function u8 to_lower(u8 c)
{
    if (c >= 'A' && c <= 'Z') {
        c += ('a' - 'A');
    };
    return c;
}
static_function char to_lower(char c)
{
    if (c >= 'A' && c <= 'Z') {
        c += ('a' - 'A');
    };
    return c;
}
static_function u16 to_lower(u16 c)
{
    if (c >= 'A' && c <= 'Z') {
        c += ('a' - 'A');
    };
    return c;
}
static_function u32 to_lower(u32 c)
{
    if (c >= 'A' && c <= 'Z') {
        c += ('a' - 'A');
    };
    return c;
}


static_function u8 to_upper(u8 c)
{
    if (c >= 'a' && c <= 'z') {
        c -= ('a' - 'A');
    };
    return c;
}
static_function char to_upper(char c)
{
    if (c >= 'a' && c <= 'z') {
        c -= ('a' - 'A');
    };
    return c;
}
static_function u16 to_upper(u16 c)
{
    if (c >= 'a' && c <= 'z') {
        c -= ('a' - 'A');
    };
    return c;
}
static_function u32 to_upper(u32 c)
{
    if (c >= 'a' && c <= 'z') {
        c -= ('a' - 'A');
    };
    return c;
}





// ================================================================
// @Section_source_string_constants.h
// ================================================================

static_const String character_table_white = " \t\n\r\v\f"_f0;




// ================================================================
// @Section_source_cstr.h
// ================================================================
static_function b32
equals(const char *value_a, const char *value_b, b32 case_ins = false)
{
    b32 result = false;
    for(;;)
    {
        char a = (*value_a++);
        char b = (*value_b++);
        
        if (case_ins)
        {
            a = to_lower(a);
            b = to_lower(b);
        }
        
        if (a != b)
        {
            break;
        }
        else if (a == 0)
        {
            result = true;
            break;
        }
    }
    return result;
}



static_function b32
cstr_starts_with(const char *haystack, const char *needle, b32 case_ins = false)
{
    b32 result = false;
    for(;;)
    {
        char h = (*haystack++);
        char n = (*needle++);
        
        if (case_ins)
        {
            h = to_lower(h);
            n = to_lower(n);
        }
        
        if (n == 0)
        {
            result = true;
            break;
        }
        else if (h != n)
        {
            break;
        }
    }
    return result;
}

static_function b32
cstr_ends_with(const char *haystack, const char *needle, b32 case_ins = false)
{
    u64 haystack_len = zero_terminated_length(haystack);
    u64 needle_len = zero_terminated_length(needle);
    
    for (u64 index_rev = 1;
         index_rev <= needle_len;
         ++index_rev)
    {
        char h = haystack[haystack_len - index_rev];
        char n = needle[needle_len - index_rev];
        
        if (case_ins)
        {
            h = to_lower(h);
            n = to_lower(n);
        }
        
        if (h != n)
        {
            return false;
        }
    }
    
    return true;
}



static_function Find_Index
cstr_index_of(const char *value, const char c)
{
    Find_Index result = {};
    for (u64 index = 0;
         ;
         ++index)
    {
        char v = value[index];
        
        if (v == c)
        {
            result.index = index;
            result.found = true;
            break;
        }
        else if (!v)
        {
            break;
        }
    }
    return result;
}


static_function Find_Index
cstr_index_of_difference(const char *value_a, const char *value_b, b32 case_ins = false)
{
    Find_Index result = {};
    
    for (u64 diff_index = 0;
         ; 
         ++diff_index)
    {
        char a = *value_a++;
        char b = *value_b++;
        
        if (case_ins) {
            a = to_lower(a);
            b = to_lower(b);
        }
        
        if (a != b)
        {
            result.index = diff_index;
            result.found = true;
            break;
        }
        
        if (a == 0) {
            break;
        }
    }
    
    return result;
}




static_function Find_Index
cstr_index_of_from_table(const char *value, String character_table)
{
    Find_Index result = {};
    
    for (u64 value_index = 0;
         ;
         ++value_index)
    {
        char v = value[value_index];
        
        for_u64(search_index, character_table.size)
        {
            char s = (char)character_table.str[search_index];
            if (v == s)
            {
                result.index = value_index;
                result.found = true;
                goto exit_break_label;
            }
        }
        
        if (!v) {
            break;
        }
    }
    
    exit_break_label:
    return result;
}


// ================================================================
// @Section_source_string.h
// ================================================================


static_function String
str_prefix(String text, u64 size)
{
    size = pick_smaller(size, text.size);
    text.size = size;
    return text;
}
static_function String16
str_prefix(String16 text, u64 size)
{
    size = pick_smaller(size, text.size);
    text.size = size;
    return text;
}


////////////////////////////////
static_function String
str_postfix(String text, u64 size)
{
    size = pick_smaller(text.size, size);
    u64 distance = text.size - size;
    
    text.str = text.str + distance;
    text.size = size;
    
    return text;
}
static_function String16
str_postfix(String16 text, u64 size)
{
    size = pick_smaller(text.size, size);
    u64 distance = text.size - size;
    
    text.str = text.str + distance;
    text.size = size;
    
    return text;
}
;

////////////////////////////////
static_function String
str_skip(String text, u64 distance)
{
    distance = pick_smaller(distance, text.size);
    String result = {};
    result.str = text.str + distance;
    result.size = text.size - distance;
    return result;
}
static_function String16
str_skip(String16 text, u64 distance)
{
    distance = pick_smaller(distance, text.size);
    String16 result = {};
    result.str = text.str + distance;
    result.size = text.size - distance;
    return result;
}


////////////////////////////////
static_function String
str_chop(String text, u64 distance_from_end)
{
    if (distance_from_end > text.size)
    {
        text.size = 0;
    }
    else
    {
        text.size = text.size - distance_from_end;
    }
    return text;
}
static_function String16
str_chop(String16 text, u64 distance_from_end)
{
    if (distance_from_end > text.size)
    {
        text.size = 0;
    }
    else
    {
        text.size = text.size - distance_from_end;
    }
    return text;
}


////////////////////////////////
static_function String
str_substr(String text, u64 distance, u64 length)
{
    String result = str_skip(text, distance);
    result.size = pick_smaller(result.size, length);
    return result;
}
static_function String16
str_substr(String16 text, u64 distance, u64 length)
{
    String16 result = str_skip(text, distance);
    result.size = pick_smaller(result.size, length);
    return result;
}









static_function b32
equals(String str_a, String str_b, b32 case_ins = false)
{
    b32 result = false;
    
    if (str_a.size == str_b.size)
    {
        result = true;
        for_u64(index, str_a.size)
        {
            auto a = str_a.str[index];
            auto b = str_b.str[index];
            
            if (case_ins)
            {
                a = to_lower(a);
                b = to_lower(b);
            }
            
            if (a != b)
            {
                result = false;
                break;
            }
        }
    }
    
    return result;
}
static_function b32
equals(String16 str_a, String16 str_b, b32 case_ins = false)
{
    b32 result = false;
    
    if (str_a.size == str_b.size)
    {
        result = true;
        for_u64(index, str_a.size)
        {
            auto a = str_a.str[index];
            auto b = str_b.str[index];
            
            if (case_ins)
            {
                a = to_lower(a);
                b = to_lower(b);
            }
            
            if (a != b)
            {
                result = false;
                break;
            }
        }
    }
    
    return result;
}





static_function b32
str_starts_with(String haystack, String needle, b32 case_ins = false)
{
    b32 result = false;
    if (haystack.size >= needle.size)
    {
        result = true;
        for_u64(index, needle.size)
        {
            auto h = haystack.str[index];
            auto n = needle.str[index];
            
            if (case_ins)
            {
                h = to_lower(h);
                n = to_lower(n);
            }
            
            if (h != n)
            {
                result = false;
                break;
            }
        }
    }
    
    return result;
}
static_function b32
str_starts_with(String16 haystack, String16 needle, b32 case_ins = false)
{
    b32 result = false;
    if (haystack.size >= needle.size)
    {
        result = true;
        for_u64(index, needle.size)
        {
            auto h = haystack.str[index];
            auto n = needle.str[index];
            
            if (case_ins)
            {
                h = to_lower(h);
                n = to_lower(n);
            }
            
            if (h != n)
            {
                result = false;
                break;
            }
        }
    }
    
    return result;
}



static_function b32
str_ends_with(String haystack, String needle, b32 case_ins = false)
{
    b32 result = false;
    if (haystack.size >= needle.size)
    {
        result = true;
        for (u64 index_rev = 1; index_rev <= needle.size; ++index_rev)
        {
            auto h = haystack.str[haystack.size - index_rev];
            auto n = needle.str[needle.size - index_rev];
            
            if (case_ins)
            {
                h = to_lower(h);
                n = to_lower(n);
            }
            
            if (h != n)
            {
                result = false;
                break;
            }
        }
    }
    
    return result;
}
static_function b32
str_ends_with(String16 haystack, String16 needle, b32 case_ins = false)
{
    b32 result = false;
    if (haystack.size >= needle.size)
    {
        result = true;
        for (u64 index_rev = 1; index_rev <= needle.size; ++index_rev)
        {
            auto h = haystack.str[haystack.size - index_rev];
            auto n = needle.str[needle.size - index_rev];
            
            if (case_ins)
            {
                h = to_lower(h);
                n = to_lower(n);
            }
            
            if (h != n)
            {
                result = false;
                break;
            }
        }
    }
    
    return result;
}











//~
static_function Find_Index
str_index_of(String haystack, char needle, b32 case_ins = false)
{
    Find_Index result = {};
    
    if (case_ins)
    {
        needle = to_lower(needle);
    }
    
    for_u64(index, haystack.size)
    {
        auto h = haystack.str[index];
        if (case_ins)
        {
            h = to_lower(h);
        }
        
        if (h == needle)
        {
            result.index = index;
            result.found = true;
            break;
        }
    }
    return result;
}
static_function Find_Index
str_index_of(String16 haystack, char needle, b32 case_ins = false)
{
    Find_Index result = {};
    
    if (case_ins)
    {
        needle = to_lower(needle);
    }
    
    for_u64(index, haystack.size)
    {
        auto h = haystack.str[index];
        if (case_ins)
        {
            h = to_lower(h);
        }
        
        if (h == needle)
        {
            result.index = index;
            result.found = true;
            break;
        }
    }
    return result;
}



static_function Find_Index
str_index_of(String haystack, String needle, b32 case_ins = false)
{
    // naive implementation
    // TODO(f0): faster search impl
    Find_Index result = {};
    
    if (needle.size && haystack.size >= needle.size)
    {
        for (u64 start_haystack_index = 0;
             start_haystack_index <= haystack.size - needle.size;
             start_haystack_index += 1)
        {
            u64 haystack_index = start_haystack_index;
            
            for (u64 needle_index = 0;
                 ;
                 ++needle_index, ++haystack_index)
            {
                if (needle_index == needle.size)
                {
                    result.index = start_haystack_index;
                    result.found = true;
                    return result;
                }
                
                auto h = haystack.str[haystack_index];
                auto n = needle.str[needle_index];
                
                if (case_ins)
                {
                    h = to_lower(h);
                    n = to_lower(n);
                }
                
                if (h != n)
                {
                    break;
                }
            }
        }
    }
    
    return result;
}
static_function Find_Index
str_index_of(String16 haystack, String16 needle, b32 case_ins = false)
{
    // naive implementation
    // TODO(f0): faster search impl
    Find_Index result = {};
    
    if (needle.size && haystack.size >= needle.size)
    {
        for (u64 start_haystack_index = 0;
             start_haystack_index <= haystack.size - needle.size;
             start_haystack_index += 1)
        {
            u64 haystack_index = start_haystack_index;
            
            for (u64 needle_index = 0;
                 ;
                 ++needle_index, ++haystack_index)
            {
                if (needle_index == needle.size)
                {
                    result.index = start_haystack_index;
                    result.found = true;
                    return result;
                }
                
                auto h = haystack.str[haystack_index];
                auto n = needle.str[needle_index];
                
                if (case_ins)
                {
                    h = to_lower(h);
                    n = to_lower(n);
                }
                
                if (h != n)
                {
                    break;
                }
            }
        }
    }
    
    return result;
}


//~
static_function Find_Index
str_index_of_reverse(String haystack, char needle, b32 case_ins = false)
{
    Find_Index result = {};
    
    if (case_ins) {
        needle = to_lower(needle);
    }
    
    for_s64_rev(index, haystack.size)
    {
        auto h = haystack.str[index];
        if (case_ins)
        {
            h = to_lower(h);
        }
        
        if (h == needle)
        {
            result.index = index;
            result.found = true;
            break;
        }
    }
    return result;
}
static_function Find_Index
str_index_of_reverse(String16 haystack, char needle, b32 case_ins = false)
{
    Find_Index result = {};
    
    if (case_ins) {
        needle = to_lower(needle);
    }
    
    for_s64_rev(index, haystack.size)
    {
        auto h = haystack.str[index];
        if (case_ins)
        {
            h = to_lower(h);
        }
        
        if (h == needle)
        {
            result.index = index;
            result.found = true;
            break;
        }
    }
    return result;
}


static_function Find_Index
str_index_of_reverse(String haystack, String needle, b32 case_ins = false)
{
    // naive implementation
    // TODO(f0): faster search impl
    Find_Index result = {};
    
    if (needle.size > 0 && haystack.size >= needle.size)
    {
        for (s64 end_haystack_index = haystack.size - 1;
             (end_haystack_index + 1) >= (s64)needle.size;
             end_haystack_index -= 1)
        {
            s64 haystack_index = end_haystack_index;
            for (s64 needle_index = needle.size - 1;
                 ;
                 --needle_index, --haystack_index)
            {
                if (needle_index < 0)
                {
                    result.index = haystack_index + 1;
                    result.found = true;
                    return result;
                }
                
                auto h = haystack.str[haystack_index];
                auto n = needle.str[needle_index];
                
                if (case_ins)
                {
                    h = to_lower(h);
                    n = to_lower(n);
                }
                
                if (h != n)
                {
                    break;
                }
            }
        }
    }
    
    return result;
}
static_function Find_Index
str_index_of_reverse(String16 haystack, String16 needle, b32 case_ins = false)
{
    // naive implementation
    // TODO(f0): faster search impl
    Find_Index result = {};
    
    if (needle.size > 0 && haystack.size >= needle.size)
    {
        for (s64 end_haystack_index = haystack.size - 1;
             (end_haystack_index + 1) >= (s64)needle.size;
             end_haystack_index -= 1)
        {
            s64 haystack_index = end_haystack_index;
            for (s64 needle_index = needle.size - 1;
                 ;
                 --needle_index, --haystack_index)
            {
                if (needle_index < 0)
                {
                    result.index = haystack_index + 1;
                    result.found = true;
                    return result;
                }
                
                auto h = haystack.str[haystack_index];
                auto n = needle.str[needle_index];
                
                if (case_ins)
                {
                    h = to_lower(h);
                    n = to_lower(n);
                }
                
                if (h != n)
                {
                    break;
                }
            }
        }
    }
    
    return result;
}





//~
static_function s64
str_compare(String a, String b)
{
    // NOTE: result -> (difference_index + 1)
    // if a.str[diff_index] < b.str[diff_index] then result is multiplied by -1
    // a.str[diff_index] > b.str[diff_index] result is positive
    
    s64 result = 0;
    
    for (s64 index = 0;
         ;
         index += 1)
    {
        b32 a_out_of_range = (a.size <= (u64)index);
        b32 b_out_of_range = (b.size <= (u64)index);
        
        if (a_out_of_range && b_out_of_range)
        {
            break;
        }
        else if (a_out_of_range)
        {
            result = -(index + 1);
            break;
        }
        else if (b_out_of_range)
        {
            result = (index + 1);
            break;
        }
        
        
        //-
        s32 delta = (a.str[index] - b.str[index]);
        if (delta != 0)
        {
            result = (index + 1)*sign_of(delta);
            break;
        }
    }
    
    return result;
}
static_function s64
str_compare(String16 a, String16 b)
{
    // NOTE: result -> (difference_index + 1)
    // if a.str[diff_index] < b.str[diff_index] then result is multiplied by -1
    // a.str[diff_index] > b.str[diff_index] result is positive
    
    s64 result = 0;
    
    for (s64 index = 0;
         ;
         index += 1)
    {
        b32 a_out_of_range = (a.size <= (u64)index);
        b32 b_out_of_range = (b.size <= (u64)index);
        
        if (a_out_of_range && b_out_of_range)
        {
            break;
        }
        else if (a_out_of_range)
        {
            result = -(index + 1);
            break;
        }
        else if (b_out_of_range)
        {
            result = (index + 1);
            break;
        }
        
        
        //-
        s32 delta = (a.str[index] - b.str[index]);
        if (delta != 0)
        {
            result = (index + 1)*sign_of(delta);
            break;
        }
    }
    
    return result;
}







//~
static_function Find_Index
str_index_of_from_table(String haystack, String table)
{
    Find_Index result = {};
    
    for_u64(haystack_index, haystack.size)
    {
        auto h = haystack.str[haystack_index];
        
        for_u64(table_index, table.size)
        {
            auto t = table.str[table_index];
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
static_function Find_Index
str_index_of_from_table(String16 haystack, String16 table)
{
    Find_Index result = {};
    
    for_u64(haystack_index, haystack.size)
    {
        auto h = haystack.str[haystack_index];
        
        for_u64(table_index, table.size)
        {
            auto t = table.str[table_index];
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


static_function Find_Index
str_index_of_reverse_from_table(String haystack, String table)
{
    Find_Index result = {};
    
    for_s64_rev(haystack_index, haystack.size)
    {
        auto h = haystack.str[haystack_index];
        
        for_u64(table_index, table.size)
        {
            auto t = table.str[table_index];
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
static_function Find_Index
str_index_of_reverse_from_table(String16 haystack, String16 table)
{
    Find_Index result = {};
    
    for_s64_rev(haystack_index, haystack.size)
    {
        auto h = haystack.str[haystack_index];
        
        for_u64(table_index, table.size)
        {
            auto t = table.str[table_index];
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




//~
static_function u64
str_count_of_from_table(String haystack, String table)
{
    u64 result = 0;
    
    for_u64(haystack_index, haystack.size)
    {
        auto h = haystack.str[haystack_index];
        
        for_u64(table_index, table.size)
        {
            auto t = table.str[table_index];
            if (h == t)
            {
                result += 1;
            }
        }
    }
    
    return result;
}
static_function u64
str_count_of_from_table(String16 haystack, String16 table)
{
    u64 result = 0;
    
    for_u64(haystack_index, haystack.size)
    {
        auto h = haystack.str[haystack_index];
        
        for_u64(table_index, table.size)
        {
            auto t = table.str[table_index];
            if (h == t)
            {
                result += 1;
            }
        }
    }
    
    return result;
}






//~
static_function String
str_trim_white(String text)
{
    String result = text;
    for (u64 index = 0;
         ;
         index += 1)
    {
        if (index >= result.size)
        {
            result = {};
            break;
        }
        
        auto u = result.str[index];
        if (!is_white(u))
        {
            result = str_skip(result, index);
            break;
        }
    }
    
    for_s64_rev(index, result.size)
    {
        auto u = result.str[index];
        if (!is_white(u))
        {
            result.size = index + 1;
            break;
        }
    }
    
    return result;
}
static_function String16
str_trim_white(String16 text)
{
    String16 result = text;
    for (u64 index = 0;
         ;
         index += 1)
    {
        if (index >= result.size)
        {
            result = {};
            break;
        }
        
        auto u = result.str[index];
        if (!is_white(u))
        {
            result = str_skip(result, index);
            break;
        }
    }
    
    for_s64_rev(index, result.size)
    {
        auto u = result.str[index];
        if (!is_white(u))
        {
            result.size = index + 1;
            break;
        }
    }
    
    return result;
}






// ================================================================
// @Section_source_string_helpers.h
// ================================================================


struct Compare_Line_Pos
{
    u32 line;
    u32 column;
    b32 is_equal;
};

static_function Compare_Line_Pos
str_compare_with_line_column(String value_a, String value_b, b32 case_ins = false)
{
    // primitive helper function to quickly compare two text files
    // TODO(f0): column counter that works with utf8?
    
    Compare_Line_Pos result = {};
    result.is_equal = true;
    
    u32 line = 1;
    u32 column = 1;
    
    for_u64(i, pick_smaller(value_a.size, value_b.size))
    {
        u8 a = value_a.str[i];
        u8 b = value_b.str[i];
        
        if (case_ins)
        {
            a = to_lower(a);
            b = to_lower(b);
        }
        
        if (a != b)
        {
            result.is_equal = false;
            break;
        }
        
        column += 1;
        
        if (a == '\n')
        {
            line += 1;
            column = 1;
        }
    }
    
    if (value_a.size != value_b.size)
    {
        result.is_equal = false;
    }
    
    if (!result.is_equal)
    {
        result.line = line;
        result.column = column;
    }
    
    return result;
}




static_function u64
cstr_hash(char *text)
{
    u64 hash = 5481;
    for (;;)
    {
        u32 t = (u32)text[0];
        if (t == 0) { break; }
        
        hash = hash*33 + t;
        text += 1;
    }
    return hash;
}

static_function u64
str_hash(String text)
{
    u64 hash = 5381;
    for_u64(text_index, text.size)
    {
        hash = ((hash << 5) + hash) + text.str[text_index];
    }
    return hash;
}






#define USE_SSE2 1
#pragma warning( push )
#pragma warning( disable : 4305 )

// ================================================================
// @Section_sse_mathfun.h
// ================================================================
/* SIMD (SSE1+MMX or SSE2) implementation of sin, cos, exp and log

   Inspired by Intel Approximate Math library, and based on the
   corresponding algorithms of the cephes math library

   The default is to use the SSE1 version. If you define USE_SSE2 the
   the SSE2 intrinsics will be used in place of the MMX intrinsics. Do
   not expect any significant performance improvement with SSE2.
*/

/* Copyright (C) 2007  Julien Pommier

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  (this is the zlib license)


================================================================
  NOTE(f0): This is a slightly modified version of this library.
  Mostly cosmetic changes in formating + some functions got deleted
================================================================
*/



#include <xmmintrin.h>

/* yes I know, the top of this file is quite ugly */

#ifdef _MSC_VER /* visual c++ */
# define ALIGN16_BEG __declspec(align(16))
# define ALIGN16_END 
#else /* gcc or icc */
# define ALIGN16_BEG
# define ALIGN16_END __attribute__((aligned(16)))
#endif

/* __m128 is ugly to write */
typedef __m128 v4sf;  // vector of 4 float (sse1)

#ifdef USE_SSE2
# include <emmintrin.h>
typedef __m128i v4si; // vector of 4 int (sse2)
#else
typedef __m64 v2si;   // vector of 2 int (mmx)
#endif

/* declare some SSE constants -- why can't I figure a better way to do that? */
#define _PS_CONST(Name, Val)                                            \
static const ALIGN16_BEG float _ps_##Name[4] ALIGN16_END = { Val, Val, Val, Val }
#define _PI32_CONST(Name, Val)                                            \
static const ALIGN16_BEG int _pi32_##Name[4] ALIGN16_END = { Val, Val, Val, Val }
#define _PS_CONST_TYPE(Name, Type, Val)                                 \
static const ALIGN16_BEG Type _ps_##Name[4] ALIGN16_END = { Val, Val, Val, Val }

_PS_CONST(1  , 1.0f);
_PS_CONST(0p5, 0.5f);
/* the smallest non denormalized float number */
_PS_CONST_TYPE(min_norm_pos, int, 0x00800000);
_PS_CONST_TYPE(mant_mask, int, 0x7f800000);
_PS_CONST_TYPE(inv_mant_mask, int, ~0x7f800000);

_PS_CONST_TYPE(sign_mask, int, (int)0x80000000);
_PS_CONST_TYPE(inv_sign_mask, int, ~0x80000000);

_PI32_CONST(1, 1);
_PI32_CONST(inv1, ~1);
_PI32_CONST(2, 2);
_PI32_CONST(4, 4);
_PI32_CONST(0x7f, 0x7f);

_PS_CONST(cephes_SQRTHF, 0.707106781186547524);
_PS_CONST(cephes_log_p0, 7.0376836292E-2);
_PS_CONST(cephes_log_p1, - 1.1514610310E-1);
_PS_CONST(cephes_log_p2, 1.1676998740E-1);
_PS_CONST(cephes_log_p3, - 1.2420140846E-1);
_PS_CONST(cephes_log_p4, + 1.4249322787E-1);
_PS_CONST(cephes_log_p5, - 1.6668057665E-1);
_PS_CONST(cephes_log_p6, + 2.0000714765E-1);
_PS_CONST(cephes_log_p7, - 2.4999993993E-1);
_PS_CONST(cephes_log_p8, + 3.3333331174E-1);
_PS_CONST(cephes_log_q1, -2.12194440e-4);
_PS_CONST(cephes_log_q2, 0.693359375);

#ifndef USE_SSE2
union xmm_mm_union
{
    __m128 xmm;
    __m64 mm[2];
};

#define COPY_XMM_TO_MM(xmm_, mm0_, mm1_) {      \
xmm_mm_union u; u.xmm = xmm_;                   \
mm0_ = u.mm[0];                                 \
mm1_ = u.mm[1];                                 \
}

#define COPY_MM_TO_XMM(mm0_, mm1_, xmm_) {                \
xmm_mm_union u; u.mm[0]=mm0_; u.mm[1]=mm1_; xmm_ = u.xmm; \
}

#endif // USE_SSE2




//~
_PS_CONST(minus_cephes_DP1, -0.78515625);
_PS_CONST(minus_cephes_DP2, -2.4187564849853515625e-4);
_PS_CONST(minus_cephes_DP3, -3.77489497744594108e-8);
_PS_CONST(sincof_p0, -1.9515295891E-4);
_PS_CONST(sincof_p1,  8.3321608736E-3);
_PS_CONST(sincof_p2, -1.6666654611E-1);
_PS_CONST(coscof_p0,  2.443315711809948E-005);
_PS_CONST(coscof_p1, -1.388731625493765E-003);
_PS_CONST(coscof_p2,  4.166664568298827E-002);
_PS_CONST(cephes_FOPI, 1.27323954473516); // 4 / M_PI


/* evaluation of 4 sines at onces, using only SSE1+MMX intrinsics so
   it runs also on old athlons XPs and the pentium III of your grand
   mother.

   The code is the exact rewriting of the cephes sinf function.
   Precision is excellent as long as x < 8192 (I did not bother to
   take into account the special handling they have for greater values
   -- it does not return garbage for arguments over 8192, though, but
   the extra precision is missing).

   Note that it is such that sinf((float)M_PI) = 8.74e-8, which is the
   surprising but correct result.

   Performance is also surprisingly good, 1.33 times faster than the
   macos vsinf SSE2 function, and 1.5 times faster than the
   __vrs4_sinf of amd's ACML (which is only available in 64 bits). Not
   too bad for an SSE1 function (with no special tuning) !
   However the latter libraries probably have a much better handling of NaN,
   Inf, denormalized and other special arguments..

   On my core 1 duo, the execution of this function takes approximately 95 cycles.

   From what I have observed on the experiments with Intel AMath lib, switching to an
   SSE2 version would improve the perf by only 10%.

   Since it is based on SSE intrinsics, it has to be compiled at -O2 to
   deliver full speed.
*/

static_function v4sf
sin_ps(v4sf x)
{ // any x
    v4sf xmm1, xmm2 = _mm_setzero_ps(), xmm3, sign_bit, y;
    
#ifdef USE_SSE2
    v4si emm0, emm2;
#else
    v2si mm0, mm1, mm2, mm3;
#endif
    sign_bit = x;
    /* take the absolute value */
    x = _mm_and_ps(x, *(v4sf*)_ps_inv_sign_mask);
    /* extract the sign bit (upper one) */
    sign_bit = _mm_and_ps(sign_bit, *(v4sf*)_ps_sign_mask);
    
    /* scale by 4/Pi */
    y = _mm_mul_ps(x, *(v4sf*)_ps_cephes_FOPI);
    
#ifdef USE_SSE2
    /* store the integer part of y in mm0 */
    emm2 = _mm_cvttps_epi32(y);
    /* j=(j+1) & (~1) (see the cephes sources) */
    emm2 = _mm_add_epi32(emm2, *(v4si*)_pi32_1);
    emm2 = _mm_and_si128(emm2, *(v4si*)_pi32_inv1);
    y = _mm_cvtepi32_ps(emm2);
    
    /* get the swap sign flag */
    emm0 = _mm_and_si128(emm2, *(v4si*)_pi32_4);
    emm0 = _mm_slli_epi32(emm0, 29);
    /* get the polynom selection mask 
       there is one polynom for 0 <= x <= Pi/4
       and another one for Pi/4<x<=Pi/2
  
       Both branches will be computed.
    */
    emm2 = _mm_and_si128(emm2, *(v4si*)_pi32_2);
    emm2 = _mm_cmpeq_epi32(emm2, _mm_setzero_si128());
    
    v4sf swap_sign_bit = _mm_castsi128_ps(emm0);
    v4sf poly_mask = _mm_castsi128_ps(emm2);
    sign_bit = _mm_xor_ps(sign_bit, swap_sign_bit);
    
#else
    /* store the integer part of y in mm0:mm1 */
    xmm2 = _mm_movehl_ps(xmm2, y);
    mm2 = _mm_cvttps_pi32(y);
    mm3 = _mm_cvttps_pi32(xmm2);
    /* j=(j+1) & (~1) (see the cephes sources) */
    mm2 = _mm_add_pi32(mm2, *(v2si*)_pi32_1);
    mm3 = _mm_add_pi32(mm3, *(v2si*)_pi32_1);
    mm2 = _mm_and_si64(mm2, *(v2si*)_pi32_inv1);
    mm3 = _mm_and_si64(mm3, *(v2si*)_pi32_inv1);
    y = _mm_cvtpi32x2_ps(mm2, mm3);
    /* get the swap sign flag */
    mm0 = _mm_and_si64(mm2, *(v2si*)_pi32_4);
    mm1 = _mm_and_si64(mm3, *(v2si*)_pi32_4);
    mm0 = _mm_slli_pi32(mm0, 29);
    mm1 = _mm_slli_pi32(mm1, 29);
    /* get the polynom selection mask */
    mm2 = _mm_and_si64(mm2, *(v2si*)_pi32_2);
    mm3 = _mm_and_si64(mm3, *(v2si*)_pi32_2);
    mm2 = _mm_cmpeq_pi32(mm2, _mm_setzero_si64());
    mm3 = _mm_cmpeq_pi32(mm3, _mm_setzero_si64());
    v4sf swap_sign_bit, poly_mask;
    COPY_MM_TO_XMM(mm0, mm1, swap_sign_bit);
    COPY_MM_TO_XMM(mm2, mm3, poly_mask);
    sign_bit = _mm_xor_ps(sign_bit, swap_sign_bit);
    _mm_empty(); /* good-bye mmx */
#endif
    
    /* The magic pass: "Extended precision modular arithmetic" 
       x = ((x - y * DP1) - y * DP2) - y * DP3; */
    xmm1 = *(v4sf*)_ps_minus_cephes_DP1;
    xmm2 = *(v4sf*)_ps_minus_cephes_DP2;
    xmm3 = *(v4sf*)_ps_minus_cephes_DP3;
    xmm1 = _mm_mul_ps(y, xmm1);
    xmm2 = _mm_mul_ps(y, xmm2);
    xmm3 = _mm_mul_ps(y, xmm3);
    x = _mm_add_ps(x, xmm1);
    x = _mm_add_ps(x, xmm2);
    x = _mm_add_ps(x, xmm3);
    
    /* Evaluate the first polynom  (0 <= x <= Pi/4) */
    y = *(v4sf*)_ps_coscof_p0;
    v4sf z = _mm_mul_ps(x,x);
    
    y = _mm_mul_ps(y, z);
    y = _mm_add_ps(y, *(v4sf*)_ps_coscof_p1);
    y = _mm_mul_ps(y, z);
    y = _mm_add_ps(y, *(v4sf*)_ps_coscof_p2);
    y = _mm_mul_ps(y, z);
    y = _mm_mul_ps(y, z);
    v4sf tmp = _mm_mul_ps(z, *(v4sf*)_ps_0p5);
    y = _mm_sub_ps(y, tmp);
    y = _mm_add_ps(y, *(v4sf*)_ps_1);
    
    /* Evaluate the second polynom  (Pi/4 <= x <= 0) */
    
    v4sf y2 = *(v4sf*)_ps_sincof_p0;
    y2 = _mm_mul_ps(y2, z);
    y2 = _mm_add_ps(y2, *(v4sf*)_ps_sincof_p1);
    y2 = _mm_mul_ps(y2, z);
    y2 = _mm_add_ps(y2, *(v4sf*)_ps_sincof_p2);
    y2 = _mm_mul_ps(y2, z);
    y2 = _mm_mul_ps(y2, x);
    y2 = _mm_add_ps(y2, x);
    
    /* select the correct result from the two polynoms */  
    xmm3 = poly_mask;
    y2 = _mm_and_ps(xmm3, y2); //, xmm3);
    y = _mm_andnot_ps(xmm3, y);
    y = _mm_add_ps(y,y2);
    /* update the sign */
    y = _mm_xor_ps(y, sign_bit);
    return y;
}





//~
/* almost the same as sin_ps */
static_function v4sf
cos_ps(v4sf x)
{ // any x
    v4sf xmm1, xmm2 = _mm_setzero_ps(), xmm3, y;
#ifdef USE_SSE2
    v4si emm0, emm2;
#else
    v2si mm0, mm1, mm2, mm3;
#endif
    /* take the absolute value */
    x = _mm_and_ps(x, *(v4sf*)_ps_inv_sign_mask);
    
    /* scale by 4/Pi */
    y = _mm_mul_ps(x, *(v4sf*)_ps_cephes_FOPI);
    
#ifdef USE_SSE2
    /* store the integer part of y in mm0 */
    emm2 = _mm_cvttps_epi32(y);
    /* j=(j+1) & (~1) (see the cephes sources) */
    emm2 = _mm_add_epi32(emm2, *(v4si*)_pi32_1);
    emm2 = _mm_and_si128(emm2, *(v4si*)_pi32_inv1);
    y = _mm_cvtepi32_ps(emm2);
    
    emm2 = _mm_sub_epi32(emm2, *(v4si*)_pi32_2);
    
    /* get the swap sign flag */
    emm0 = _mm_andnot_si128(emm2, *(v4si*)_pi32_4);
    emm0 = _mm_slli_epi32(emm0, 29);
    /* get the polynom selection mask */
    emm2 = _mm_and_si128(emm2, *(v4si*)_pi32_2);
    emm2 = _mm_cmpeq_epi32(emm2, _mm_setzero_si128());
    
    v4sf sign_bit = _mm_castsi128_ps(emm0);
    v4sf poly_mask = _mm_castsi128_ps(emm2);
#else
    /* store the integer part of y in mm0:mm1 */
    xmm2 = _mm_movehl_ps(xmm2, y);
    mm2 = _mm_cvttps_pi32(y);
    mm3 = _mm_cvttps_pi32(xmm2);
    
    /* j=(j+1) & (~1) (see the cephes sources) */
    mm2 = _mm_add_pi32(mm2, *(v2si*)_pi32_1);
    mm3 = _mm_add_pi32(mm3, *(v2si*)_pi32_1);
    mm2 = _mm_and_si64(mm2, *(v2si*)_pi32_inv1);
    mm3 = _mm_and_si64(mm3, *(v2si*)_pi32_inv1);
    
    y = _mm_cvtpi32x2_ps(mm2, mm3);
    
    
    mm2 = _mm_sub_pi32(mm2, *(v2si*)_pi32_2);
    mm3 = _mm_sub_pi32(mm3, *(v2si*)_pi32_2);
    
    /* get the swap sign flag in mm0:mm1 and the 
       polynom selection mask in mm2:mm3 */
    
    mm0 = _mm_andnot_si64(mm2, *(v2si*)_pi32_4);
    mm1 = _mm_andnot_si64(mm3, *(v2si*)_pi32_4);
    mm0 = _mm_slli_pi32(mm0, 29);
    mm1 = _mm_slli_pi32(mm1, 29);
    
    mm2 = _mm_and_si64(mm2, *(v2si*)_pi32_2);
    mm3 = _mm_and_si64(mm3, *(v2si*)_pi32_2);
    
    mm2 = _mm_cmpeq_pi32(mm2, _mm_setzero_si64());
    mm3 = _mm_cmpeq_pi32(mm3, _mm_setzero_si64());
    
    v4sf sign_bit, poly_mask;
    COPY_MM_TO_XMM(mm0, mm1, sign_bit);
    COPY_MM_TO_XMM(mm2, mm3, poly_mask);
    _mm_empty(); /* good-bye mmx */
#endif
    /* The magic pass: "Extended precision modular arithmetic" 
       x = ((x - y * DP1) - y * DP2) - y * DP3; */
    xmm1 = *(v4sf*)_ps_minus_cephes_DP1;
    xmm2 = *(v4sf*)_ps_minus_cephes_DP2;
    xmm3 = *(v4sf*)_ps_minus_cephes_DP3;
    xmm1 = _mm_mul_ps(y, xmm1);
    xmm2 = _mm_mul_ps(y, xmm2);
    xmm3 = _mm_mul_ps(y, xmm3);
    x = _mm_add_ps(x, xmm1);
    x = _mm_add_ps(x, xmm2);
    x = _mm_add_ps(x, xmm3);
    
    /* Evaluate the first polynom  (0 <= x <= Pi/4) */
    y = *(v4sf*)_ps_coscof_p0;
    v4sf z = _mm_mul_ps(x,x);
    
    y = _mm_mul_ps(y, z);
    y = _mm_add_ps(y, *(v4sf*)_ps_coscof_p1);
    y = _mm_mul_ps(y, z);
    y = _mm_add_ps(y, *(v4sf*)_ps_coscof_p2);
    y = _mm_mul_ps(y, z);
    y = _mm_mul_ps(y, z);
    v4sf tmp = _mm_mul_ps(z, *(v4sf*)_ps_0p5);
    y = _mm_sub_ps(y, tmp);
    y = _mm_add_ps(y, *(v4sf*)_ps_1);
    
    /* Evaluate the second polynom  (Pi/4 <= x <= 0) */
    
    v4sf y2 = *(v4sf*)_ps_sincof_p0;
    y2 = _mm_mul_ps(y2, z);
    y2 = _mm_add_ps(y2, *(v4sf*)_ps_sincof_p1);
    y2 = _mm_mul_ps(y2, z);
    y2 = _mm_add_ps(y2, *(v4sf*)_ps_sincof_p2);
    y2 = _mm_mul_ps(y2, z);
    y2 = _mm_mul_ps(y2, x);
    y2 = _mm_add_ps(y2, x);
    
    /* select the correct result from the two polynoms */  
    xmm3 = poly_mask;
    y2 = _mm_and_ps(xmm3, y2); //, xmm3);
    y = _mm_andnot_ps(xmm3, y);
    y = _mm_add_ps(y,y2);
    /* update the sign */
    y = _mm_xor_ps(y, sign_bit);
    
    return y;
}


//~
/* since sin_ps and cos_ps are almost identical, sincos_ps could replace both of them..
   it is almost as fast, and gives you a free cosine with your sine */
static_function void
sincos_ps(v4sf x, v4sf *s, v4sf *c)
{
    v4sf xmm1, xmm2, xmm3 = _mm_setzero_ps(), sign_bit_sin, y;
#ifdef USE_SSE2
    v4si emm0, emm2, emm4;
#else
    v2si mm0, mm1, mm2, mm3, mm4, mm5;
#endif
    sign_bit_sin = x;
    /* take the absolute value */
    x = _mm_and_ps(x, *(v4sf*)_ps_inv_sign_mask);
    /* extract the sign bit (upper one) */
    sign_bit_sin = _mm_and_ps(sign_bit_sin, *(v4sf*)_ps_sign_mask);
    
    /* scale by 4/Pi */
    y = _mm_mul_ps(x, *(v4sf*)_ps_cephes_FOPI);
    
#ifdef USE_SSE2
    /* store the integer part of y in emm2 */
    emm2 = _mm_cvttps_epi32(y);
    
    /* j=(j+1) & (~1) (see the cephes sources) */
    emm2 = _mm_add_epi32(emm2, *(v4si*)_pi32_1);
    emm2 = _mm_and_si128(emm2, *(v4si*)_pi32_inv1);
    y = _mm_cvtepi32_ps(emm2);
    
    emm4 = emm2;
    
    /* get the swap sign flag for the sine */
    emm0 = _mm_and_si128(emm2, *(v4si*)_pi32_4);
    emm0 = _mm_slli_epi32(emm0, 29);
    v4sf swap_sign_bit_sin = _mm_castsi128_ps(emm0);
    
    /* get the polynom selection mask for the sine*/
    emm2 = _mm_and_si128(emm2, *(v4si*)_pi32_2);
    emm2 = _mm_cmpeq_epi32(emm2, _mm_setzero_si128());
    v4sf poly_mask = _mm_castsi128_ps(emm2);
#else
    /* store the integer part of y in mm2:mm3 */
    xmm3 = _mm_movehl_ps(xmm3, y);
    mm2 = _mm_cvttps_pi32(y);
    mm3 = _mm_cvttps_pi32(xmm3);
    
    /* j=(j+1) & (~1) (see the cephes sources) */
    mm2 = _mm_add_pi32(mm2, *(v2si*)_pi32_1);
    mm3 = _mm_add_pi32(mm3, *(v2si*)_pi32_1);
    mm2 = _mm_and_si64(mm2, *(v2si*)_pi32_inv1);
    mm3 = _mm_and_si64(mm3, *(v2si*)_pi32_inv1);
    
    y = _mm_cvtpi32x2_ps(mm2, mm3);
    
    mm4 = mm2;
    mm5 = mm3;
    
    /* get the swap sign flag for the sine */
    mm0 = _mm_and_si64(mm2, *(v2si*)_pi32_4);
    mm1 = _mm_and_si64(mm3, *(v2si*)_pi32_4);
    mm0 = _mm_slli_pi32(mm0, 29);
    mm1 = _mm_slli_pi32(mm1, 29);
    v4sf swap_sign_bit_sin;
    COPY_MM_TO_XMM(mm0, mm1, swap_sign_bit_sin);
    
    /* get the polynom selection mask for the sine */
    
    mm2 = _mm_and_si64(mm2, *(v2si*)_pi32_2);
    mm3 = _mm_and_si64(mm3, *(v2si*)_pi32_2);
    mm2 = _mm_cmpeq_pi32(mm2, _mm_setzero_si64());
    mm3 = _mm_cmpeq_pi32(mm3, _mm_setzero_si64());
    v4sf poly_mask;
    COPY_MM_TO_XMM(mm2, mm3, poly_mask);
#endif
    
    /* The magic pass: "Extended precision modular arithmetic" 
       x = ((x - y * DP1) - y * DP2) - y * DP3; */
    xmm1 = *(v4sf*)_ps_minus_cephes_DP1;
    xmm2 = *(v4sf*)_ps_minus_cephes_DP2;
    xmm3 = *(v4sf*)_ps_minus_cephes_DP3;
    xmm1 = _mm_mul_ps(y, xmm1);
    xmm2 = _mm_mul_ps(y, xmm2);
    xmm3 = _mm_mul_ps(y, xmm3);
    x = _mm_add_ps(x, xmm1);
    x = _mm_add_ps(x, xmm2);
    x = _mm_add_ps(x, xmm3);
    
#ifdef USE_SSE2
    emm4 = _mm_sub_epi32(emm4, *(v4si*)_pi32_2);
    emm4 = _mm_andnot_si128(emm4, *(v4si*)_pi32_4);
    emm4 = _mm_slli_epi32(emm4, 29);
    v4sf sign_bit_cos = _mm_castsi128_ps(emm4);
#else
    /* get the sign flag for the cosine */
    mm4 = _mm_sub_pi32(mm4, *(v2si*)_pi32_2);
    mm5 = _mm_sub_pi32(mm5, *(v2si*)_pi32_2);
    mm4 = _mm_andnot_si64(mm4, *(v2si*)_pi32_4);
    mm5 = _mm_andnot_si64(mm5, *(v2si*)_pi32_4);
    mm4 = _mm_slli_pi32(mm4, 29);
    mm5 = _mm_slli_pi32(mm5, 29);
    v4sf sign_bit_cos;
    COPY_MM_TO_XMM(mm4, mm5, sign_bit_cos);
    _mm_empty(); /* good-bye mmx */
#endif
    
    sign_bit_sin = _mm_xor_ps(sign_bit_sin, swap_sign_bit_sin);
    
    
    /* Evaluate the first polynom  (0 <= x <= Pi/4) */
    v4sf z = _mm_mul_ps(x,x);
    y = *(v4sf*)_ps_coscof_p0;
    
    y = _mm_mul_ps(y, z);
    y = _mm_add_ps(y, *(v4sf*)_ps_coscof_p1);
    y = _mm_mul_ps(y, z);
    y = _mm_add_ps(y, *(v4sf*)_ps_coscof_p2);
    y = _mm_mul_ps(y, z);
    y = _mm_mul_ps(y, z);
    v4sf tmp = _mm_mul_ps(z, *(v4sf*)_ps_0p5);
    y = _mm_sub_ps(y, tmp);
    y = _mm_add_ps(y, *(v4sf*)_ps_1);
    
    /* Evaluate the second polynom  (Pi/4 <= x <= 0) */
    
    v4sf y2 = *(v4sf*)_ps_sincof_p0;
    y2 = _mm_mul_ps(y2, z);
    y2 = _mm_add_ps(y2, *(v4sf*)_ps_sincof_p1);
    y2 = _mm_mul_ps(y2, z);
    y2 = _mm_add_ps(y2, *(v4sf*)_ps_sincof_p2);
    y2 = _mm_mul_ps(y2, z);
    y2 = _mm_mul_ps(y2, x);
    y2 = _mm_add_ps(y2, x);
    
    /* select the correct result from the two polynoms */  
    xmm3 = poly_mask;
    v4sf ysin2 = _mm_and_ps(xmm3, y2);
    v4sf ysin1 = _mm_andnot_ps(xmm3, y);
    y2 = _mm_sub_ps(y2,ysin2);
    y = _mm_sub_ps(y, ysin1);
    
    xmm1 = _mm_add_ps(ysin1,ysin2);
    xmm2 = _mm_add_ps(y,y2);
    
    /* update the sign */
    *s = _mm_xor_ps(xmm1, sign_bit_sin);
    *c = _mm_xor_ps(xmm2, sign_bit_cos);
}



//~ =================== End of sse_mathfun.h ==================

#pragma warning( pop )


// ================================================================
// @Section_source_math_f32_4x.h
// ================================================================
static_function f32_4x
load_f32_4x(f32 *address)
{
    return {_mm_loadu_ps(address)};
}

static_function void
store_f32_4x(f32 *address, f32_4x a)
{
    _mm_storeu_ps(address, a.p);
}

static_function f32_4x
F32_4x(f32 all_4)
{
    return {_mm_set1_ps(all_4)};
}

static_function f32_4x
F32_4x(f32 a, f32 b, f32 c, f32 d)
{
    return {a, b, c, d};
}


//-
static_function f32_4x
operator+(f32_4x a, f32_4x b)
{
    return {_mm_add_ps(a.p, b.p)};
}

static_function f32_4x
operator-(f32_4x a, f32_4x b)
{
    return {_mm_sub_ps(a.p, b.p)};
}

static_function f32_4x
operator*(f32_4x a, f32_4x b)
{
    return {_mm_mul_ps(a.p, b.p)};
}

static_function b32
operator==(f32_4x a, f32_4x b)
{
    return (_mm_movemask_ps(_mm_cmpeq_ps(a.p, b.p)) == 0xF);
}

//-
static_function f32_4x
operator+=(f32_4x &a, f32_4x b)
{
    a = a + b;
    return a;
}

static_function f32_4x
operator-=(f32_4x &a, f32_4x b)
{
    a = a - b;
    return a;
}

static_function f32_4x
operator*=(f32_4x &a, f32_4x b)
{
    a = a * b;
    return a;
}


//-
static_function f32_4x
operator+(f32_4x a, f32 b)
{
    return a + F32_4x(b);
}
static_function f32_4x operator+(f32 a, f32_4x b) { return b + a; }

static_function f32_4x
operator-(f32_4x a, f32 b)
{
    return a - F32_4x(b);
}
static_function f32_4x operator-(f32 a, f32_4x b) { return b - a; }

static_function f32_4x
operator*(f32_4x a, f32 b)
{
    return a * F32_4x(b);
}
static_function f32_4x operator*(f32 a, f32_4x b) { return b * a; }


//-
static_function f32_4x
operator+=(f32_4x &a, f32 b)
{
    a = a + b;
    return a;
}

static_function f32_4x
operator-=(f32_4x &a, f32 b)
{
    a = a - b;
    return a;
}

static_function f32_4x
operator*=(f32_4x &a, f32 b)
{
    a = a * b;
    return a;
}



//~
static_function f32_4x
floor_f32_4x(f32_4x a)
{
    return {_mm_round_ps(a.p, Def_SSE4_Rounding_Floor)};
}

static_function f32_4x
ceil_f32_4x(f32_4x a)
{
    return {_mm_round_ps(a.p, Def_SSE4_Rounding_Ceil)};
}

static_function f32_4x
round_f32_4x(f32_4x a)
{
    return {_mm_round_ps(a.p, Def_SSE4_Rounding_Nearest)};
}

static_function f32_4x
truncate_f32_4x(f32_4x a)
{
    return {_mm_round_ps(a.p, Def_SSE4_Rounding_Truncate)};
}


// ================================================================
// @Section_source_math_intrinsics.h
// ================================================================

struct Bit_Scan_Result
{
    u32 index;
    b32 found;
};

//-
static_function b32
equals(Bit_Scan_Result a, Bit_Scan_Result b)
{
    b32 result = false;
    if (a.found && b.found)
    {
        if (a.index == b.index)
        {
            result = true;
        }
    }
    else if (!a.found && !b.found)
    {
        result = true;
    }
    return result;
}

static_function b32
operator==(Bit_Scan_Result a, Bit_Scan_Result b)
{
    b32 result = equals(a, b);
    return result;
}


//~
static_function Bit_Scan_Result
find_most_significant_bit(u64 value)
{
    Bit_Scan_Result result = {};
#if Def_Compiler_Msvc
    unsigned long index;
    result.found = _BitScanReverse64(&index, value);
    result.index = index;
#else
    if (value)
    {
        result.found = true;
        result.index = 63 - __builtin_clzll(value);
    }
#endif
    return result;
}
static_function Bit_Scan_Result find_most_significant_bit(s64 value) {
    return find_most_significant_bit((u64)value);
}

//-
static_function Bit_Scan_Result
find_most_significant_bit(u32 value)
{
    Bit_Scan_Result result = {};
#if Def_Compiler_Msvc
    unsigned long index;
    result.found = _BitScanReverse(&index, value);
    result.index = index;
#else
    if (value)
    {
        result.found = true;
        result.index = 31 - __builtin_clz(value);
    }
#endif
    return result;
}
static_function Bit_Scan_Result find_most_significant_bit(s32 value) {
    return find_most_significant_bit((u32)value);
}


//-///////////////////////////////////////////
static_function Bit_Scan_Result
find_least_significant_bit(u32 value)
{
    Bit_Scan_Result result = {};
#if Def_Compiler_Msvc
    unsigned long index;
    result.found = _BitScanForward(&index, value);
    result.index = index;
    
#else
    if (value)
    {
        result.found = true;
        result.index = __builtin_ctz(value);
    }
#endif
    return result;
}
static_function Bit_Scan_Result find_least_significant_bit(s32 value) {
    return find_least_significant_bit((u32)value);
};


//-
static_function Bit_Scan_Result
find_least_significant_bit(u64 value)
{
    Bit_Scan_Result result = {};
    
#if Def_Compiler_Msvc
    unsigned long index;
    result.found = _BitScanForward64(&index, value);
    result.index = index;
#else
    if (value)
    {
        result.found = true;
        result.index = __builtin_ctzll(value);
    }
#endif
    return result;
}
static_function Bit_Scan_Result find_least_significant_bit(s64 value) {
    return find_least_significant_bit((u64)value);
};









//~
static_function b32
is_non_zero_power_of_two(u64 value)
{
    Bit_Scan_Result msb = find_most_significant_bit(value);
    Bit_Scan_Result lsb = find_least_significant_bit(value);
    b32 result = (msb.found && lsb.found &&
                  msb.index == lsb.index);
    return result;
}



//~
//~
static_function f32
square_root(f32 value)
{
    f32 result = _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(value)));
    return result;
}


static_function u32
rotate_left(u32 value, s32 amount)
{
#if Def_Compiler_Msvc
    u32 result = _rotl(value, amount);
#else
    u32 result = __builtin_rotateleft32(value, amount);
#endif
    
    return result;
}
static_function u32
rotate_right(u32 value, s32 amount)
{
#if Def_Compiler_Msvc
    u32 result = _rotr(value, amount);
#else
    u32 result = __builtin_rotateright32(value, amount);
#endif
    
    return result;
}

static_function s64
rotate_left(s64 value, s32 amount)
{
    s64 result = _rotl64(value, amount);
    return result;
}
static_function u64
rotate_left(u64 value, s32 amount)
{
    u64 result = _rotl64(value, amount);
    return result;
}

static_function s64
rotate_right(s64 value, s32 amount)
{
    s64 result = _rotr64(value, amount);
    return result;
}
static_function u64
rotate_right(u64 value, s32 amount)
{
    u64 result = _rotr64(value, amount);
    return result;
}




//-
static_function f32
floor_f32(f32 value)
{
    f32 result = _mm_cvtss_f32(_mm_round_ss(_mm_setzero_ps(), _mm_set_ss(value), Def_SSE4_Rounding_Floor));
    return result;
}

static_function s32
floor_f32_to_s32(f32 value)
{
    s32 result = _mm_cvtss_si32(_mm_round_ss(_mm_setzero_ps(), _mm_set_ss(value), Def_SSE4_Rounding_Floor));
    return result;
}

//-
static_function f32
ceil_f32(f32 value)
{
    f32 result = _mm_cvtss_f32(_mm_round_ss(_mm_setzero_ps(), _mm_set_ss(value), Def_SSE4_Rounding_Ceil));
    return result;
}

static_function s32
ceil_f32_to_s32(f32 value)
{
    s32 result = _mm_cvtss_si32(_mm_round_ss(_mm_setzero_ps(), _mm_set_ss(value), Def_SSE4_Rounding_Ceil));
    return result;
}

//-
static_function f32
round_f32(f32 value)
{
    f32 result = _mm_cvtss_f32(_mm_round_ss(_mm_setzero_ps(), _mm_set_ss(value), Def_SSE4_Rounding_Nearest));
    return result;
};

static_function s32
round_f32_to_s32(f32 value)
{
    s32 result = _mm_cvtss_si32(_mm_round_ss(_mm_setzero_ps(), _mm_set_ss(value), Def_SSE4_Rounding_Nearest));
    return result;
}

static_function u32
round_f32_to_u32(f32 value)
{
    u32 result = safe_truncate_to_u32(round_f32_to_s32(value));
    return result;
}

//-
static_function s32
truncate_f32_to_s32(f32 value)
{
    s32 result = (s32)value;
    return result;
}


//~
static_function f32
sin(f32 angle)
{
    f32 result = _mm_cvtss_f32(sin_ps(_mm_set_ss(angle)));
    return result;
}

static_function f32
sin01(f32 t)
{
    return 0.5f + 0.5f*sin(t);
}

static_function f32_4x
sin(f32_4x angle)
{
    f32_4x result;
    result.p = sin_ps(angle.p);
    return result;
}

static_function f32_4x
sin01(f32_4x angle)
{
    f32_4x result = {sin_ps(angle.p)};
    f32_4x half = F32_4x(0.5f);
    result = result*half + half;
    return result;
}


//-
static_function f32
cos(f32 angle)
{
    f32 result = _mm_cvtss_f32(cos_ps(_mm_set_ss(angle)));
    return result;
}

static_function f32_4x
cos(f32_4x angle)
{
    f32_4x result;
    result.p = cos_ps(angle.p);
    return result;
}

//-
static_function void
sin_cos(f32_4x angle, f32_4x *sin_out, f32_4x *cos_out)
{
    sincos_ps(angle.p, &sin_out->p, &cos_out->p);
}

struct Sin_Cos
{
    f32 sin;
    f32 cos;
};

static_function Sin_Cos
sin_cos(f32 angle)
{
    f32_4x sin4;
    f32_4x cos4;
    f32_4x angle4 = {_mm_set_ss(angle)};
    sincos_ps(angle4.p, &sin4.p, &cos4.p);
    
    Sin_Cos result = {
        _mm_cvtss_f32(sin4.p),
        _mm_cvtss_f32(cos4.p),
    };
    return result;
};

        
//-
#if 0
static_function f32
atan2(f32 y, f32 x)
{
    f32 result = atan2f(y, x);
    return result;
}
#endif




// ================================================================
// @Section_source_math_m4x4.h
// ================================================================

static_function m4x4
identity()
{
    m4x4 result =
    {
        {
            {1, 0, 0, 0},
            {0, 1, 0, 0},
            {0, 0, 1, 0},
            {0, 0, 0, 1},
        },
    };
    return result;
}

static_function m4x4
x_rotation(f32 rad)
{
    Sin_Cos sc = sin_cos(rad);
    f32 s = sc.sin;
    f32 c = sc.cos;
    
    m4x4 result =
    {
        {
            { 1,  0,  0,  0},
            { 0,  c, -s,  0},
            { 0,  s,  c,  0},
            { 0,  0,  0,  1},
        }
    };
    return result;
}

static_function m4x4
y_rotation(f32 rad)
{
    Sin_Cos sc = sin_cos(rad);
    f32 s = sc.sin;
    f32 c = sc.cos;
    
    m4x4 result =
    {
        {
            { c,  0,  s,  0},
            { 0,  1,  0,  0},
            {-s,  0,  c,  0},
            { 0,  0,  0,  1},
        }
    };
    return result;
}

static_function m4x4
z_rotation(f32 rad)
{
    Sin_Cos sc = sin_cos(rad);
    f32 s = sc.sin;
    f32 c = sc.cos;
    
    m4x4 result =
    {
        {
            { c, -s,  0,  0},
            { s,  c,  0,  0},
            { 0,  0,  1,  0},
            { 0,  0,  0,  1},
        }
    };
    return result;
}

static_function m4x4
translation(v3 t)
{
    m4x4 result =
    {
        {
            {1, 0, 0, t.x},
            {0, 1, 0, t.y},
            {0, 0, 1, t.z},
            {0, 0, 0, 1},
        }
    };
    return result;
}

//~
//~

static_function m4x4
transpose(m4x4 a)
{
    m4x4 r = a;
    _MM_TRANSPOSE4_PS(r.row[0].p, r.row[1].p, r.row[2].p, r.row[3].p);
    return r;
}


//-
static_function m4x4
transform(m4x4 a, m4x4 b)
{
    m4x4 result = {};
    for(int r = 0; r <= 3; ++r) // NOTE(casey): Rows (of A)
    {
        for(int c = 0; c <= 3; ++c) // NOTE(casey): Column (of B)
        {
            for(int i = 0; i <= 3; ++i) // NOTE(casey): Columns of A, rows of B!
            {
                result.e[r][c] += a.e[r][i] * b.e[i][c];
            }
        }
    }
    return(result);
}

static_function v4
transform(m4x4 a, v4 p)
{
    v4 r;
    r.x = p.x*a.e[0][0] + p.y*a.e[0][1] + p.z*a.e[0][2] + p.w*a.e[0][3];
    r.y = p.x*a.e[1][0] + p.y*a.e[1][1] + p.z*a.e[1][2] + p.w*a.e[1][3];
    r.z = p.x*a.e[2][0] + p.y*a.e[2][1] + p.z*a.e[2][2] + p.w*a.e[2][3];
    r.w = p.x*a.e[3][0] + p.y*a.e[3][1] + p.z*a.e[3][2] + p.w*a.e[3][3];
    return(r);
}


//~
static_function b32
operator==(m4x4 a, m4x4 b)
{
    __m128 res128 = _mm_and_ps(_mm_and_ps(_mm_cmpeq_ps(a.row[0].p, b.row[0].p),
                                          _mm_cmpeq_ps(a.row[1].p, b.row[1].p)),
                               _mm_and_ps(_mm_cmpeq_ps(a.row[2].p, b.row[2].p),
                                          _mm_cmpeq_ps(a.row[3].p, b.row[3].p)));
    
    b32 result = (_mm_movemask_ps(res128) == 0xF);
    return result;
}


//-
static_function m4x4
operator*(m4x4 a, f32 scalar)
{
    f32_4x scalar4 = F32_4x(scalar);
    m4x4 result;
    result.row[0] = a.row[0] * scalar4;
    result.row[1] = a.row[1] * scalar4;
    result.row[2] = a.row[2] * scalar4;
    result.row[3] = a.row[3] * scalar4;
    return result;
}
static_function m4x4
operator*(f32 scalar, m4x4 a)
{
    return a*scalar;
}

static_function m4x4
operator*(m4x4 a, m4x4 b)
{
    return transform(a, b);
}

static_function v4
operator*(m4x4 a, v4 b)
{
    return transform(a, b);
}

static_function m4x4
operator+(m4x4 a, m4x4 b)
{
    m4x4 r;
    r.row[0] = a.row[0] + b.row[0];
    r.row[1] = a.row[1] + b.row[1];
    r.row[2] = a.row[2] + b.row[2];
    r.row[3] = a.row[3] + b.row[3];
    return r;
}

static_function m4x4
operator-(m4x4 a, m4x4 b)
{
    m4x4 r;
    r.row[0] = a.row[0] - b.row[0];
    r.row[1] = a.row[1] - b.row[1];
    r.row[2] = a.row[2] - b.row[2];
    r.row[3] = a.row[3] - b.row[3];
    return r;
}

//~
static_function m4x4
operator+=(m4x4 &a, m4x4 b)
{
    a = a + b;
    return a;   
}
static_function m4x4
operator-=(m4x4 &a, m4x4 b)
{
    a = a - b;
    return a;   
}
static_function m4x4
operator*=(m4x4 &a, m4x4 b)
{
    a = a * b;
    return a;   
}
static_function m4x4
operator*=(m4x4 &a, f32 b)
{
    a = a * b;
    return a;   
}






#if 0
// TODO(f0): These implementations roughly target SSE4
// AVX implementations might be faster (citation needed)
// None of this was profiled so it is a huge BS anyway
// I just implemented it for fun
// Very old & rough example of how AVX version can be done:
// https://gist.github.com/rygorous/4172889
//
// I'll stick to naive implementations for now and let the compiler do its magic
// This should get looked at once I have some opprutinity to profile this code
static_function m4x4
sse_multiply_m4x4_m4x4(m4x4 a, m4x4 b)
{
    m4x4 result;
    
    for_u64(i, 4)
    {
        f32_4x ac[4];
        ac[0] = F32_4x(a.e[i][0]);
        ac[1] = F32_4x(a.e[i][1]);
        ac[2] = F32_4x(a.e[i][2]);
        ac[3] = F32_4x(a.e[i][3]);
        
        result.row[i] = ((ac[0] * b.row[0]) + (ac[1] * b.row[1]) +
                         (ac[2] * b.row[2]) + (ac[3] * b.row[3]));
    }
    
    return result;
}

static_function v4
sse_multiply_m4x4_v4(m4x4 a, v4 b)
{
    // TODO(f0): Can I do better without reorganizing matrix to be stored as [col][row]?
    // (probably yes)
    a = transpose(a);
    
    f32_4x bc[4];
    bc[0] = F32_4x(b.x);
    bc[1] = F32_4x(b.y);
    bc[2] = F32_4x(b.z);
    bc[3] = F32_4x(b.w);
    
    f32_4x result = ((a.row[0] * bc[0]) + (a.row[1] * bc[1]) +
                     (a.row[2] * bc[2]) + (a.row[3] * bc[3]));
    return result.vec;
}
#endif



// ================================================================
// @Section_source_math_v2.h
// ================================================================
static_function v2
arm2(f32 angle)
{
    v2 result = {cos(angle), sin(angle)};
    return result;
}

static_function v2
perp(v2 a)
{
    v2 result = {-a.y, a.x};
    return result;
}


//~
static_function void
clamp_ref_bot(f32 min, v2 *value)
{
    clamp_ref_bot(min, &value->x);
    clamp_ref_bot(min, &value->y);
}
static_function void
clamp_ref_top(v2 *value, f32 max)
{
    clamp_ref_top(&value->x, max);
    clamp_ref_top(&value->y, max);
}

static_function void
clamp_ref_bot(v2 min, v2 *value)
{
    clamp_ref_bot(min.x, &value->x);
    clamp_ref_bot(min.y, &value->y);
}
static_function void
clamp_ref_top(v2 *value, v2 max)
{
    clamp_ref_top(&value->x, max.x);
    clamp_ref_top(&value->y, max.y);
}

static_function v2
clamp01(v2 value)
{
    v2 result;
    result.x = clamp01(value.x);
    result.y = clamp01(value.y);
    return result;
}



//~
static_function v2
V2(f32 x, f32 y)
{
    v2 result = {x, y};
    return result;
}

static_function v2
V2(f32 xy)
{
    v2 result = {xy, xy};
    return result;
}

static_function v2
V2i(s32 x, s32 y)
{
    v2 result = {(f32)x, (f32)y};
    return result;
}

static_function v2
V2i(s32 xy)
{
    v2 result = {(f32)xy, (f32)xy};
    return result;
}

static_function v2
V2i(v2s a)
{
    v2 result = {(f32)a.x, (f32)a.y};
    return result;
}



static_function v2
operator*(f32 a, v2 b)
{
    v2 result {
        a * b.x,
        a * b.y
    };
    return result;
}

static_function v2
operator*(v2 a, f32 b)
{
    v2 result = b * a;
    return result;
}

static_function v2
operator-(v2 a)
{
    v2 result;
    result.x = -a.x;
    result.y = -a.y;
    return result;
}

static_function v2
operator+(v2 a, v2 b)
{
    v2 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return result;
}

static_function v2
operator-(v2 a, v2 b)
{
    v2 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    return result;
}

static_function v2
operator*=(v2 &a, f32 b)
{ 
    a = b * a;
    return a;
}

static_function v2
operator+=(v2 &a, v2 b)
{
    a = a + b;
    return a;   
}

static_function v2
operator-=(v2 &a, v2 b)
{
    a = a - b;
    return a;   
}




static_function v2
lerp(v2 a, f32 t, v2 b)
{
    v2 result = (1.0f - t)*a + t*b;
    return result;
}

static_function v2
hadamard(v2 a, v2 b)
{
    v2 result;
    result.x = a.x * b.x;
    result.y = a.y * b.y;
    
    return result;
}

static_function f32
inner(v2 a, v2 b)
{
    f32 result = a.x * b.x + a.y * b.y;
    return result;
}

static_function f32
get_length_sq(v2 a)
{
    f32 result = inner(a, a);
    return result;
}

static_function f32
get_length(v2 a)
{
    f32 result = square_root(get_length_sq(a));
    return result;
}

static_function v2
get_normalized(v2 a)
{
    v2 result = a * (1.f / get_length(a));
    return result;
}


// ================================================================
// @Section_source_math_v3.h
// ================================================================
static_function void
clamp_ref_bot(f32 min, v3 *value)
{
    clamp_ref_bot(min, &value->x);
    clamp_ref_bot(min, &value->y);
    clamp_ref_bot(min, &value->z);
}
static_function void
clamp_ref_top(v3 *value, f32 max)
{
    clamp_ref_top(&value->x, max);
    clamp_ref_top(&value->y, max);
    clamp_ref_top(&value->z, max);
}

static_function void
clamp_ref_bot(v3 min, v3 *value)
{
    clamp_ref_bot(min.x, &value->x);
    clamp_ref_bot(min.y, &value->y);
    clamp_ref_bot(min.z, &value->z);
}
static_function void
clamp_ref_top(v3 *value, v3 max)
{
    clamp_ref_top(&value->x, max.x);
    clamp_ref_top(&value->y, max.y);
    clamp_ref_top(&value->z, max.z);
}

static_function v3
clamp01(v3 value)
{
    v3 result;
    result.x = clamp01(value.x);
    result.y = clamp01(value.y);
    result.z = clamp01(value.z);
    return result;
}


//~
static_function v3
V3(f32 x, f32 y, f32 z)
{
    v3 result = {x, y, z};
    return result;
}

static_function v3
V3(v2 xy, f32 z)
{
    v3 result = {xy.x, xy.y, z};
    return result;
}

static_function v3
V3(f32 xyz)
{
    v3 result = {xyz, xyz, xyz};
    return result;
}

static_function v3
V3i(s32 x, s32 y, s32 z)
{
    v3 result = {(f32)x, (f32)y, (f32)z};
    return result;
}

static_function v3
operator*(f32 a, v3 b)
{
    v3 result;
    result.x = a * b.x;
    result.y = a * b.y;
    result.z = a * b.z;
    
    return result;
}

static_function v3
operator*(v3 a, f32 b)
{
    v3 result = b * a;
    return result;
}

static_function v3
operator-(v3 a)
{
    v3 result;
    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;
    
    return result;
}

static_function v3
operator+(v3 a, v3 b)
{
    v3 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    return result;
}

static_function v3
operator-(v3 a, v3 b)
{
    v3 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return result;
}

static_function v3
operator*=(v3 &a, f32 b)
{ 
    a = b * a;
    return a;
}

static_function v3
operator+=(v3 &a, v3 b)
{
    a = a + b;
    return a;   
}

static_function v3
operator-=(v3 &a, v3 b)
{
    a = a - b;
    return a;
}

static_function v3
lerp(v3 a, f32 t, v3 b)
{
    v3 result = (1.0f - t)*a + t*b;
    return result;
}

static_function v3
hadamard(v3 a, v3 b)
{
    v3 result;
    result.x = a.x * b.x;
    result.y = a.y * b.y; 
    result.z = a.z * b.z;
    
    return result;
}

static_function f32
inner(v3 a, v3 b)
{
    f32 result = (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
    return result;
}

static_function f32
get_length_sq(v3 a)
{
    f32 result = inner(a, a);
    return result;
}

static_function f32
get_length(v3 a)
{
    f32 result = square_root(get_length_sq(a));
    return result;
}

static_function v3
get_normalized(v3 a)
{
    v3 result = a * (1.f / get_length(a));
    return result;
}


// ================================================================
// @Section_source_math_v4.h
// ================================================================
static_function void
clamp_ref_bot(f32 min, v4 *value)
{
    clamp_ref_bot(min, &value->x);
    clamp_ref_bot(min, &value->y);
    clamp_ref_bot(min, &value->z);
    clamp_ref_bot(min, &value->w);
}
static_function void
clamp_ref_top(v4 *value, f32 max)
{
    clamp_ref_top(&value->x, max);
    clamp_ref_top(&value->y, max);
    clamp_ref_top(&value->z, max);
    clamp_ref_top(&value->w, max);
}

static_function void
clamp_ref_bot(v4 min, v4 *value)
{
    clamp_ref_bot(min.x, &value->x);
    clamp_ref_bot(min.y, &value->y);
    clamp_ref_bot(min.z, &value->z);
    clamp_ref_bot(min.w, &value->w);
}
static_function void
clamp_ref_top(v4 *value, v4 max)
{
    clamp_ref_top(&value->x, max.x);
    clamp_ref_top(&value->y, max.y);
    clamp_ref_top(&value->z, max.z);
    clamp_ref_top(&value->w, max.w);
}

static_function v4
clamp01(v4 value)
{
    v4 result;
    result.x = clamp01(value.x);
    result.y = clamp01(value.y);
    result.z = clamp01(value.z);
    result.w = clamp01(value.w);
    return result;
}


//~
static_function v4
V4(f32 x, f32 y, f32 z, f32 w)
{
    v4 result = {x, y, z, w};
    return result;
}

static_function v4
V4(v3 xyz, f32 w)
{
    v4 result;
    result.xyz = xyz;
    result.w = w;
    return result;
}


static_function v4
V4(f32 scalar_xyzw)
{
    v4 result = {scalar_xyzw, scalar_xyzw, scalar_xyzw, scalar_xyzw};
    return result;
}


static_function v4
V4i(s32 x, s32 y, s32 z, s32 w)
{
    v4 result = { (f32)x, (f32)y, (f32)z, (f32)w };
    return result;
}

static_function v4
V4i(u32 x, u32 y, u32 z, u32 w)
{
    v4 result = { (f32)x, (f32)y, (f32)z, (f32)w };
    return result;
}


//~

static_function b32
operator==(v4 a, v4 b)
{
    f32_4x a4, b4;
    a4.vec = a;
    b4.vec = b;
    b32 result = (a4 == b4);
    return result;
}


static_function v4
operator*(f32 a, v4 b)
{
    v4 result;
    result.x = a * b.x;
    result.y = a * b.y;
    result.z = a * b.z;
    result.w = a * b.w;
    
    return result;
}

static_function v4
operator*(v4 a, f32 b)
{
    v4 result = b * a;
    return result;
}

static_function v4
operator-(v4 a)
{
    v4 result;
    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;
    result.w = -a.w;
    
    return result;
}

static_function v4
operator+(v4 a, v4 b)
{
    v4 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    result.w = a.w + b.w;
    
    return result;
}

static_function v4
operator-(v4 a, v4 b)
{
    v4 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    result.w = a.w - b.w;
    
    return result;
}

static_function v4
operator*=(v4 &a, f32 b)
{ 
    a = b * a;
    return a;
}

static_function v4
operator+=(v4 &a, v4 b)
{
    a = a + b;
    return a;   
}

static_function v4
operator-=(v4 &a, v4 b)
{
    a = a - b;
    return a;   
}

static_function v4
lerp(v4 a, f32 t, v4 b)
{
    v4 result = (1.0f - t)*a + t*b;
    return result;
}

static_function v4
hadamard(v4 a, v4 b)
{
    v4 result;
    result.x = a.x * b.x;
    result.y = a.y * b.y; 
    result.z = a.z * b.z;
    result.w = a.w * b.w;
    
    return result;
}

static_function f32
inner(v4 a, v4 b)
{
    f32 result = (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
    return result;
}

static_function f32
get_length_sq(v4 a)
{
    f32 result = inner(a, a);
    return result;
}

static_function f32
get_length(v4 a)
{
    f32 result = square_root(get_length_sq(a));
    return result;
}

static_function v4
get_normalized(v4 a)
{
    v4 result = a * (1.f / get_length(a));
    return result;
}


// ================================================================
// @Section_source_math_rect2.h
// ================================================================

//~
static_function f32
get_width(Rect2 rect)
{
    f32 result = rect.max.x - rect.min.x;
    return result;
}

static_function f32
get_height(Rect2 rect)
{
    f32 result = rect.max.y - rect.min.y;
    return result;
}


//~ NOTE(f0): Rect3 -> Rect2
static_function Rect2
to_rect_xy(Rect3 a)
{
    Rect2 result;
    result.min = a.min.xy;
    result.max = a.max.xy;
    return result;
}

// Rect2 constructors

static_function Rect2
rect_min_max(v2 min, v2 max)
{
    Rect2 output;
    output.min = min;
    output.max = max;
    return output;
};

static_function Rect2
rect_min_dim(v2 min, v2 dim)
{
    Rect2 output;
    output.min = min;
    output.max = min + dim;
    return output;
};

static_function Rect2
rect_center_half_dim(v2 center, v2 half_dim)
{
    Rect2 output;
    output.min = center - half_dim;
    output.max = center + half_dim;
    return output;
};

static_function Rect2
rect_center_dim(v2 center, v2 dim)
{
    Rect2 output = rect_center_half_dim(center, 0.5f*dim);
    return output;
};

// Rect2 static_functions

static_function Rect2
add_radius_to(Rect2 rectangle, v2 radius)
{
    Rect2 result;
    result.min = rectangle.min - radius;
    result.max = rectangle.max + radius;
    
    return result;
}

static_function Rect2
get_offset(Rect2 rectangle, v2 offset)
{
    Rect2 result;
    result.min = rectangle.min + offset;
    result.max = rectangle.max + offset;
    
    return result;
}

static_function b32
is_in_rectangle(Rect2 rectangle, v2 test)
{
    b32 result = ((test.x >= rectangle.min.x) && 
                  (test.y >= rectangle.min.y) &&
                  (test.x < rectangle.max.x) &&
                  (test.y < rectangle.max.y));
    
    return result;
} 

static_function v2
get_min_corner(Rect2 rect)
{
    v2 result = rect.min;
    return result;
}

static_function v2
get_max_corner(Rect2 rect)
{
    v2 result = rect.max;
    return result;
}

static_function v2
get_center(Rect2 rect)
{
    v2 result = 0.5f*(rect.min + rect.max);
    return result;
}

static_function v2
get_dim(Rect2 rect)
{
    v2 result = rect.max - rect.min;
    return result;
}

static_function b32
are_intersecting(Rect2 a, Rect2 b)
{
    b32 result = !((b.max.x <= a.min.x) || // x axis
                   (b.min.x >= a.max.x) ||
                   (b.max.y <= a.min.y) || // y axis
                   (b.min.y >= a.max.y));
    
    return result;
}

static_function v2
get_barycentric(Rect2 rect, v2 p)
{
    v2 result;
    result.x = safe_ratio_0((p.x - rect.min.x), (rect.max.x - rect.min.x));
    result.y = safe_ratio_0((p.y - rect.min.y), (rect.max.y - rect.min.y));
    
    return result;
}


// ================= additional ================

static_function Rect2
get_intersection(Rect2 a, Rect2 b)
{
    Rect2 result;
    result.min.x = pick_bigger(a.min.x, b.min.x);
    result.min.y = pick_bigger(a.min.y, b.min.y);
    result.max.x = pick_smaller(a.max.x, b.max.x);
    result.max.y = pick_smaller(a.max.y, b.max.y);
    
    return result;
}

static_function Rect2
get_union(Rect2 a, Rect2 b)
{
    // NOTE(mg): Optimistic approximation of the union as rectangle.
    
    Rect2 result;
    result.min.x = pick_smaller(a.min.x, b.min.x);
    result.min.y = pick_smaller(a.min.y, b.min.y);
    result.max.x = pick_bigger(a.max.x, b.max.x);
    result.max.y = pick_bigger(a.max.y, b.max.y);
    
    return result;
}

static_function f32
get_clamped_rect_area(Rect2 a)
{
    f32 width = a.max.x - a.min.x;
    f32 height = a.max.y - a.min.y;
    f32 result = 0;
    
    if ((width > 0) && (height > 0))
    {
        result = width*height;
    }
    
    return result;
}

static_function b32
has_area(Rect2 a)
{
    b32 result = ((a.min.x < a.max.x) && (a.min.y < a.max.y));
    return result;
}

static_function Rect2
inverted_infinity_rect2()
{
    Rect2 result;
    result.min.x = result.min.y = F32_Max;
    result.max.x = result.max.y = -F32_Max;
    return result;
}


// ================================================================
// @Section_source_math_rect3.h
// ================================================================

// Rect3 constructors
static_function Rect3
rect_min_max(v3 min, v3 max)
{
    Rect3 output;
    output.min = min;
    output.max = max;
    return output;
};

static_function Rect3
rect_min_dim(v3 min, v3 dim)
{
    Rect3 output;
    output.min = min;
    output.max = min + dim;
    return output;
};

static_function Rect3
rect_center_half_dim(v3 center, v3 half_dim)
{
    Rect3 output;
    output.min = center - half_dim;
    output.max = center + half_dim;
    return output;
};

static_function Rect3
rect_center_dim(v3 center, v3 dim)
{
    Rect3 output = rect_center_half_dim(center, 0.5f*dim);
    return output;
};

// Rect3 static_functions

static_function Rect3
add_radius_to(Rect3 rectangle, v3 radius)
{
    Rect3 result;
    result.min = rectangle.min - radius;
    result.max = rectangle.max + radius;
    
    return result;
}

static_function Rect3
get_offset(Rect3 rectangle, v3 offset)
{
    Rect3 result;
    result.min = rectangle.min + offset;
    result.max = rectangle.max + offset;
    
    return result;
}

static_function b32
is_in_rectangle(Rect3 rectangle, v3 test)
{
    b32 result = ((test.x >= rectangle.min.x) && 
                  (test.y >= rectangle.min.y) &&
                  (test.z >= rectangle.min.z) &&
                  (test.x < rectangle.max.x) &&
                  (test.y < rectangle.max.y) &&
                  (test.z < rectangle.max.z));
    
    return result;
} 


static_function v3
get_min_corner(Rect3 rect)
{
    v3 result = rect.min;
    return result;
}

static_function v3
get_max_corner(Rect3 rect)
{
    v3 result = rect.max;
    return result;
}

static_function v3
get_center(Rect3 rect)
{
    v3 result = 0.5f*(rect.min + rect.max);
    return result;
}

static_function v3
get_dim(Rect3 rect)
{
    v3 result = rect.max - rect.min;
    return result;
}


static_function b32
are_intersecting(Rect3 a, Rect3 b)
{
    b32 result = !((b.max.x <= a.min.x) || // x axis
                   (b.min.x >= a.max.x) ||
                   (b.max.y <= a.min.y) || // y axis
                   (b.min.y >= a.max.y) ||
                   (b.max.z <= a.min.z) || // z axis
                   (b.min.z >= a.max.z));
    
    return result;
}

static_function v3
get_barycentric(Rect3 rect, v3 p)
{
    v3 result;
    result.x = safe_ratio_0((p.x - rect.min.x), (rect.max.x - rect.min.x));
    result.y = safe_ratio_0((p.y - rect.min.y), (rect.max.y - rect.min.y));
    result.z = safe_ratio_0((p.z - rect.min.z), (rect.max.z - rect.min.z));
    
    return result;
}


// ================================================================
// @Section_source_math_rect2s.h
// ================================================================

static_function Rect2s
get_intersection(Rect2s a, Rect2s b)
{
    Rect2s result;
    result.min = {
        pick_bigger(a.min.x, b.min.x),
        pick_bigger(a.min.y, b.min.y)
    };
    result.max = {
        pick_smaller(a.max.x, b.max.x),
        pick_smaller(a.max.y, b.max.y)
    };
    return result;
}

static_function Rect2s
get_union(Rect2s a, Rect2s b)
{
    // NOTE: "Optimistic" approximation of the union as rectangle.
    Rect2s result;
    result.min = {
        pick_smaller(a.min.x, b.min.x),
        pick_smaller(a.min.y, b.min.y)
    };
    result.max = {
        pick_bigger(a.max.x, b.max.x),
        pick_bigger(a.max.y, b.max.y)
    };
    return result;
}

static_function s32
get_clamped_rect_area(Rect2s a)
{
    s32 width = a.max.x - a.min.x;
    s32 height = a.max.y - a.min.y;
    s32 result = 0;
    
    if ((width > 0) && (height > 0))
    {
        result = width*height;
    }
    
    return result;
}

static_function b32
has_area(Rect2s a)
{
    b32 result = ((a.min.x < a.max.x) && (a.min.y < a.max.y));
    return result;
}

static_function Rect2s
inverted_infinity_rect2s()
{
    Rect2s result;
    result.min.x = result.min.y = S32_Max;
    result.max.x = result.max.y = S32_Min;
    return result;
}




// ================================================================
// @Section_source_rect_ui.h
// ================================================================
static_function Rect2
cut_left(Rect2 *r, f32 a)
{
    f32 min_x = r->min.x;
    r->min.x = pick_smaller(r->max.x, r->min.x + a);
    Rect2 result = {{min_x, r->min.y}, {r->min.x, r->max.y}};
    return result;
}

static_function Rect2
cut_right(Rect2 *r, f32 a)
{
    f32 max_x = r->max.x;
    r->max.x = pick_bigger(r->min.x, r->max.x - a);
    Rect2 result = {{r->max.x, r->min.y}, {max_x, r->max.y}};
    return result;
}

static_function Rect2
cut_top(Rect2 *r, f32 a)
{
    f32 min_y = r->min.y;
    r->min.y = pick_smaller(r->max.y, r->min.y + a);
    Rect2 result = {{r->min.x, min_y}, {r->max.x, r->min.y}};
    return result;
}

static_function Rect2
cut_bottom(Rect2 *r, f32 a)
{
    f32 max_y = r->max.y;
    r->max.y = pick_bigger(r->min.y, r->max.y - a);
    Rect2 result = {{r->min.x, r->max.y}, {r->max.x, max_y}};
    return result;
}


//-
static_function Rect2
get_left(Rect2 r, f32 a)
{
    f32 min_x = r.min.x;
    f32 max_x = pick_smaller(r.max.x, r.min.x + a);
    Rect2 result = {{min_x, r.min.y}, {max_x, r.max.y}};
    return result;
}

static_function Rect2
get_right(Rect2 r, f32 a)
{
    f32 max_x = r.max.x;
    f32 min_x = pick_bigger(r.min.x, r.max.x - a);
    Rect2 result = {{min_x, r.min.y}, {max_x, r.max.y}};
    return result;
}

static_function Rect2
get_top(Rect2 r, f32 a)
{
    f32 min_y = r.min.y;
    f32 max_y = pick_smaller(r.max.y, r.min.y + a);
    Rect2 result = {{r.min.x, min_y}, {r.max.x, max_y}};
    return result;
}

static_function Rect2
get_bottom(Rect2 r, f32 a)
{
    f32 max_y = r.max.y;
    f32 min_y = pick_bigger(r.min.y, r.max.y - a);
    Rect2 result = {{r.min.x, min_y}, {r.max.x, max_y}};
    return result;
}



// ================================================================
// @Section_source_parse_string.h
// ================================================================

struct S64_Result
{
    s64 value;
    b32 success;
};

static_function S64_Result
parse_s64_from_string(String text)
{
    S64_Result result = {};
    
    text = str_trim_white(text);
    if (text.size > 0)
    {
        b32 has_minus = false;
        
        for (u64 i = 0;
             ;
             ++i)
        {
            if (i >= text.size) {
                result.success = true;
                break;
            }
            
            u8 u = text.str[i];
            
            if (is_number(u))
            {
                result.value *= 10;
                result.value += (u - '0');
            }
            else if (u == '-' && (i == 0))
            {
                has_minus = true;
            }
            else
            {
                break;
            }
        }
        
        if (has_minus)
        {
            result.value *= -1;
        }
    }
    
    return result;
}






struct B32_Result
{
    b32 value;
    b32 success;
};

static_function B32_Result
parse_b32_from_string(String text)
{
    B32_Result result = {};
    if (equals(text, "true"_f0))
    {
        result.value = true;
        result.success = true;
    }
    else if (equals(text, "false"_f0))
    {
        result.value = false;
        result.success = true;
    }
    return result;
}








static_function b32
is_hex(u32 codepoint)
{
    b32 result = ((codepoint >= '0' && codepoint <= '9') ||
                  (codepoint >= 'A' && codepoint <= 'F') ||
                  (codepoint >= 'a' && codepoint <= 'f'));
    return result;
}

static_function s32
hex_codepoint_to_number(u32 hex)
{
    s32 result = 0;
    
    if (hex >= '0' && hex <= '9')
    {
        result = hex - '0';
    }
    else if (hex >= 'A' && hex <= 'F')
    {
        result = (hex - 'A') + 10;
    }
    else if (hex >= 'a' && hex <= 'f')
    {
        result = (hex - 'a') + 10;
    }
    else
    {
        assert(0);
    }
    
    return result;
}

static_function b32
str_is_hexs(String string)
{
    b32 result = (string.size != 0);
    for_u64(i, string.size)
    {
        if (!is_hex(string.str[i]))
        {
            result = false;
            break;
        }
    }
    return result;
}


struct Color_Result
{
    v4 value;
    b32 success;
};

static_function Color_Result
parse_color_from_string(String t)
{
    Color_Result result = {};
    
    if (t.size)
    {
        if (t.str[0] == '#')
        {
            t = str_skip(t, 1);
            
            if (str_is_hexs(t))
            {
                if (t.size == 3)
                {
                    s32 r15 = hex_codepoint_to_number(t.str[0]);
                    s32 g15 = hex_codepoint_to_number(t.str[1]);
                    s32 b15 = hex_codepoint_to_number(t.str[2]);
                    result.value = V4(r15/15.f, g15/15.f, b15/15.f, 1.f);
                    result.success = true;
                }
                else if (t.size == 4)
                {
                    s32 r15 = hex_codepoint_to_number(t.str[0]);
                    s32 g15 = hex_codepoint_to_number(t.str[1]);
                    s32 b15 = hex_codepoint_to_number(t.str[2]);
                    s32 a15 = hex_codepoint_to_number(t.str[3]);
                    result.value = V4(r15/15.f, g15/15.f, b15/15.f, a15/15.f);
                    result.success = true;
                }
                else if (t.size == 6)
                {
                    s32 r255 = 16*hex_codepoint_to_number(t.str[0]) + hex_codepoint_to_number(t.str[1]);
                    s32 g255 = 16*hex_codepoint_to_number(t.str[2]) + hex_codepoint_to_number(t.str[3]);
                    s32 b255 = 16*hex_codepoint_to_number(t.str[4]) + hex_codepoint_to_number(t.str[5]);
                    result.value = V4(r255/255.f, g255/255.f, b255/255.f, 1.f);
                    result.success = true;
                }
                else if (t.size == 8)
                {
                    s32 r255 = 16*hex_codepoint_to_number(t.str[0]) + hex_codepoint_to_number(t.str[1]);
                    s32 g255 = 16*hex_codepoint_to_number(t.str[2]) + hex_codepoint_to_number(t.str[3]);
                    s32 b255 = 16*hex_codepoint_to_number(t.str[4]) + hex_codepoint_to_number(t.str[5]);
                    s32 a255 = 16*hex_codepoint_to_number(t.str[6]) + hex_codepoint_to_number(t.str[7]);
                    result.value = V4(r255/255.f, g255/255.f, b255/255.f, a255/255.f);
                    result.success = true;
                }
            }
        }
    }
    
    return result;
}


// ================================================================
// @Section_source_colors.h
// ================================================================

static_function v4
get_red(f32 secondary=0.f, f32 primary=1.f)
{
    v4 result = {primary, secondary, secondary, 1};
    return result;
}

static_function v4
get_green(f32 secondary=0.f, f32 primary=1.f)
{
    v4 result = {secondary, primary, secondary, 1};
    return result;
}

static_function v4
get_blue(f32 secondary=0.f, f32 primary=1.f)
{
    v4 result = {secondary, secondary, primary, 1};
    return result;
}

static_function v4
get_yellow(f32 secondary=0.f, f32 primary=1.f)
{
    v4 result = {primary, primary, secondary, 1};
    return result;
}

static_function v4
get_cyan(f32 secondary=0.f, f32 primary=1.f)
{
    v4 result = {secondary, primary, primary, 1};
    return result;
}

static_function v4
get_magenta(f32 secondary=0.f, f32 primary=1.f)
{
    v4 result = {primary, secondary, primary, 1};
    return result;
}

static_function v4
get_gray(f32 shade=0.5f)
{
    v4 result = {shade, shade, shade, 1};
    return result;
}


static_function v4
get_black(f32 shade=0.0f)
{
    v4 result = {shade, shade, shade, 1};
    return result;
}

static_function v4
get_white(f32 shade=1.0f)
{
    v4 result = {shade, shade, shade, 1};
    return result;
}





struct Color_Hsv
{
    f32 h;
    f32 s;
    f32 v;
};

static_function v4
rgb_from_hsv(Color_Hsv in)
{
    v4 out = {0,0,0,1};
    
    in.h *= 6.f;
    s32 h_int = (s32)in.h;
    
    f32 h_reminder = in.h - (f32)h_int;
    
    f32 p = in.v * (1.f - in.s);
    f32 q = in.v * (1.f - in.s * h_reminder);
    f32 t = in.v * (1.f - in.s * (1.f - h_reminder));
    
    switch (h_int)
    {
        case 0: out.rgb = {in.v,  t,     p}; break;
        case 1: out.rgb = {q,     in.v,  p}; break;
        case 2: out.rgb = {p,     in.v,  t}; break;
        case 3: out.rgb = {p,     q,     in.v}; break;
        case 4: out.rgb = {t,     p,     in.v}; break;
        case 5: out.rgb = {in.v,  p,     q}; break;
    }
    
    return out;
}






//~
struct Stf0_Global_State
{
    s64 query_perfomance_frequency;
    f32 inv_query_perfomance_frequency;
};
static_global Stf0_Global_State stf0_global_state;

static_function void
stf0_initialize()
{
#if Def_Windows
    // timing
    LARGE_INTEGER large_perfomance_freq;
    b32 return_code_test = QueryPerformanceFrequency(&large_perfomance_freq);
    assert(return_code_test);
    assert(large_perfomance_freq.QuadPart);
    stf0_global_state.query_perfomance_frequency = large_perfomance_freq.QuadPart;
    stf0_global_state.inv_query_perfomance_frequency = 1.f / (f32)stf0_global_state.query_perfomance_frequency;
    
#elif Def_Linux
    
#endif
};







//~
static_function void
os_sleep_ms(s32 sleep_ms)
{
    if (sleep_ms >= 0)
    {
#if Def_Windows
        Sleep(sleep_ms);
#else
        usleep(sleep_ms*1000);
#endif
    }
}

static_function void
os_exit_process(s32 return_code)
{
#if Def_Windows
    ExitProcess(return_code);
#elif Def_Linux
    exit(return_code);
#endif
}























//~


#if Def_Slow && Def_More_Breakpoints

static_function void
debug_inspect_error_if_true(b32 condition)
{
    if (condition)
    {
#if Def_Windows
        DWORD error_code = GetLastError();
#elif Def_Linux
        int error_code = errno;
#endif
        debug_break();
        error_code += 10000000;
        error_code -= 10000000;
    }
}

#else
#define debug_inspect_error_if_true(...)
#endif




//~
static_function b32
filesystem_is_case_insensitive()
{
#if Def_Windows
    return true;
#elif Def_Linux
    return false;
#endif
}

static_function b32
filesystem_str_equals(String a, String b)
{
    return equals(a, b, filesystem_is_case_insensitive());
}



// ================================================================
// @Section_source_arena.h
// ================================================================

//~
struct Arena
{
    void *base;
    u64 position;
    u64 capacity;
    //
    u64 reserved_capacity;
    //
    u32 page_size;
    s32 stack_count; // NOTE(f0): safety/testing variable
};



//~
struct Arena_Scope
{
    Arena *copy;
    u64 position;
};

static_function Arena_Scope
create_arena_scope(Arena *arena)
{
    Arena_Scope result = {};
    result.copy = arena;
    result.position = arena->position;
    return result;
}

static_function void
reset_arena_position(Arena *arena, u64 position=0)
{
    arena->position = position;
}

static_function void
pop_arena_scope(Arena_Scope *scope)
{
    assert(scope);
    assert(scope->copy);
    
    scope->copy->position = scope->position;
}

struct Automatic_Arena_Scope
{
    Arena_Scope scope;
    s32 stack_count;
    
    Automatic_Arena_Scope(Arena* arena)
    {
        scope = create_arena_scope(arena);
        stack_count = arena->stack_count++;
    }
    
    ~Automatic_Arena_Scope()
    {
        pop_arena_scope(&scope);
        scope.copy->stack_count -= 1;
        assert(scope.copy->stack_count == stack_count);
    }
};

#define arena_scope(Arena) Automatic_Arena_Scope glue(automatic_arena_scope_, This_Line_S32)(Arena)






//~ Push memory
#define push_array(ArenaPtr, Type, Count)\
(Type *)push_bytes_((ArenaPtr), (sizeof(Type)*(Count)), alignof(Type))

#define push_array_align(ArenaPtr, Type, Count, Align)\
(Type *)push_bytes_((ArenaPtr), (sizeof(Type)*(Count)), Align)

#define push_array_clear(ArenaPtr, Type, Count)\
(Type *)push_bytes_clear_((ArenaPtr), (sizeof(Type)*(Count)), alignof(Type))

#define push_array_clear_align(ArenaPtr, Type, Count, Align)\
(Type *)push_bytes_clear_((ArenaPtr), (sizeof(Type)*(Count)), Align)

////////////////////////////////
#define push_struct(ArenaPtr, Type)       push_array(ArenaPtr, Type, 1)
#define push_struct_align(ArenaPtr, Type) push_array_align(ArenaPtr, Type, 1)
#define push_struct_clear(ArenaPtr, Type) push_array_clear(ArenaPtr, Type, 1)

////////////////////////////////
#define push_array_copy(ArenaPtr, Source, Type, Count)\
(Type *)push_bytes_and_copy_((ArenaPtr), (Source), sizeof(Type)*(Count), alignof(Type))



//~ Memory on stack - Alloca (mostly unused?)
#define push_stack_array(Type, Count) (Type *)alloca(sizeof(Type)*Count)






//~
static_function u64
get_aligment_offset(Arena *arena, u64 aligment)
{
    u64 aligment_offset = 0;
    u64 result_pointer = (u64)arena->base + arena->position;
    u64 aligment_mask = aligment - 1;
    
    if (result_pointer & aligment_mask)
    {
        aligment_offset = aligment - (result_pointer & aligment_mask);
    }
    
    return aligment_offset;
}


static_function u64
round_up_to_page_size(Arena *arena, u64 value)
{
    u64 result = align_bin_to(value, (u64)arena->page_size);
    return result;
}

static_function void
commit_virtual_memory_(Arena *arena, u64 position_required_to_fit)
{
    assert(position_required_to_fit <= arena->reserved_capacity);
    //u64 target_capacity = 2 * position_required_to_fit;
    u64 target_capacity = position_required_to_fit + (arena->capacity >> 2) + 4096;
    target_capacity = pick_smaller(target_capacity, arena->reserved_capacity);
    
    u64 commit_end_position = round_up_to_page_size(arena, target_capacity);
    u64 commit_size = commit_end_position - arena->capacity;
    u8 *commit_address = (u8 *)arena->base + arena->capacity;
    
    //-
#if Def_Windows
    void *commit_result = VirtualAlloc(commit_address, commit_size, MEM_COMMIT, PAGE_READWRITE);
    assert(commit_result);
    
#elif Def_Linux
    int protect_res = mprotect(commit_address, commit_size, PROT_READ|PROT_WRITE);
    assert(protect_res != -1);
#endif
    
    //-
    arena->capacity = commit_end_position;
}

static_function void *
push_bytes_virtual_commit_(Arena *arena, u64 alloc_size, u64 alignment)
{
    assert(arena->base);
    assert(alloc_size > 0);
    u64 alignment_offset = get_aligment_offset(arena, alignment);
    u64 future_position = arena->position + alignment_offset + alloc_size;

    if (future_position > arena->capacity)
    {
        commit_virtual_memory_(arena, future_position);
    }

    void *result = (u8 *)arena->base + arena->position + alignment_offset;
    arena->position = future_position;
    
    return result;
}

static_function void *
push_bytes_virtual_commit_unaligned_(Arena *arena, u64 alloc_size)
{
    assert(arena->base);
    u64 future_position = arena->position + alloc_size;

    if (future_position > arena->capacity)
    {
        commit_virtual_memory_(arena, future_position);
    }
    
    void *result = (u8 *)arena->base + arena->position;
    arena->position = future_position;
    return result;
}






//~
static_function Arena
create_virtual_arena(u64 target_reserved_capacity = gigabytes(16))
{
    Arena arena = {};
    
#if Def_Windows
    SYSTEM_INFO system_info;
    GetSystemInfo(&system_info);
    
    arena.page_size = system_info.dwPageSize;
    arena.reserved_capacity = round_up_to_page_size(&arena, target_reserved_capacity);
    arena.base = VirtualAlloc(nullptr, arena.reserved_capacity, MEM_RESERVE, PAGE_READWRITE);
    assert(arena.base);

#elif Def_Linux
    arena.page_size = getpagesize();
    arena.reserved_capacity = round_up_to_page_size(&arena, target_reserved_capacity);
    arena.base = mmap(nullptr, arena.reserved_capacity,
                      PROT_NONE,
                      MAP_PRIVATE|MAP_ANONYMOUS,
                      -1, 0);
    
    if (!arena.base || arena.base == (void*)-1)
    {
        int error = errno;
        assert(0);
    }
#endif
    
    return arena;
}



static_function void
free_virtual_arena(Arena *arena)
{
#if Def_Windows
    b32 result = VirtualFree(arena->base, 0, MEM_RELEASE);
    assert(result);
    
#elif Def_Linux
    int free_res = munmap(arena->base, arena->reserved_capacity);
    assert(free_res != -1);
#endif
    
    *arena = {};
}





static_function void *
push_bytes_(Arena *arena, u64 alloc_size, u64 alignment)
{
    void *result = push_bytes_virtual_commit_(arena, alloc_size, alignment);
    return result;
}

static_function void *
push_bytes_clear_(Arena *arena, u64 alloc_size, u64 alignment)
{
    void *result = push_bytes_virtual_commit_(arena, alloc_size, alignment);
    clear_bytes(result, alloc_size);
    return result;
}

static_function void *
push_bytes_and_copy_(Arena *arena, void *source, u64 alloc_size, u64 alignment)
{
    void *mem = push_bytes_(arena, alloc_size, alignment);
    copy_bytes(mem, source, alloc_size);
    return mem;
}












//~
template <typename T>
struct Virtual_Array
{
    union {
        Arena arena;
        T *array;
    };
    u64 count;
    
    inline T *at(u64 index)
    {
        assert(index < count);
        T *result = array + index;
        return result;
    }
    
    inline T *grow(u64 grow_count = 1)
    {
        assert(grow_count > 0);
        T *result = array + count;
        push_bytes_virtual_commit_unaligned_(&arena, sizeof(T)*grow_count);
        count += grow_count;
        return result;
    }
    
    inline void reset(u64 new_count=0)
    {
        assert(new_count <= count);
        count = new_count;
        arena.position = sizeof(T)*new_count;
    }
};


template <typename T>
static_function Virtual_Array<T>
create_virtual_array(u64 initial_count = 0,
                     u64 target_reserved_capacity = gigabytes(8))
{
    Virtual_Array<T> result = {};
    result.arena = create_virtual_arena(target_reserved_capacity);
    if (initial_count > 0)
    {
        result.grow(initial_count);
    }
    return result;
}


template <typename T>
static_function void
free_virtual_array(Virtual_Array<T> *array)
{
    free_virtual_arena(&array->arena);
    *array = {};
}











//~
template <typename T>
struct Virtual_Array_Scope
{
    Arena_Scope arena_scope;
    Virtual_Array<T> *copy;
    u64 element_count;
};

template <typename T>
static_function Virtual_Array_Scope<T>
create_virtual_array_scope(Virtual_Array<T> *array)
{
    Virtual_Array_Scope<T> result = {};
    result.arena_scope = create_arena_scope(&array->arena);
    result.copy = array;
    result.element_count = array->count;
    return result;
}

template <typename T>
static_function void
pop_virtual_array_scope(Virtual_Array_Scope<T> *scope)
{
    assert(scope);
    assert(scope->copy);
    
    pop_arena_scope(&scope->arena_scope);
    scope->copy->count = scope->element_count;
}












//~
template <typename T>
struct Linked_List_Node
{
    Linked_List_Node<T> *next;
    T item;
};

template <typename T>
struct Linked_List
{
    Linked_List_Node<T> *first;
    Linked_List_Node<T> *last;
    u64 count;
    
    inline Linked_List_Node<T> *push_get_node(Arena *arena)
    {
        if (count)
        {
            assert(first);
            assert(last);
            last->next = push_array(arena, Linked_List_Node<T>, 1);
            last = last->next;
        }
        else
        {
            first = push_array(arena, Linked_List_Node<T>, 1);
            last = first;
        }
        
        ++count;
        last->next = nullptr;
        return last;
    }
    
    inline T *push(Arena *arena)
    {
        auto result = push_get_node(arena);
        return &result->item;
    }
    
    inline T *at(u64 index)
    {
        assert(index < count);
        
        T *result = nullptr;
        
        for (Linked_List_Node<T> *node = first;
             node;
             node = node->next)
        {
            result = &node->item;
            
            if (index == 0) { break; }
            index -= 1;
        }
        
        return result;
    }
};







//~
template <class T>
struct Checked_Array
{
    T *array;
    u64 count;
    
    
    
    inline void initialize(Arena *a, u64 initial_count = 0)
    {
        if (initial_count)
        {
            array = push_array(a, T, initial_count);
            count = initial_count;
        }
        else
        {
            u64 aligment_offset = get_aligment_offset(a, alignof(T));
            if (aligment_offset) {
                push_bytes_virtual_commit_unaligned_(a, aligment_offset);
            }
            
            array = (T *)((u8 *)a->base + a->position);
            count = 0;
        }
    }
    
    
    
    inline T *at(u64 index)
    {
        assert(index < count);
        return array + index;
    }
    
    inline void safe_increase_count_(T *first_new_element_check, u64 increase_by=1)
    {
        // NOTE(f0): checks if array is continuous in memory
        assert(array);
        assert(array + count == first_new_element_check);
        count += increase_by;
    }
    
    inline T *expand(Arena *a, u64 increase_by=1)
    {
        T *result = push_array(a, T, increase_by);
        safe_increase_count_(result, increase_by);
        return result;
    }
};

template <class T>
static_function Checked_Array<T>
create_checked_array(Arena *a, u64 initial_count = 0)
{
    Checked_Array<T> result = {};
    result.initialize(a, initial_count);
    return result;
}
    

// ================================================================
// @Section_source_helpers_that_require_alloc.h
// ================================================================

//~
struct Directory
{
    String *names;
    u64 name_count; // TODO(f0): rename to count?
    // TODO(f0): or segment count?
    // TODO(f0): or nah?
    
    inline String last()
    {
        String result = {};
        if (name_count > 0)
        {
            result = names[name_count-1];
        }
        return result;
    }
};

typedef Linked_List<Directory> Directory_List;
typedef Linked_List<String> String_List;





static_function String
push_string(Arena *arena, u64 size)
{
    String result = {};
    if (size > 0) {
        result = {push_array(arena, u8, size), size};
    }
    return result;
}


static_function String
push_copy(Arena *arena, String source)
{
    String result = {};
    if (source.size > 0) {
        result = {push_array(arena, u8, source.size), source.size};
        copy_array(result.str, source.str, u8, source.size);
    }
    return result;
}


static_function char *
push_copy(Arena *arena, const char *source, s64 overwrite_len = -1)
{
    u64 len = (u64)((overwrite_len > 0) ? overwrite_len : zero_terminated_length(source));
    char *result = push_array(arena, char, len + 1);
    copy_array(result, (void*)source, char, len);
    result[len] = 0;
    return result;
}


static_function char *
cstr_from_get_string(Arena *arena, String string)
{
    char *cstr = push_array(arena, char, string.size+1);
    copy_array(cstr, string.str, char, string.size);
    cstr[string.size] = 0;
    return cstr;
}



static_function Directory
push_copy(Arena *arena, Directory directory)
{
    Directory result = {};
    
    if (directory.name_count > 0)
    {
        result.names = push_array(arena, String, directory.name_count);
        result.name_count = directory.name_count;
        
        for_u64(name_index, directory.name_count)
        {
            result.names[name_index] = push_copy(arena, directory.names[name_index]);
        }
    }
    
    return result;
}





//~
static_function b32
equals(Directory a, Directory b)
{
    b32 result = false;
    
    if (a.name_count == b.name_count)
    {
        result = true;
        for_u64(name_index, a.name_count)
        {
            String name_a = a.names[name_index];
            String name_b = b.names[name_index];
            b32 names_equal = filesystem_str_equals(name_a, name_b);
            
            if (!names_equal) {
                result = false;
                break;
            }
        }
    }
    
    return result;
}

static_function Directory
directory_from_string(Arena *arena, String source)
{
    Directory result = {};
    result.name_count = str_count_of_from_table(source, lit2str("/\\"));
    if (source.size > 0 && !is_slash(source.str[source.size-1]))
    {
        result.name_count += 1;
    }
    
    if (result.name_count > 0)
    {
        result.names = push_array(arena, String, result.name_count);
        
        u64 current_p = 0;
        for_u64(name_index, result.name_count)
        {
            u64 start_p = current_p;
            String element = str_skip(source, start_p);
            
            for (;
                 current_p < source.size;
                 ++current_p)
            {
                if (is_slash(source.str[current_p]))
                {
                    element.size = (current_p++ - start_p);
                    break;
                }
            }
            
            result.names[name_index] = element;
        }
    }
    
    return result;
}


static_function Directory
directory_append(Arena *arena, Directory parent_directory, String sub_directory_name)
{
    Directory result = {};
    result.name_count = parent_directory.name_count + 1;
    result.names = push_array(arena, String, result.name_count);
    for_u64(name_index, parent_directory.name_count)
    {
        result.names[name_index] = parent_directory.names[name_index];
    }
    result.names[result.name_count-1] = sub_directory_name;
    return result;
}



static_function u64
get_directory_names_length_sum(Directory directory)
{
    u64 result = 0;
    for_u64(name_index, directory.name_count)
    {
        result += directory.names[name_index].size;
    }
    return result;
}

static_function u64
get_directory_string_length(Directory directory)
{
    u64 result = get_directory_names_length_sum(directory);
    result += directory.name_count;
    return result;
}

















//~
static_function String_Dll_Node *
str_split_on_character_table(Arena *a, String haystack, String character_table_needle, b32 skip_empty_nodes)
{
    // NOTE: This function allocates Nodes but doesn't copy strings;
    // .string field contains substring from String haystack
    
    // NOTE: When skip_empty_nodes = false:
    // this_func(a, "/home/"_f0, "/"_f0) -> {  {sentinel}, {.size=0}, {.size=4}, {.size=0}  }
    
    String_Dll_Node *result = push_struct_clear(a, String_Dll_Node);
    dll_initialize_sentinel(result);
    
    u64 to_split_size = 0;
    while (haystack.size > to_split_size)
    {
        b32 has_match = false;
        for_u64(needle_index, character_table_needle.size)
        {
            if (haystack.str[to_split_size] == character_table_needle.str[needle_index])
            {
                has_match = true;
                break;
            }
        }
        
        
        if (has_match)
        {
            if (!skip_empty_nodes || to_split_size)
            {
                String_Dll_Node *node = push_struct(a, String_Dll_Node);
                node->string = get_string(haystack.str, to_split_size);
                dll_insert_before(result, node);
            }
            
            haystack = str_skip(haystack, to_split_size + 1);
            to_split_size = 0;
        }
        else
        {
            to_split_size += 1;
        }
    }
    
    if (!skip_empty_nodes || haystack.size)
    {
        String_Dll_Node *node = push_struct(a, String_Dll_Node);
        node->string = haystack;
        dll_insert_before(result, node);
    }
    
    return result;
}





//~
static_function String
string_trim_repeated_white_and_replace_white_with_whitespace(Arena *a, String text)
{
    String result = {};
    
    text = str_trim_white(text);
    if (text.size)
    {
        result = push_string(a, text.size);
        u64 size = 0;
        
        b32 prev_white = false;
        for_u64(i, text.size)
        {
            u8 c = text.str[i];
            if (is_white(c))
            {
                if (prev_white) {
                    continue;
                }
                
                prev_white = true;
                c = ' ';
            }
            else
            {
                prev_white = false;
            }
            
            result.str[size] = c;
            size += 1;
        }
        
        result.size = size;
    }
    
    return result;
}


// ================================================================
// @Section_source_to_string.h
// ================================================================


static_function char *
to_cstr(Arena *a, String string)
{
    char *result = push_array(a, char, string.size+1);
    copy_bytes(result, string.str, string.size);
    result[string.size] = 0;
    return result;
}





//- Directory
static_function u64
fill_buffer_from_directory(void *output, u64 output_size,
                           Directory directory,
                           b32 use_windows_slash = Def_Windows)
{
    u8 slash = (u8)(use_windows_slash ? '\\' : '/');
    u64 out_index = 0;
    b32 full_fill = false;
    u8 *out = (u8*)output;
    
    for_u64(name_index, directory.name_count)
    {
        String *dir_name = directory.names + name_index;
        for_u64(char_index, dir_name->size)
        {
            if (out_index >= output_size) {
                goto early_exit_label;
            }
            
            out[out_index++] = dir_name->str[char_index];
        }
        
        
        if (out_index >= output_size) {
            goto early_exit_label;
        }
        
        out[out_index++] = slash;
    }
    
    full_fill = true;
    early_exit_label:
    
    assert(full_fill);
    return out_index;
}

static_function String
to_string(Arena *arena, Directory directory,
          b32 use_windows_slash = Def_Windows)
{
    u64 pre_len = get_directory_string_length(directory);
    String result = push_string(arena, pre_len);
    
    u64 post_len = fill_buffer_from_directory(result.str, result.size, directory, use_windows_slash);
    assert(pre_len == post_len);
    
    return result;
}

static_function char *
to_cstr(Arena *arena, Directory directory,
        b32 use_windows_slash = (Native_Slash_Char == '\\'))
{
    u64 pre_len = get_directory_string_length(directory);
    char *result = push_array(arena, char, pre_len + 1);
    
    u64 post_len = fill_buffer_from_directory(result, pre_len, directory, use_windows_slash);
    assert(pre_len == post_len);
    
    result[pre_len] = 0;
    return result;
}








// ====================== @Alloc_Stringf ======================
static_function String
stringf(Arena *arena, char *format, ...)
{
    va_list args1;
    va_list args2;
    va_start(args1, format);
    va_copy(args2, args1);
    
    s32 len = vsnprintf(0, 0, format, args1);
    va_end(args1);
    assert(len >= 0);
    
    len += 1;
    String result = push_string(arena, (u64)len);
    vsnprintf((char *)result.str, result.size, format, args2);
    --result.size;
    va_end(args2);
    
    return result;
}


static_function char *
cstrf(Arena *arena, char *format, ...)
{
    va_list args1;
    va_list args2;
    va_start(args1, format);
    va_copy(args2, args1);
    
    s32 len = vsnprintf(0, 0, format, args1);
    va_end(args1);
    assert(len >= 0);
    
    len += 1;
    char *result = push_array(arena, char, (u64)len);
    vsnprintf(result, len, format, args2);
    result[len-1] = 0;
    va_end(args2);
    
    return result;
}







// =================== @String_Helpers ==================
static_function String
str_concatenate(Arena *arena, String first, String second)
{
    String result = push_string(arena, first.size + second.size);
    copy_bytes(result.str, first.str, first.size);
    copy_bytes(result.str + first.size, second.str, second.size);
    return result;
}

static_function String
str_concatenate(Arena *arena, String first, String second, String third)
{
    String result = push_string(arena, first.size + second.size + third.size);
    copy_bytes(result.str, first.str, first.size);
    copy_bytes(result.str + first.size, second.str, second.size);
    copy_bytes(result.str + first.size + second.size, third.str, third.size);
    return result;
}

static_function String
str_concatenate_array(Arena *arena, String *strings, u64 string_count)
{
    u64 sum_size = 0;
    for_u64(i, string_count)
    {
        sum_size += strings[i].size;
    }
    
    String result = push_string(arena, sum_size);
    auto str = result.str;
    for_u64(i, string_count)
    {
        copy_bytes(str, strings[i].str, strings[i].size);
        str += strings[i].size;
    }
    return result;
}








// ======================== @to_string ========================
static_function String
to_string(Arena *a, u32 value)
{
    String result = stringf(a, "%u", value);
    return result;
}

static_function String
to_string(Arena *a, u64 value)
{
    String result = stringf(a, "%llu", value);
    return result;
}

//-
static_function String
to_string(Arena *a, s32 value)
{
    String result = stringf(a, "%d", value);
    return result;
}

static_function String
to_string(Arena *a, s64 value)
{
    String result = stringf(a, "%lld", value);
    return result;
}

//-
static_function String
to_string(Arena *a, f32 value)
{
    String result = stringf(a, "%.2f", value);
    return result;
}


static_function String
to_string(Arena *a, v2 value)
{
    String result = stringf(a, "{%.2f, %.2f}",
                            value.x, value.y);
    return result;
}

static_function String
to_string(Arena *a, v3 value)
{
    String result = stringf(a, "{%.2f, %.2f, %.2f}",
                            value.x, value.y, value.z);
    return result;
}

static_function String
to_string(Arena *a, v4 value)
{
    String result = stringf(a, "{%.2f, %.2f, %.2f, %.2f}",
                            value.x, value.y, value.z, value.w);
    return result;
}


static_function String
to_string(Arena *a, Rect2 value)
{
    String result = stringf(a, "{min: {%.2f, %.2f}, max: {%.2f, %.2f}}",
                            value.min.x, value.min.y,
                            value.max.x, value.max.y);
    return result;
}

static_function String
to_string(Arena *a, m4x4 value)
{
    String result = stringf(a, "{%.2f, %.2f, %.2f, %.2f,""\n"
                            "%.2f, %.2f, %.2f, %.2f,""\n"
                            "%.2f, %.2f, %.2f, %.2f,""\n"
                            "%.2f, %.2f, %.2f, %.2f}",
                            value.e[0][0], value.e[0][1], value.e[0][2], value.e[0][3],
                            value.e[1][0], value.e[1][1], value.e[1][2], value.e[1][3],
                            value.e[2][0], value.e[2][1], value.e[2][2], value.e[2][3],
                            value.e[3][0], value.e[3][1], value.e[3][2], value.e[3][3]);
    return result;
}

//-
static_function String
to_string(Arena *a, String_List list, String separator={})
{
    u64 len = 0;
    u64 count = 0;
    for_linked_list(node, list)
    {
        count += 1;
        len += node->item.size;
    }
    
    if (count > 1)
    {
        len += (count - 1)*separator.size;
    }
    
    String result = push_string(a, len);
    u64 write_index = 0;
    
    for_linked_list(node, list)
    {
        for_u64(i, node->item.size)
        {
            result.str[write_index++] = node->item.str[i];
        }
        
        if (count > 1)
        {
            count -= 1;
            for_u64(i, separator.size)
            {
                result.str[write_index++] = separator.str[i];
            }
        }
    }
    
    return result;
}












//~
static_function String
to_short_bytes_string(Arena *a, u64 number, u64 boundary = 8)
{
#define To_Short_Bytes_Boundary 8
    
    String result = {};
    if (number > exabytes(boundary))
    {
        result = stringf(a, "%lluEB", number/exabytes(1));
    }
    else if (number > petabytes(boundary))
    {
        result = stringf(a, "%lluPB", number/petabytes(1));
    }
    else if (number > terabytes(boundary))
    {
        result = stringf(a, "%lluTB", number/terabytes(1));
    }
    else if (number > gigabytes(boundary))
    {
        result = stringf(a, "%lluGB", number/gigabytes(1));
    }
    else if (number > megabytes(boundary))
    {
        result = stringf(a, "%lluMB", number/megabytes(1));
    }
    else if (number > kilobytes(boundary))
    {
        result = stringf(a, "%llukB", number/kilobytes(1));
    }
    else
    {
        result = stringf(a, "%lluB", number);
    }
    
    return result;
}


static_function String
to_short_decimal_string(Arena *a, u64 number, u64 boundary = 10)
{
    String result = {};
    if (number > exa(boundary))
    {
        result = stringf(a, "%lluE", number/exa(1));
    }
    else if (number > peta(boundary))
    {
        result = stringf(a, "%lluP", number/peta(1));
    }
    else if (number > tera(boundary))
    {
        result = stringf(a, "%lluT", number/tera(1));
    }
    else if (number > giga(boundary))
    {
        result = stringf(a, "%lluG", number/giga(1));
    }
    else if (number > mega(boundary))
    {
        result = stringf(a, "%lluM", number/mega(1));
    }
    else if (number > kilo(boundary))
    {
        result = stringf(a, "%lluk", number/kilo(1));
    }
    else
    {
        result = to_string(a, number);
    }
    
    return result;
}



