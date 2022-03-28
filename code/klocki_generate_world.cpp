
static_function u64
my_stupid_hash1(u64 x, u64 y)
{
    x ^= (x >> 30);
    x *= 0xbf58476d1ce4e5b9;
    x ^= (x >> 27);
    x *= 0x94d049bb133111eb;
    x ^= (x >> 31);
    
    y ^= (rotate_right(y, 49) ^ rotate_right(y, 24));
    y *= 0x9fb21c651e98df25;
    y ^= (y >> 28);
    y *= 0x9fb21c651e98df25;
    y ^= (y >> 28);
    
    u64 result = x^y;
    return result;
}

static_function u64
my_stupid_hash2(u64 x, u64 y)
{
    x ^= (x >> 30);
    x *= 0xbf58476d1ce4e5b9;
    x ^= (x >> 27);
    x *= 0x94d049bb133111eb;
    x ^= (x >> 31);
    
    y ^= (rotate_right(y, 49) ^ rotate_right(y, 24));
    y *= 0x9fb21c651e98df25;
    y ^= (y >> 28);
    y *= 0x9fb21c651e98df25;
    y ^= (y >> 28);
    
    u32 rot_x = x >> 59u;
    u32 rot_y = y >> 59u;
    
    x = rotate_right(x, rot_y);
    y = rotate_right(y, rot_x);
    
    u64 result = x ^ y;
    return result;
}

static_function u64
my_stupid_hash3(u64 x, u64 y, u64 seed)
{
    __m128i s = _mm_set1_epi64x(seed);
    __m128i a = _mm_set_epi64x(x, y);
    a = _mm_aesdec_si128(s, a);
    a = _mm_aesdec_si128(a, a);
    a = _mm_aesdec_si128(a, a);
    a = _mm_aesdec_si128(a, a);
    u64 result = _mm_cvtsi128_si64(a);
    return result;
}


static_function u64
my_stupid_hash3(f64 x, f64 y, u64 seed)
{
    return my_stupid_hash3((u64)x, (u64)y, seed);
}


static_function Pcg_Entropy
rng_from_xy(s64 x, s64 y, u64 seed)
{
    Pcg_Entropy rng = {};
    rng.state = seed ^ my_stupid_hash1(x, y);
    rng.inc = (rotate_right(seed, 23) ^ my_stupid_hash1(y, x)) & 1;
    return rng;
}






static_function f32
simplex_noise11(u64 seed, f64 px, f64 py)
{
    f64 magic64 = (3. - square_root(3.)) / 6.;
    f32 magic32 = (3.f - square_root(3.f)) / 6.f;
    
    f64 s = (px + py) * ((square_root(3.) - 1.) * 0.5);
    
    f64 ix = floor_f64(px + s);
    f64 iy = floor_f64(py + s);
    
    f64 t = (ix + iy) * magic64;
    f32 x0 = (f32)(px - (ix - t));
    f32 y0 = (f32)(py - (iy - t));
    
    f32 ix1 = (x0 > y0);
    f32 iy1 = !ix1;
    
    f32 x1 = x0 - ix1 + magic32;
    f32 y1 = y0 - iy1 + magic32;
    f32 x2 = x0 - 1.f + 2.f*magic32;
    f32 y2 = y0 - 1.f + 2.f*magic32;
    
    u64 rng0 = my_stupid_hash3(ix, iy, seed);
    u64 rng1 = my_stupid_hash3(ix + ix1, iy + iy1, seed);
    u64 rng2 = my_stupid_hash3(ix + 1, iy + 1, seed);
    v2 grad0 = V2((rng0 & Bit_57) ? 1.f : -1.f, (rng0 & Bit_47) ? 1.f : -1.f);
    v2 grad1 = V2((rng1 & Bit_57) ? 1.f : -1.f, (rng1 & Bit_47) ? 1.f : -1.f);
    v2 grad2 = V2((rng2 & Bit_57) ? 1.f : -1.f, (rng2 & Bit_47) ? 1.f : -1.f);
    
    
    f32 n0 = 0.f;
    f32 n1 = 0.f;
    f32 n2 = 0.f;
    
    f32 t0 = 0.5f - square(x0) - square(y0);
    if (t0 >= 0)
    {
        t0 = square(t0);
        n0 = t0 * t0 * inner(grad0, V2(x0, y0));
    }
    
    f32 t1 = 0.5f - square(x1) - square(y1);
    if (t1 >= 0)
    {
        t1 = square(t1);
        n1 = t1 * t1 * inner(grad1, V2(x1, y1));
    }
    
    f32 t2 = 0.5f - square(x2) - square(y2);
    if (t2 >= 0)
    {
        t2 = square(t2);
        n2 = t2 * t2 * inner(grad2, V2(x2, y2));
    }
    
    f32 result = 70.f * (n0 + n1 + n2);
    return result;
}


