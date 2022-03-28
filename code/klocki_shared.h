#pragma once

#include "klocki_vec2_s64.h"
#include "klocki_vec3_s64.h"
#include "klocki_vec3_s32.h"

#define Fence_ReadWrite() _ReadWriteBarrier()

#define alloc_heap_memory(Type, Count) (Type *)malloc(sizeof(Type)*(Count))
#define free_heap_memory(Address) free(Address)

struct m4x4_inv
{
    m4x4 forward;
    m4x4 inverse;
};




// port back:
static_function v2
operator/(v2 a, f32 b)
{
    f32 inv = 1.f / b;
    v2 result {
        a.x * inv,
        a.y * inv,
    };
    return result;
}
static_function v3
operator/(v3 a, f32 b)
{
    f32 inv = 1.f / b;
    v3 result {
        a.x * inv,
        a.y * inv,
        a.z * inv,
    };
    return result;
}
static_function v4
operator/(v4 a, f32 b)
{
    f32 inv = 1.f / b;
    v4 result {
        a.x * inv,
        a.y * inv,
        a.z * inv,
        a.w * inv,
    };
    return result;
}


static_function v2
operator/=(v2 &a, f32 b)
{
    a = a / b;
    return a;
}
static_function v3
operator/=(v3 &a, f32 b)
{
    a = a / b;
    return a;
}
static_function v4
operator/=(v4 &a, f32 b)
{
    a = a / b;
    return a;
}




static_function f64
floor_f64(f64 value)
{
    f64 result = _mm_cvtsd_f64(_mm_round_sd(_mm_setzero_pd(), _mm_set_sd(value), Def_SSE4_Rounding_Floor));
    return result;
}

static_function s64
floor_f64_to_s64(f64 value)
{
    s64 result = _mm_cvtsd_si64(_mm_round_sd(_mm_setzero_pd(), _mm_set_sd(value), Def_SSE4_Rounding_Floor));
    return result;
}

static_function f64
square_root(f64 value)
{
    f64 result = _mm_cvtsd_f64(_mm_sqrt_sd(_mm_setzero_pd(), _mm_set_sd(value)));
    return result;
}


static_function f64
square(f64 a)
{
    f64 result = a * a;
    return result;
}



static_function s32
atomic_compare_exchange_s32(volatile s32 *destination, s32 exchange_with, s32 compare_to)
{
    long exchange = exchange_with;
    long compare = compare_to;
    s32 pre_exchange = InterlockedCompareExchange((volatile long *)destination, exchange, compare);
    return pre_exchange;
}


static_function s32
atomic_increment_s32(volatile s32 *value)
{
    s32 post_increment_result = InterlockedIncrement((volatile long *)value);
    return post_increment_result;
}

static_function s32
atomic_decrement_s32(volatile s32 *value)
{
    s32 post_decrement_result = InterlockedDecrement((volatile long *)value);
    return post_decrement_result;
}

static_function s32
atomic_add_s32(volatile s32 *value, s32 value_to_be_added)
{
    s32 post_add_result = InterlockedAdd((volatile long *)value, value_to_be_added);
    return post_add_result;
}









static_function v4
color_dim(v4 color, f32 dim_mul)
{
    color.xyz *= dim_mul;
    return color;
}



static_function s64
time_perf()
{
    LARGE_INTEGER large;
    QueryPerformanceCounter(&large);
    s64 result = large.QuadPart;
    return result;
}


static_function f32
time_elapsed(s64 recent, s64 old, f32 inv_freq)
{
    s64 delta = recent - old;
    f32 result = ((f32)delta *inv_freq);
    return result;
}




template <class T>
struct Preallocated_Array
{
    T *array;
    u64 capacity_count;
    u64 count;
    
    inline T *at(u64 index)
    {
        assert(array && capacity_count > 0);
        assert(index < count);
        T *result = array + index;
        return result;
    }
    
    inline T *grow(u64 grow_count = 1)
    {
        assert(array && capacity_count > 0);
        assert(grow_count > 0);
        T *result = array + count;
        count += grow_count;
        assert(count <= capacity_count);
        return result;
    }
    
