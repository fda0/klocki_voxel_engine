#pragma once

struct Pcg_Entropy
{
    u64 state;
    u64 inc;
};

static_function u32
rand_full_u32(Pcg_Entropy *rng)
{
    u64 old_state = rng->state;
    
    // Advance internal state
    rng->state = old_state * 6364136223846793005ULL + (rng->inc|1);
    
    // Calculate output function (XSH RR), uses old state for max ILP
    u32 xor_shifted = (u32)(((old_state >> 18u) ^ old_state) >> 27u);
    u32 rot = old_state >> 59u;
    
    return (xor_shifted >> rot) | (xor_shifted << ((-(s32)rot) & 31));
}

static_function u32
rand_u32(Pcg_Entropy *rng, u32 range)
{
    u32 x = rand_full_u32(rng);
    u64 m = (u64)x * (u64)range;
    return m >> 32;
}


static_function s32
rand_s32(Pcg_Entropy *rng, s32 min, s32 max)
{
    return (s32)rand_u32(rng, max - min) + min;
}

static_function s32
rand_s32(Pcg_Entropy *rng, s32 min_max)
{
    return (s32)rand_u32(rng, min_max*2) - min_max;
}



static_function f32
rand_f32_01(Pcg_Entropy *rng)
{
    return 0x1.0p-32f * rand_full_u32(rng);
}

static_function f32
rand_f32_11(Pcg_Entropy *rng)
{
    return rand_f32_01(rng)*2.f - 1.f;
}

static_function b32
rand_choice(Pcg_Entropy *rng)
{
    return !(rand_full_u32(rng) & Bit_14);
}