static_function f32
simplex_noise01(u64 seed, f64 px, f64 py)
{
    return (simplex_noise11(seed, px, py)+1.f)*0.5f;
}


static_function f32
linearstep(f32 a, f32 t, f32 b)
{
    f32 result = clamp(0.f, (t - a)/(b - a), 1.f);
    return result;
}

static_function f32
smoothstep(f32 a, f32 t, f32 b)
{
#if Def_SuperSlow
    assert(a != b);
#endif
    f32 linear = clamp(0.f, (t - a)/(b - a), 1.f);
    f32 result = linear * linear * (3 - 2 * linear);
    return result;
}





struct Simplex_Parameters
{
    f64 x_ratio, y_ratio;
    f64 x_offset, y_offset;
    u64 seed;
    f64 weigth;
};
static_function f32
simplex_noise_params11(Simplex_Parameters *params, u64 param_count, f64 x, f64 y)
{
    f64 result = 0.;
    f64 contrib = 1. / (f64)param_count; 
    for_u64(param_index, param_count)
    {
        Simplex_Parameters param = params[param_index];
        f64 noise = simplex_noise11(param.seed,
                                    param.x_ratio * x + param.x_offset,
                                    param.y_ratio * y + param.y_offset);
        noise *= param.weigth;
        result += noise*contrib;
    }
    return (f32)result;
}
static_function f32
simplex_noise_params01(Simplex_Parameters *params, u64 param_count, f64 x, f64 y)
{
    f32 result = simplex_noise_params11(params, param_count, x, y);
    result = (result + 1.f) * 0.5f;
    return result;
}



#include "math.h" // @delete

struct Round_Cliff_Result
{
    f32 value;
    b32 did_anything;
};

static_function Round_Cliff_Result
round_cliff_edge(f32 t, f32 rounding_range)
{
    Round_Cliff_Result result = {t};
    
    if (t < rounding_range &&
        rounding_range > -1.f)
    {
        // map from [-1; rounding_range] to [1; rounding_range]
        // example: from [-1; -0.9] to [1; -0.9]
        t = (t + 1) * (rounding_range - 1);
        t /= (rounding_range + 1);
        t += 1;
        
        result = {t, true};
    }
    
    return result;
}

static_function f32
seed0_to_f32(u64 seed)
{
    f32 zero_one = (u32)(seed & Bitmask_32)*0x1.0p-32f;
    f32 result = zero_one*200.f - 100.f;
    return result;
}

static_function f32
seed1_to_f32(u64 seed)
{
    f32 zero_one = (u32)((seed >> 32) & Bitmask_32)*0x1.0p-32f;
    f32 result = zero_one*200.f - 100.f;
    return result;
}



static_function f32
simplex_octave(u64 seed, s32 octave_level, f64 x, f64 y)
{
    assert(octave_level >= 1);
    f64 offset_x = seed1_to_f32(seed);
    f64 offset_y = seed0_to_f32(seed);
    
    f64 result = 0;
    f64 octave_coef = 1.f;
    for_s32(i, octave_level)
    {
        result += simplex_noise11(seed + i, x*octave_coef + offset_x, y*octave_coef + offset_y);
        octave_coef *= 0.5;
    }
    return (f32)result;
}