    inline void reset(u64 new_count = 0)
    {
        assert(array && capacity_count > 0);
        assert(new_count <= count);
        assert(capacity_count >= new_count);
        count = new_count;
    }
};



template <typename T> static_function Preallocated_Array<T>
create_preallocated_array(T *array, u64 capacity_count, u64 initial_count = 0)
{
    Preallocated_Array<T> result = {};
    result.array = array;
    result.capacity_count = capacity_count;
    result.count = initial_count;
    return result;
}






static_function f32
truncate(f32 value)
{
    f32 result = _mm_cvtss_f32(_mm_round_ss(_mm_setzero_ps(), _mm_set_ss(value), Def_SSE4_Rounding_Truncate));
    return result;
};


static_function f32
float_modulo(f32 a, f32 mod)
{
    return a - truncate(a / mod) * mod;
}


static_function f32
float_wrap(f32 min, f32 a, f32 max)
{
    f32 range = max - min;
    f32 offset = a - min;
    
    f32 result = (offset - (floor_f32(offset / range) * range) + min);
    return result;
}


static_function v2
float_modulo(v2 a, f32 mod)
{
    // @simd
    v2 result = {
        float_modulo(a.x, mod),
        float_modulo(a.y, mod)
    };
    return result;
}





static_function v4
get_color(f32 r, f32 g, f32 b)
{
    v4 result = {r, g, b, 1};
    return result;
}

static_function v4
get_color(f32 rgb)
{
    v4 result = {rgb, rgb, rgb, 1};
    return result;
}


static_function v4
get_rgb(s32 r, s32 g, s32 b)
{
    f32 inv = 1.f / 255.f;
    v4 result = {(f32)r*inv, (f32)g*inv, (f32)b*inv, 1};
    return result;
}

static_function v4
get_argb(s32 hex)
{
    s32 a = (hex >> 24);
    s32 r = (hex >> 16) & 0xFF;
    s32 g = (hex >>  8) & 0xFF;
    s32 b = (hex >>  0) & 0xFF;
    f32 inv = 1.f / 255.f;
    
    v4 result = {
        (f32)r*inv,
        (f32)g*inv,
        (f32)b*inv,
        (f32)a*inv,
    };
    return result;
}


static_function v4
get_rgb(s32 hex)
{
    s32 r = (hex >> 16) & 0xFF;
    s32 g = (hex >>  8) & 0xFF;
    s32 b = (hex >>  0) & 0xFF;
    f32 inv = 1.f / 255.f;
    
    v4 result = {
        (f32)r*inv,
        (f32)g*inv,
        (f32)b*inv,
        1.f,
    };
    return result;
}






//
static_function u32
pack_pos15(s32 x, s32 y, s32 z)
{
    assert((x & Bitmask_15) == x);
    assert((y & Bitmask_15) == y);
    assert((z & Bitmask_15) == z);
    
    u32 packed15bit = (x |
                       (y << 5) |
                       (z << 10));
    return packed15bit;
}

static_function u8
pack_pos3(u8 x, u8 y, u8 z)
{
    u8 packed = ((x & Bitmask_1) |
                 ((y & Bitmask_1) << 1) |
                 ((z & Bitmask_1) << 2));
    return packed;
}

static_function u32
pack_color12(v4 color)
{
    u32 color12 = (((round_f32_to_u32(color.r*15.f) & Bitmask_4) << 8) |
                   ((round_f32_to_u32(color.g*15.f) & Bitmask_4) << 4) |
                   ((round_f32_to_u32(color.b*15.f) & Bitmask_4)     ));
    return color12;
}
static_function u32
pack_color18(v4 color)
{
    u32 color18 = (((round_f32_to_u32(color.r*63.f) & Bitmask_6) << 12) |
                   ((round_f32_to_u32(color.g*63.f) & Bitmask_6) << 6) |
                   ((round_f32_to_u32(color.b*63.f) & Bitmask_6)     ));
    return color18;
}