static_function void
generate_world_chunk(Chunk *chunk)
{
    profile_function();
    
#define simplex_params01(Array, X, Y) simplex_noise_params01(Array, array_count(Array), X, Y)
#define simplex_params11(Array, X, Y) simplex_noise_params11(Array, array_count(Array), X, Y)
#define simplex_xz11(Seed, RatioX, RatioZ) simplex_noise11(Seed, (RatioX)*nx + seed0_to_f32(Seed), (RatioZ)*nz + seed1_to_f32(Seed))
#define simplex_xz01(Seed, RatioX, RatioZ) simplex_noise01(Seed, (RatioX)*nx + seed0_to_f32(Seed), (RatioZ)*nz + seed1_to_f32(Seed))
    
    
    if (chunk && !chunk->blocks_generated)
    {
        if (chunk->early_free)
        {
            decrement_chunk_lock(chunk);
            return;
        }
        
        
        u32 has_any_block = 0;
        
        Vec3_S64 at_chunk = chunk->at;
        Vec3_S64 chunk_start_corner = hadamard(at_chunk, Chunk_Dim_Vec3_S64);
        Vec3_S64 chunk_one_past_end_corner = chunk_start_corner + Vec3_S64 Chunk_Dim_Vec3_S64;
        f64 chunk_x_inv = 1. / (f64)Chunk_Dim_X;
        f64 chunk_z_inv = 1. / (f64)Chunk_Dim_Z;
        
        
        Pcg_Entropy rng = rng_from_xy(at_chunk.x, at_chunk.z, 0xfaba'baba);
        
        s32 leaves_ascent = 1;
        s32 tree_base_height = 5;
        Vec3_S64 trees[Chunk_Dim_X*Chunk_Dim_Z];
        s32 tree_counter = 0;
        
        
        
        
        
        for_s64(block_z, Chunk_Dim_Z)
        {
            f64 nz = (f64)at_chunk.z + chunk_z_inv*(f64)block_z;
            
            for_s64(block_x, Chunk_Dim_X)
            {
                f64 nx = (f64)at_chunk.x + chunk_x_inv*(f64)block_x;
                
                
                Block_Type overwrite_type = {};
                
                b32 smoothed_cliff = false;
                f32 ground_lv = 0.f;
                f32 cliff_cave_min_lv = 0.f;
                f32 cliff_cave_max_lv = 0.f;
                
                f32 sand_beach_offset_t = 0.f;
                f32 gravel_ocean_pit_t = 0.f;
                
                
                // @hack early out for now because we don't generate meaningful terrain in these ranges anyway
                // in the future terrain gen will be bundled vertically to some extent;
                // for now we put this hack in for slight perfomance boost
                if ((chunk_start_corner.y > 110 ||
                     chunk_start_corner.y < -80))
                {
                    ground_lv = 50.f;
                }
                else
                {
                    sand_beach_offset_t = simplex_noise01(0x9dfababa9a99a, 0.02f*nx, 0.02f*nz);
                    sand_beach_offset_t = 0.5f*sand_beach_offset_t + 0.5f;
                    sand_beach_offset_t = square(sand_beach_offset_t);
                    
                    gravel_ocean_pit_t = simplex_octave(0x7854785478545478, 3, 0.46f*nx, 0.46f*nz) / 3.f;
                    
                    
                    Simplex_Parameters hill_gate_params[] =
                    {
                        {0.01, 0.01, 23.1, 41.1, 0xf81971f8791f897, 1.},
                        {0.02, 0.02, 0.33, -13, 0x777777777721843, 1.},
                        {0.1, 0.1, 68.77, -99.1, 0x2223331233fff12, 1.},
                    };
                    f32 hill_gate = simplex_params01(hill_gate_params, nx, nz);
                    hill_gate = smoothstep(0.4f, hill_gate, 0.6f);
                    
                    
                    
                    if (hill_gate)
                    {
                        Simplex_Parameters hill_x_params[] =
                        {
                            {0.01, 0.01, -5.1, 32.1, 0x85267943678419, 1.},
                            {0.1, 0.1, 91.2, -5.64, 0xbaab0982374589, 1.},
                            {0.01, 0.01, 2.2, -42.6, 0xbea879ea8e9a99, 1.},
                        };
                        Simplex_Parameters hill_y_params[] =
                        {
                            {0.01, 0.01, 211.2, 4.3, 0xb9878980a908f0, 1.},
                            {0.1, 0.1, 93.4, -94.55, 0xc789fa8097c790, 1.},
                            {0.01, 0.01, 11.6, 2.3, 0xc09a0a8ca7895, 1.},
                        };
                        f32 hill_x = simplex_params11(hill_x_params, nx, nz);
                        //f32 hill_x = simplex_octave(0x192839329829094, 5, .15*nx, .15*nz) / 5.f;
                        f32 hill_y = simplex_params11(hill_y_params, nx, nz);
                        f32 hill_angle = atan2f(hill_x, hill_y) / Pi32;
                        
                        
                        
                        f32 high_hill = smoothstep(-0.2f, hill_angle, 0.7f);
                        f32 hill_mag_boost0 = smoothstep(0.35f, simplex_xz01(0x33434721378, 0.01373, 0.01337), 0.65f);
                        f32 hill_mag_boost1 = smoothstep(0.35f, simplex_xz01(0x12390cacbab4231, 0.0724, 0.0724), 0.65f);
                        f32 hill_mag_boost2 = smoothstep(0.35f, simplex_xz01(0xabc0effebfeafbd, 0.154, 0.154), 0.65f);
                        f32 hill_magnitude = 50.f + 15.f*hill_mag_boost0 + 15.f*hill_mag_boost1 + 10.f*hill_mag_boost2;
                        
                        
                        f32 cliff_smoothing_range = 0.999f - 0.15f*simplex_xz01(0x90401cccacacaca, 0.32, 0.32);
                        f32 cliff_smoothing = smoothstep(cliff_smoothing_range, -hill_angle, 1.f);
                        
                        f32 cliff_smoothing_height = 0.4f*simplex_xz01(0x93cccccc5904331, 0.372, 0.372);
                        
                        f32 cliff_smoothing_depth = -0.01f - 0.5f*simplex_xz01(0xca0158959ca21, 0.0911, 0.0911);
                        cliff_smoothing = lerp(cliff_smoothing_depth, cliff_smoothing, cliff_smoothing_height);
                        
                        smoothed_cliff = (cliff_smoothing > 0.1f);
                        
                        
                        f32 hill = (cliff_smoothing + high_hill)*hill_magnitude;
                        ground_lv = hill_gate*hill;
                        
                        
                        
                        
                        
                        
                        // cliff caves / overhang mountains
                        
                        f32 half_hill_magnitude = 0.5f*hill_magnitude;
                        f32 cliff_cave_center = simplex_xz01(0x66666666663343, 0.2, 0.2);
                        cliff_cave_center = square(cliff_cave_center);
                        cliff_cave_center *= 0.5f*half_hill_magnitude;
                        
                        
                        f32 cliff_cave_half_height = simplex_xz11(0x123f0f0f0f0f00, 0.106, 0.106);
                        cliff_cave_half_height = smoothstep(0.4f, cliff_cave_half_height, 0.7f);
                        cliff_cave_half_height *= half_hill_magnitude*0.5f;
                        
                        f32 cliff_cave_top_height_offset = simplex_xz11(0x178293459380fafa, 0.184, 0.184);
                        cliff_cave_top_height_offset = smoothstep(0.4f, cliff_cave_top_height_offset, 0.7f);
                        cliff_cave_top_height_offset *= half_hill_magnitude;
                        
                        
                        f32 cliff_cave_gate_depth = 0.98f - 0.18f*simplex_xz01(0x9201388912739119, 0.01, 0.01);
                        f32 cliff_cave_gate = smoothstep(-0.8f, hill_angle, -0.99f);
                        cliff_cave_gate += smoothstep(cliff_cave_gate_depth, hill_angle, 1.f);
                        //cliff_cave_gate *= hill_gate;
                        
                        
                        cliff_cave_min_lv = cliff_cave_center - cliff_cave_half_height*cliff_cave_gate;
                        cliff_cave_max_lv = cliff_cave_center + cliff_cave_half_height*cliff_cave_gate;
                        cliff_cave_min_lv -= cliff_cave_top_height_offset*cliff_cave_gate*0.5f;
                        cliff_cave_max_lv += cliff_cave_top_height_offset*cliff_cave_gate;
                    }
                    
                    
                    if (hill_gate != 1.f)
                    {
                        Simplex_Parameters round_hills_params[] =
                        {
                            {0.0777, 0.0777, 45.2, -12.1, 0xffbea1a9019, 1.},
                            {0.1421, 0.1421, -192.1, 82.65, 0xc99c76e776e87, 1.},
                            {0.21, 0.21, -201.1, 44.22, 0x8789eada87765d, 1.},
                        };
                        f32 round_hills = simplex_params01(round_hills_params, nx, nz);
                        
                        
                        Simplex_Parameters round_hills_mag_params[] =
                        {
                            {0.0492, 0.0492, -92.1, -433.73, 0xbe0be012e331, 1.},
                            {0.0921, 0.0921, -72.75, -29.1, 0xceba4bceba01, 1.},
                        };
                        f32 round_hills_mag = simplex_params01(round_hills_mag_params, nx, nz);
                        round_hills_mag = smoothstep(0.4f, round_hills_mag, 0.9f);
                        
                        f32 round_hills_lv = (1.f - hill_gate)*round_hills*round_hills_mag*36.f;
                        ground_lv += round_hills_lv;
                    }
                    
                    
                    
                    {
                        f32 is_terrain_low = smoothstep(40.f, ground_lv, 15.f);
#if 0
                        Simplex_Parameters plains_params[] =
                        {
                            {0.0165, 0.0165, 123.1, 43.2, 0xbead98dfeadf09e, 1.},
                            {0.0476, 0.0476, 77.1, -233.12, 0xba897b89ab6, 1.},
                            {0.114, 0.114, -232.45, 21.52, 0x9209872131287ab, 1.},
                        };
                        f32 plains = simplex_params01(plains_params, nx, nz);
#else
                        f32 plains = simplex_octave(0x35362536bcbcbaaa, 6, .556*nx, .556*nz) / 6.f;
                        plains += 0.5f;
#endif
                        f32 plains_lv = is_terrain_low*plains*15.f;
                        ground_lv += plains_lv;
                    }
                }
                
                
                
                
                
                s64 cliff_cave_min = round_f32_to_s32(cliff_cave_min_lv);
                s64 cliff_cave_max = round_f32_to_s32(cliff_cave_max_lv);
                
                s64 ground_height = round_f32_to_s32(ground_lv);
                s64 stone_level = ground_height - 4;
                
                s64 sand_beach_height_offset = round_f32_to_s32(sand_beach_offset_t*4.f);
                s64 sand_beach_depth_offset = round_f32_to_s32(sand_beach_offset_t*2.f);
                
                
                for_s64(block_y, Chunk_Dim_Y)
                {
                    Vec3_S64 at_block = vec3_s64(block_x, block_y, block_z);
                    Vec3_S64 at_world = chunk_start_corner + at_block;
                    //
                    Block *block = &chunk->blocks[block_z][block_y][block_x];
                    Block_Type set_type = {};
                    //-
                    
                    if (at_world.y > cliff_cave_min && at_world.y < cliff_cave_max)
                    {
                        //set_type = Block_Pumpkin;
                    }
                    else if (at_world.y <= ground_height)
                    {
                        set_type = Block_Stone;
                        
                        
                        
                        if (at_world.y > stone_level)
                        {
                            if (at_world.y >= sand_beach_height_offset)
                            {
                                if (at_world.y == ground_height)
                                {
                                    set_type = Block_Grass;
                                }
                                else 
                                {
                                    set_type = Block_Dirt;
                                }
                                
                                
                                
                                if (smoothed_cliff)
                                {
                                    set_type = (at_world.y == ground_height ? Block_Grass : Block_Dirt);
                                }
                                
                                
                                if (cliff_cave_min != cliff_cave_max &&
                                    at_world.y == cliff_cave_min)
                                {
                                    set_type = Block_Grass;
                                }
                            }
                            else
                            {
                                set_type = Block_Sand;
                                
                                if (at_world.y < -sand_beach_depth_offset)
                                {
                                    set_type = Block_Dirt;
                                    
                                    if (sand_beach_offset_t > 0.96f)
                                    {
                                        set_type = Block_Sand;
                                    }
                                    
                                    if (gravel_ocean_pit_t > 0.4f)
                                    {
                                        set_type = Block_Gravel;
                                    }
                                    
                                }
                            }
                        }
                        
                        
                        
                        
                        
                        if (overwrite_type)
                        {
                            set_type = overwrite_type;
                        }
                    }
                    
                    
                    if (at_world.y <= 0 && !set_type)
                    {
                        set_type = Block_Water;
                    }
                    
                    
                    
                    
                    //-
                    *block = {set_type};
                    chunk->can_contain_blocks |= !!set_type;
                }
                
                
                
                
                
                // super hacky randomize & test if tree can be placed here
                if (tree_counter < array_count(trees))
                {
                    u32 place_a_tree = rand_u32(&rng, 1000);
                    //do_a_tree = 0;
                    
                    if (!place_a_tree)
                    {
                        s64 ground_index = ground_height - chunk_start_corner.y;
                        
                        if (ground_index >= 0 && ground_index < Chunk_Dim_Y)
                        {
                            s32 leaves_radius = 3;
                            
                            if (block_z - leaves_radius >= 0 && block_z + leaves_radius < Chunk_Dim_Z &&
                                block_x - leaves_radius >= 0 && block_x + leaves_radius < Chunk_Dim_X)
                            {
                                s64 possible_height = Chunk_Dim_Y - ground_index - 1;
                                if (possible_height >= (tree_base_height + leaves_ascent))
                                {
                                    f32 tree_height_variance = (f32)(possible_height - leaves_ascent - tree_base_height);
                                    clamp_ref_top(&tree_height_variance, 5.f);
                                    
                                    f32 tree_height_f = tree_base_height + tree_height_variance*rand_f32_01(&rng);
                                    s32 tree_height = round_f32_to_s32(tree_height_f);
                                    s32 total_height = tree_height + leaves_ascent;
                                    
                                    
                                    if (ground_index + total_height < Chunk_Dim_Y)
                                    {
                                        Block *ground_block = safe_get_block(chunk, block_x, ground_index, block_z);
                                        if (ground_block->type == Block_Grass)
                                        {
                                            trees[tree_counter] = {block_x, block_z, ground_index};
                                            tree_counter += 1;
                                        }
                                    }
                                    else
                                    {
                                        assert(0);
                                    }
                                }
                            }
                        }
                    }
                }
                else
                {
                    assert(0);
                }
                
            }
        }
        
        
        
        
        // deferred tree placement
        for_s32(tree_index, tree_counter)
        {
            s64 tree_x = trees[tree_index].x;
            s64 tree_z = trees[tree_index].y;
            s64 ground_index = trees[tree_index].z;
            
            s64 possible_height = Chunk_Dim_Y - ground_index - 1;
            f32 tree_height_variance = (f32)(possible_height - leaves_ascent - tree_base_height);
            clamp_ref_top(&tree_height_variance, 3.f);
            
            f32 tree_height_f = tree_base_height + tree_height_variance*rand_f32_01(&rng);
            s32 tree_height = round_f32_to_s32(tree_height_f);
            
            
            s32 lower_leaves_start = tree_height - 3;
            s32 upper_leaves_start = tree_height - 1;
            
            for_s32(tree_level, tree_height)
            {
                s64 y = ground_index + tree_level + 1;
                Block *tree_block = safe_get_block(chunk, tree_x, y, tree_z);
                tree_block->type = Block_OakLog;
                
                if (tree_level >= lower_leaves_start)
                {
                    s64 radius = 2;
                    if (tree_level >= upper_leaves_start) { radius = 1; }
                    
                    
                    for (s64 x_off = -radius;
                         x_off <= radius;
                         x_off += 1)
                    {
                        s64 x = tree_x + x_off;
                        
                        for (s64 z_off = -radius;
                             z_off <= radius;
                             z_off += 1)
                        {
                            s64 z = tree_z + z_off;
                            
                            if ((x_off == -radius || x_off == radius) &&
                                (z_off == -radius || z_off == radius))
                            {
                                s32 chance = 2 + tree_level;
                                if (tree_level + 1 == tree_height) { chance += tree_level; }
                                
                                u32 skip_this = rand_u32(&rng, chance);
                                if (skip_this) { continue; }
                            }
                            
                            Block *leaves_block = safe_get_block(chunk, x, y, z);
                            if (!leaves_block->type)
                            {
                                leaves_block->type = Block_OakLeaves;
                                //leaves_block->type = Block_LapisBlock;
                            }
                        }
                    }
                }
            }
            
            s64 y = ground_index + tree_height + 1;
            safe_get_block(chunk, tree_x, y, tree_z)->type = Block_OakLeaves;
            safe_get_block(chunk, tree_x, y, tree_z - 1)->type = Block_OakLeaves;
            safe_get_block(chunk, tree_x, y, tree_z + 1)->type = Block_OakLeaves;
            safe_get_block(chunk, tree_x - 1, y, tree_z)->type = Block_OakLeaves;
            safe_get_block(chunk, tree_x + 1, y, tree_z)->type = Block_OakLeaves;
        }
        
        
        
        Fence_ReadWrite();
        chunk->blocks_generated = true;
        decrement_chunk_lock(chunk);
    }
    else
    {
        assert(0);
    }
}







//
//
// Old unused code
//
//


struct Perlin_Chunk
{
    v2 g0, g1, g2, g3;
    v2 p_mul;
    v2 p_add;
};



static_function f32
perlin_fade(f32 t)
{
    f32 result = t*t*t*(t*(t*6.0f - 15.0f) + 10.0f);
    return result;
}


static_function f32
perlin_noise(Perlin_Chunk perlin, v2 p)
{
    p = hadamard(p, perlin.p_mul);
    p += perlin.p_add;
    assert(p.x >= 0.f && p.x <= 1.f);
    assert(p.y >= 0.f && p.y <= 1.f);
    
    v2 p_p0 = p;
    v2 p_p1 = p_p0 - V2(1, 0);
    v2 p_p2 = p_p0 - V2(0, 1);
    v2 p_p3 = p_p0 - V2(1, 1);
    
    f32 t0 = p_p0.x;
    f32 fade_t0 = perlin_fade(t0);
    
    f32 t1 = p_p0.y;
    f32 fade_t1 = perlin_fade(t1);
    
    f32 p0p1 = (1.f - fade_t0) * inner(perlin.g0, p_p0) + fade_t0 * inner(perlin.g1, p_p1);
    f32 p2p3 = (1.f - fade_t0) * inner(perlin.g2, p_p2) + fade_t0 * inner(perlin.g3, p_p3);
    
    f32 noise = lerp(p0p1, fade_t1, p2p3);
    return noise;
}



static_function Perlin_Chunk
get_perlin_chunk(s64 chunk_x, s64 chunk_z, u64 seed,
                 s64 distance_x = 1, s64 distance_z = 1)
{
    f32 inv_len = 1.f / square_root(2.f);
    
    s64 dx = distance_x;
    s64 dz = distance_z;
    u64 x = (u64)chunk_x + (u64)(S64_Min);
    u64 z = (u64)chunk_z + (u64)(S64_Min);
    
    u64 x0 = (x / dx);
    u64 z0 = (z / dz);
    u64 x1 = (x0 + 1);
    u64 z1 = (z0 + 1);
    
    
    Perlin_Chunk result = {};
#if 1
    u64 rng0 = my_stupid_hash3(x0, z0, seed);
    u64 rng1 = my_stupid_hash3(x1, z0, seed);
    u64 rng2 = my_stupid_hash3(x0, z1, seed);
    u64 rng3 = my_stupid_hash3(x1, z1, seed);
    result.g0 = V2((rng0 & Bit_57) ? 1.f : -1.f, (rng0 & Bit_47) ? 1.f : -1.f);
    result.g1 = V2((rng1 & Bit_57) ? 1.f : -1.f, (rng1 & Bit_47) ? 1.f : -1.f);
    result.g2 = V2((rng2 & Bit_57) ? 1.f : -1.f, (rng2 & Bit_47) ? 1.f : -1.f);
    result.g3 = V2((rng3 & Bit_57) ? 1.f : -1.f, (rng3 & Bit_47) ? 1.f : -1.f);
#elif 0
    u64 rng0 = seed ^ my_stupid_hash2(x0, z0);
    u64 rng1 = seed ^ my_stupid_hash2(x1, z0);
    u64 rng2 = seed ^ my_stupid_hash2(x0, z1);
    u64 rng3 = seed ^ my_stupid_hash2(x1, z1);
    result.g0 = V2((rng0 & Bit_57) ? 1.f : -1.f, (rng0 & Bit_47) ? 1.f : -1.f);
    result.g1 = V2((rng1 & Bit_57) ? 1.f : -1.f, (rng1 & Bit_47) ? 1.f : -1.f);
    result.g2 = V2((rng2 & Bit_57) ? 1.f : -1.f, (rng2 & Bit_47) ? 1.f : -1.f);
    result.g3 = V2((rng3 & Bit_57) ? 1.f : -1.f, (rng3 & Bit_47) ? 1.f : -1.f);
#else
    // @todo This is  wasteful;
    Pcg_Entropy rng0 = rng_from_xy(x0, z0, seed);
    Pcg_Entropy rng1 = rng_from_xy(x1, z0, seed);
    Pcg_Entropy rng2 = rng_from_xy(x0, z1, seed);
    Pcg_Entropy rng3 = rng_from_xy(x1, z1, seed);
    result.g0 = V2((rand_choice(&rng0) ? 1.f : -1.f), (rand_choice(&rng0) ? 1.f : -1.f));
    result.g1 = V2((rand_choice(&rng1) ? 1.f : -1.f), (rand_choice(&rng1) ? 1.f : -1.f));
    result.g2 = V2((rand_choice(&rng2) ? 1.f : -1.f), (rand_choice(&rng2) ? 1.f : -1.f));
    result.g3 = V2((rand_choice(&rng3) ? 1.f : -1.f), (rand_choice(&rng3) ? 1.f : -1.f));
#endif
    
    result.g0 = result.g0 * inv_len;
    result.g1 = result.g1 * inv_len;
    result.g2 = result.g2 * inv_len;
    result.g3 = result.g3 * inv_len;
    
    result.p_mul = V2(1.f / dx, 1.f / dz);
    
    s64 wx = (x - x0*dx);
    s64 wz = (z - z0*dz);
    result.p_add = V2((f32)wx, (f32)wz);
    result.p_add = hadamard(result.p_add, result.p_mul);
    
    return result;
}



static_function void
generate_world_chunk_rng_test(Chunk *chunk)
{
    profile_function();
    
    
    if (chunk && !chunk->blocks_generated)
    {
        if (chunk->early_free)
        {
            decrement_chunk_lock(chunk);
            return;
        }
        
        
        Vec3_S64 at_chunk = chunk->at;
        
        
        
        for_s64(block_z, Chunk_Dim_Z)
        {
            for_s64(block_x, Chunk_Dim_X)
            {
                s64 sea_level = 0;
                
                
                for_s64(block_y, Chunk_Dim_Y)
                {
                    Vec3_S64 at_block = vec3_s64(block_x, block_y, block_z);
                    Vec3_S64 at_world = hadamard(at_chunk, Chunk_Dim_Vec3_S64) + at_block;
                    //
                    Block *block = &chunk->blocks[block_z][block_y][block_x];
                    Block_Type set_type = {};
                    //-
                    
                    v2 noise_p = V2((f32)block_x / (f32)Chunk_Dim_X, (f32)block_z / (f32)Chunk_Dim_Z);
                    
                    
                    
#if 1 // just hash
                    s64 offset = my_stupid_hash3((u64)at_world.x, (u64)at_world.z, 0xfefefafaf0f);
#endif
                    
                    
#if 0 // perlin every block
                    Perlin_Chunk perlin = get_perlin_chunk(at_world.x, at_world.z, 0x1234'FFFF'5675'1234);
                    f32 offset = (perlin_noise(perlin, V2(0.3f)) +
                                  perlin_noise(perlin, V2(0.4f)) +
                                  perlin_noise(perlin, V2(0.6f)) +
                                  perlin_noise(perlin, V2(0.7f)))*0.25f;
#endif
                    
                    
#if 0 // perlin chunks "world" (slow - perlin chunk should be pulled above)
                    Perlin_Chunk perlin = get_perlin_chunk(at_chunk.x, at_chunk.z, 0x1234'FFFF'5675'1234);
                    f32 offset = perlin_noise(perlin, noise_p);
#endif
                    
                    
#if 0 // simplex every block
                    f32 offset = simplex_noise(vec2_s64(at_world.x, at_world.z), V2(0.5f));
#endif
                    
                    
#if 0 // simplex chunks "world"
                    f32 offset = simplex_noise(0xffff, vec2_s64(at_chunk.x, at_chunk.z), noise_p);
#endif
                    
                    
                    //assert(offset >= -1.f && offset <= 1.f);
                    //s64 ground_level = sea_level + round_f32_to_s32(offset * 10.f);
                    s64 ground_level = sea_level + offset;
                    ground_level %= 20;
                    
                    
                    if (at_world.y < ground_level)
                    {
                        set_type = map_s64_to_block_type(at_world.y);
                    }
                    
                    
                    
                    //-
                    *block = {set_type};
                    chunk->can_contain_blocks |= !!set_type;
                }
            }
        }
        
        
        Fence_ReadWrite();
        chunk->blocks_generated = true;
        decrement_chunk_lock(chunk);
    }
    else
    {
        assert(0);
    }
}

