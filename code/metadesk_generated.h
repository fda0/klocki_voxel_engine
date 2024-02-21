enum Block_Type : u8
{
    Block_Air,

    Block_Stone,
    Block_Cobblestone,
    Block_Dirt,
    Block_Grass,
    Block_Gravel,
    Block_Sandstone,
    Block_Sand,
    Block_CoalOre,
    Block_CoalBlock,
    Block_IronOre,
    Block_IronBlock,
    Block_GoldOre,
    Block_GoldBlock,
    Block_DiamondOre,
    Block_DiamondBlock,
    Block_EmeraldOre,
    Block_EmeraldBlock,
    Block_RedstoneOre,
    Block_RedstoneBlock,
    Block_LapisOre,
    Block_LapisBlock,
    Block_Brick,
    Block_StoneBrick,
    Block_CraftingTable,
    Block_Furnace,
    Block_Barrel,
    Block_Bookshelf,
    Block_Tnt,
    Block_Piston,
    Block_Sponge,
    Block_AcaciaLog,
    Block_AcaciaPlanks,
    Block_BirchLog,
    Block_BirchPlanks,
    Block_DarkOakLog,
    Block_DarkOakPlanks,
    Block_JungleLog,
    Block_JunglePlanks,
    Block_OakLog,
    Block_OakPlanks,
    Block_SpruceLog,
    Block_SprucePlanks,
    Block_Pumpkin,
    Block_Melon,
    Block_HayBlock,
    Block_DriedKelp,
    Block_HoneycombBlock,
    Block_BeeNest,
    Block_Beehive,
    Block_OakLeaves,
    Block_Glass,
    Block_Ice,
    Block_Honey,
    Block_Water,
    Block_TestDirOpaque,
    Block_TestDirTranslucent,

    Block_Count,
};

static_function String
get_block_name_string(u32 block_type)
{
    String result = {};
    switch (block_type)
    {
        Invalid_Default_Case;
        case Block_Air: result = "Air"_f0; break;
        case Block_Stone: result = "Stone"_f0; break;
        case Block_Cobblestone: result = "Cobblestone"_f0; break;
        case Block_Dirt: result = "Dirt"_f0; break;
        case Block_Grass: result = "Grass"_f0; break;
        case Block_Gravel: result = "Gravel"_f0; break;
        case Block_Sandstone: result = "Sandstone"_f0; break;
        case Block_Sand: result = "Sand"_f0; break;
        case Block_CoalOre: result = "CoalOre"_f0; break;
        case Block_CoalBlock: result = "CoalBlock"_f0; break;
        case Block_IronOre: result = "IronOre"_f0; break;
        case Block_IronBlock: result = "IronBlock"_f0; break;
        case Block_GoldOre: result = "GoldOre"_f0; break;
        case Block_GoldBlock: result = "GoldBlock"_f0; break;
        case Block_DiamondOre: result = "DiamondOre"_f0; break;
        case Block_DiamondBlock: result = "DiamondBlock"_f0; break;
        case Block_EmeraldOre: result = "EmeraldOre"_f0; break;
        case Block_EmeraldBlock: result = "EmeraldBlock"_f0; break;
        case Block_RedstoneOre: result = "RedstoneOre"_f0; break;
        case Block_RedstoneBlock: result = "RedstoneBlock"_f0; break;
        case Block_LapisOre: result = "LapisOre"_f0; break;
        case Block_LapisBlock: result = "LapisBlock"_f0; break;
        case Block_Brick: result = "Brick"_f0; break;
        case Block_StoneBrick: result = "StoneBrick"_f0; break;
        case Block_CraftingTable: result = "CraftingTable"_f0; break;
        case Block_Furnace: result = "Furnace"_f0; break;
        case Block_Barrel: result = "Barrel"_f0; break;
        case Block_Bookshelf: result = "Bookshelf"_f0; break;
        case Block_Tnt: result = "Tnt"_f0; break;
        case Block_Piston: result = "Piston"_f0; break;
        case Block_Sponge: result = "Sponge"_f0; break;
        case Block_AcaciaLog: result = "AcaciaLog"_f0; break;
        case Block_AcaciaPlanks: result = "AcaciaPlanks"_f0; break;
        case Block_BirchLog: result = "BirchLog"_f0; break;
        case Block_BirchPlanks: result = "BirchPlanks"_f0; break;
        case Block_DarkOakLog: result = "DarkOakLog"_f0; break;
        case Block_DarkOakPlanks: result = "DarkOakPlanks"_f0; break;
        case Block_JungleLog: result = "JungleLog"_f0; break;
        case Block_JunglePlanks: result = "JunglePlanks"_f0; break;
        case Block_OakLog: result = "OakLog"_f0; break;
        case Block_OakPlanks: result = "OakPlanks"_f0; break;
        case Block_SpruceLog: result = "SpruceLog"_f0; break;
        case Block_SprucePlanks: result = "SprucePlanks"_f0; break;
        case Block_Pumpkin: result = "Pumpkin"_f0; break;
        case Block_Melon: result = "Melon"_f0; break;
        case Block_HayBlock: result = "HayBlock"_f0; break;
        case Block_DriedKelp: result = "DriedKelp"_f0; break;
        case Block_HoneycombBlock: result = "HoneycombBlock"_f0; break;
        case Block_BeeNest: result = "BeeNest"_f0; break;
        case Block_Beehive: result = "Beehive"_f0; break;
        case Block_OakLeaves: result = "OakLeaves"_f0; break;
        case Block_Glass: result = "Glass"_f0; break;
        case Block_Ice: result = "Ice"_f0; break;
        case Block_Honey: result = "Honey"_f0; break;
        case Block_Water: result = "Water"_f0; break;
        case Block_TestDirOpaque: result = "TestDirOpaque"_f0; break;
        case Block_TestDirTranslucent: result = "TestDirTranslucent"_f0; break;
    }
    return result;
}

static_function String
get_block_pretty_name_string(u32 block_type)
{
    String result = {};
    switch (block_type)
    {
        Invalid_Default_Case;
        case Block_Air: result = "Air"_f0; break;
        case Block_Stone: result = "Stone"_f0; break;
        case Block_Cobblestone: result = "Cobblestone"_f0; break;
        case Block_Dirt: result = "Dirt"_f0; break;
        case Block_Grass: result = "Grass"_f0; break;
        case Block_Gravel: result = "Gravel"_f0; break;
        case Block_Sandstone: result = "Sandstone"_f0; break;
        case Block_Sand: result = "Sand"_f0; break;
        case Block_CoalOre: result = "Coal Ore"_f0; break;
        case Block_CoalBlock: result = "Coal Block"_f0; break;
        case Block_IronOre: result = "Iron Ore"_f0; break;
        case Block_IronBlock: result = "Iron Block"_f0; break;
        case Block_GoldOre: result = "Gold Ore"_f0; break;
        case Block_GoldBlock: result = "Gold Block"_f0; break;
        case Block_DiamondOre: result = "Diamond Ore"_f0; break;
        case Block_DiamondBlock: result = "Diamond Block"_f0; break;
        case Block_EmeraldOre: result = "Emerald Ore"_f0; break;
        case Block_EmeraldBlock: result = "Emerald Block"_f0; break;
        case Block_RedstoneOre: result = "Redstone Ore"_f0; break;
        case Block_RedstoneBlock: result = "Redstone Block"_f0; break;
        case Block_LapisOre: result = "Lapis Ore"_f0; break;
        case Block_LapisBlock: result = "Lapis Block"_f0; break;
        case Block_Brick: result = "Brick"_f0; break;
        case Block_StoneBrick: result = "Stone Brick"_f0; break;
        case Block_CraftingTable: result = "Crafting Table"_f0; break;
        case Block_Furnace: result = "Furnace"_f0; break;
        case Block_Barrel: result = "Barrel"_f0; break;
        case Block_Bookshelf: result = "Bookshelf"_f0; break;
        case Block_Tnt: result = "Tnt"_f0; break;
        case Block_Piston: result = "Piston"_f0; break;
        case Block_Sponge: result = "Sponge"_f0; break;
        case Block_AcaciaLog: result = "Acacia Log"_f0; break;
        case Block_AcaciaPlanks: result = "Acacia Planks"_f0; break;
        case Block_BirchLog: result = "Birch Log"_f0; break;
        case Block_BirchPlanks: result = "Birch Planks"_f0; break;
        case Block_DarkOakLog: result = "Dark Oak Log"_f0; break;
        case Block_DarkOakPlanks: result = "Dark Oak Planks"_f0; break;
        case Block_JungleLog: result = "Jungle Log"_f0; break;
        case Block_JunglePlanks: result = "Jungle Planks"_f0; break;
        case Block_OakLog: result = "Oak Log"_f0; break;
        case Block_OakPlanks: result = "Oak Planks"_f0; break;
        case Block_SpruceLog: result = "Spruce Log"_f0; break;
        case Block_SprucePlanks: result = "Spruce Planks"_f0; break;
        case Block_Pumpkin: result = "Pumpkin"_f0; break;
        case Block_Melon: result = "Melon"_f0; break;
        case Block_HayBlock: result = "Hay Block"_f0; break;
        case Block_DriedKelp: result = "Dried Kelp"_f0; break;
        case Block_HoneycombBlock: result = "Honeycomb Block"_f0; break;
        case Block_BeeNest: result = "Bee Nest"_f0; break;
        case Block_Beehive: result = "Beehive"_f0; break;
        case Block_OakLeaves: result = "Oak Leaves"_f0; break;
        case Block_Glass: result = "Glass"_f0; break;
        case Block_Ice: result = "Ice"_f0; break;
        case Block_Honey: result = "Honey"_f0; break;
        case Block_Water: result = "Water"_f0; break;
        case Block_TestDirOpaque: result = "Test Dir Opaque"_f0; break;
        case Block_TestDirTranslucent: result = "Test Dir Translucent"_f0; break;
    }
    return result;
}

static_global Block_Tex_Ids map_block_to_tex_id[] =
{
    {},
    {0,0,0,0,0,0,},
    {1,1,1,1,1,1,},
    {2,2,2,2,2,2,},
    {3,2,4,4,4,4,},
    {5,5,5,5,5,5,},
    {7,6,8,8,8,8,},
    {9,9,9,9,9,9,},
    {10,10,10,10,10,10,},
    {11,11,11,11,11,11,},
    {12,12,12,12,12,12,},
    {13,13,13,13,13,13,},
    {14,14,14,14,14,14,},
    {15,15,15,15,15,15,},
    {16,16,16,16,16,16,},
    {17,17,17,17,17,17,},
    {18,18,18,18,18,18,},
    {19,19,19,19,19,19,},
    {20,20,20,20,20,20,},
    {21,21,21,21,21,21,},
    {22,22,22,22,22,22,},
    {23,23,23,23,23,23,},
    {24,24,24,24,24,24,},
    {25,25,25,25,25,25,},
    {28,29,26,27,26,27,},
    {31,31,30,32,32,32,},
    {33,34,35,35,35,35,},
    {29,29,36,36,36,36,},
    {37,39,38,38,38,38,},
    {40,42,41,41,41,41,},
    {43,43,43,43,43,43,},
    {44,44,45,45,45,45,},
    {46,46,46,46,46,46,},
    {47,47,48,48,48,48,},
    {49,49,49,49,49,49,},
    {50,50,51,51,51,51,},
    {52,52,52,52,52,52,},
    {53,53,54,54,54,54,},
    {55,55,55,55,55,55,},
    {56,56,57,57,57,57,},
    {29,29,29,29,29,29,},
    {58,58,59,59,59,59,},
    {60,60,60,60,60,60,},
    {61,61,62,62,62,62,},
    {63,63,64,64,64,64,},
    {65,65,66,66,66,66,},
    {67,68,69,69,69,69,},
    {70,70,70,70,70,70,},
    {72,73,71,74,74,74,},
    {76,76,75,77,77,77,},
    {78,78,78,78,78,78,},
    {79,79,79,79,79,79,},
    {80,80,80,80,80,80,},
    {81,82,83,83,83,83,},
    {84,84,84,84,84,84,},
    {85,86,87,88,89,90,},
    {91,92,93,94,95,96,},
};

static_global To_Load_Texture array_to_load_textures[] =
{
    {"stone.png"_f0, 0},
    {"cobblestone.png"_f0, 1},
    {"dirt.png"_f0, 2},
    {"grass_top.png"_f0, 3},
    {"grass_side.png"_f0, 4},
    {"gravel.png"_f0, 5},
    {"sandstone_bottom.png"_f0, 6},
    {"sandstone_top.png"_f0, 7},
    {"sandstone.png"_f0, 8},
    {"sand.png"_f0, 9},
    {"coal_ore.png"_f0, 10},
    {"coal_block.png"_f0, 11},
    {"iron_ore.png"_f0, 12},
    {"iron_block.png"_f0, 13},
    {"gold_ore.png"_f0, 14},
    {"gold_block.png"_f0, 15},
    {"diamond_ore.png"_f0, 16},
    {"diamond_block.png"_f0, 17},
    {"emerald_ore.png"_f0, 18},
    {"emerald_block.png"_f0, 19},
    {"redstone_ore.png"_f0, 20},
    {"redstone_block.png"_f0, 21},
    {"lapis_ore.png"_f0, 22},
    {"lapis_block.png"_f0, 23},
    {"brick.png"_f0, 24},
    {"stone_brick.png"_f0, 25},
    {"crafting_table_front.png"_f0, 26},
    {"crafting_table_side.png"_f0, 27},
    {"crafting_table_top.png"_f0, 28},
    {"oak_planks.png"_f0, 29},
    {"furnace_front.png"_f0, 30},
    {"furnace_yaxis.png"_f0, 31},
    {"furnace_side.png"_f0, 32},
    {"barrel_top.png"_f0, 33},
    {"barrel_bottom.png"_f0, 34},
    {"barrel_side.png"_f0, 35},
    {"bookshelf.png"_f0, 36},
    {"tnt_top.png"_f0, 37},
    {"tnt_side.png"_f0, 38},
    {"tnt_bottom.png"_f0, 39},
    {"piston_top.png"_f0, 40},
    {"piston_side.png"_f0, 41},
    {"piston_bottom.png"_f0, 42},
    {"sponge.png"_f0, 43},
    {"acacia_log_yaxis.png"_f0, 44},
    {"acacia_log.png"_f0, 45},
    {"acacia_planks.png"_f0, 46},
    {"birch_log_yaxis.png"_f0, 47},
    {"birch_log.png"_f0, 48},
    {"birch_planks.png"_f0, 49},
    {"dark_oak_log_yaxis.png"_f0, 50},
    {"dark_oak_log.png"_f0, 51},
    {"dark_oak_planks.png"_f0, 52},
    {"jungle_log_yaxis.png"_f0, 53},
    {"jungle_log.png"_f0, 54},
    {"jungle_planks.png"_f0, 55},
    {"oak_log_yaxis.png"_f0, 56},
    {"oak_log.png"_f0, 57},
    {"spruce_log_yaxis.png"_f0, 58},
    {"spruce_log.png"_f0, 59},
    {"spruce_planks.png"_f0, 60},
    {"pumpkin_yaxis.png"_f0, 61},
    {"pumpkin_side.png"_f0, 62},
    {"melon_yaxis.png"_f0, 63},
    {"melon_side.png"_f0, 64},
    {"hay_block_yaxis.png"_f0, 65},
    {"hay_block_side.png"_f0, 66},
    {"dried_kelp_top.png"_f0, 67},
    {"dried_kelp_bottom.png"_f0, 68},
    {"dried_kelp_side.png"_f0, 69},
    {"honeycomb_block.png"_f0, 70},
    {"bee_nest_front.png"_f0, 71},
    {"bee_nest_top.png"_f0, 72},
    {"bee_nest_bottom.png"_f0, 73},
    {"bee_nest_side.png"_f0, 74},
    {"beehive_front.png"_f0, 75},
    {"beehive_yaxis.png"_f0, 76},
    {"beehive_side.png"_f0, 77},
    {"oak_leaves.png"_f0, 78},
    {"glass.png"_f0, 79},
    {"ice.png"_f0, 80},
    {"honey_top.png"_f0, 81},
    {"honey_bottom.png"_f0, 82},
    {"honey_side.png"_f0, 83},
    {"water.png"_f0, 84},
    {"test_dir_opaque_up.png"_f0, 85},
    {"test_dir_opaque_down.png"_f0, 86},
    {"test_dir_opaque_north.png"_f0, 87},
    {"test_dir_opaque_south.png"_f0, 88},
    {"test_dir_opaque_east.png"_f0, 89},
    {"test_dir_opaque_west.png"_f0, 90},
    {"test_dir_translucent_up.png"_f0, 91},
    {"test_dir_translucent_down.png"_f0, 92},
    {"test_dir_translucent_north.png"_f0, 93},
    {"test_dir_translucent_south.png"_f0, 94},
    {"test_dir_translucent_east.png"_f0, 95},
    {"test_dir_translucent_west.png"_f0, 96},
};

static_global String gl_shader_chunk = R"__Raw--String__(#define Bitmask_1 0x00000001u
#define Bitmask_2 0x00000003u
#define Bitmask_3 0x00000007u
#define Bitmask_4 0x0000000fu
#define Bitmask_5 0x0000001fu
#define Bitmask_6 0x0000003fu
#define Bitmask_7 0x0000007fu
#define Bitmask_8 0x000000ffu
#define Bitmask_9 0x000001ffu
#define Bitmask_10 0x000003ffu
#define Bitmask_11 0x000007ffu
#define Bitmask_12 0x00000fffu
#define Bitmask_13 0x00001fffu
#define Bitmask_14 0x00003fffu
#define Bitmask_15 0x00007fffu
#define Bitmask_16 0x0000ffffu
#define Bit_1  0x00000001u
#define Bit_2  0x00000002u
#define Bit_3  0x00000004u
#define Bit_4  0x00000008u
#define Bit_5  0x00000010u
#define Bit_6  0x00000020u
#define Bit_7  0x00000040u
#define Bit_8  0x00000080u


#if defined(VERTEX_SHADER)

layout (location = 0) in uint v_packed;

uniform vec3 uni_mesh_translate;
uniform mat4 uni_world_transform;
uniform usamplerBuffer uni_face_array;

smooth out vec3 f_voxel_p;
smooth out float f_ambient;
flat out uint f_face_data;

void main()
{
    int face_index = gl_VertexID >> 2;
    uvec4 face_data = texelFetch(uni_face_array, face_index);
    
    
    uint packed_pos = (face_data.y << 8u | face_data.x);
    vec3 pos;
    pos.x = float((packed_pos       ) & Bitmask_5);
    pos.y = float((packed_pos >>  5u) & Bitmask_5);
    pos.z = float((packed_pos >> 10u) & Bitmask_5);
    
    vec3 pos_offset;
    pos_offset.x = float((v_packed     ) & 1u);
    pos_offset.y = float((v_packed >> 1u) & 1u);
    pos_offset.z = float((v_packed >> 2u) & 1u);
    pos += pos_offset;
    
    f_ambient = 1.0 - ((float((v_packed >> 3u) & Bitmask_2) / (3.0)) * 0.53);
    
    f_voxel_p = pos + uni_mesh_translate;
    gl_Position = vec4(f_voxel_p, 1.f) * uni_world_transform;
    
    f_face_data = ((face_data.w << 9u) |
                   (face_data.z << 1u) |
                   (face_data.y >> 7u));
}



#elif defined(FRAGMENT_SHADER)

smooth in vec3 f_voxel_p;
smooth in float f_ambient;
flat in uint f_face_data;

uniform sampler2DArray uni_tex;
uniform vec3 uni_fog_translate;
uniform float uni_mesh_age_t;

out vec4 out_color;


const vec3 tex_perpendiculars[12] = vec3[12]
(
    // x axis (S)
    vec3(-1, 0, 0), // up
    vec3(-1, 0, 0), // down
    vec3( 1, 0, 0), // N
    vec3(-1, 0, 0), // S
    vec3( 0, 0,-1), // E
    vec3( 0, 0, 1), // W
    
    // y axis (T)
    vec3( 0, 0, 1), // up
    vec3( 0, 0,-1), // down
    vec3( 0, 1, 0), // N
    vec3( 0, 1, 0), // S
    vec3( 0, 1, 0), // E
    vec3( 0, 1, 0) // W
);

const float face_colors[6] = float[6]
(
    1.f,   // up
    0.71f, // down
    0.81f, // N
    0.81f, // S
    0.91f, // E
    0.91f // W
);


//const vec3 fog_color = vec3(0.53, 0.1, 0.1); // test red
const vec3 fog_color = vec3(0.53, 0.63, 0.75);


vec3 compute_fog(vec3 color, vec3 rel_p)
{
    float f = dot(rel_p.xz, rel_p.xz)*0.0000047;
    f = smoothstep(0.6, 1.0, f);
    return mix(color, fog_color, f);
}


void main()
{
    uint normal_index = ((f_face_data >> 14u) & Bitmask_3);
    vec3 texgen_s = tex_perpendiculars[normal_index];
    vec3 texgen_t = tex_perpendiculars[normal_index + 6u];
    
    vec2 tex_coord;
    tex_coord.s = dot(f_voxel_p, texgen_s);
    tex_coord.t = dot(f_voxel_p, texgen_t);
    
    uint tex_id = f_face_data & Bitmask_14;
    vec4 tex_color = texture(uni_tex, vec3(tex_coord, float(tex_id)));
    out_color = tex_color;
    
    
    float light = face_colors[normal_index];
    out_color.rgb *= light;
    out_color.rgb *= f_ambient;
    
    out_color.rgb = mix(fog_color, out_color.rgb, uni_mesh_age_t);
    
    vec3 world_rel_p = f_voxel_p  - uni_fog_translate;
    out_color.rgb = compute_fog(out_color.rgb, world_rel_p);
}

#endif)__Raw--String__"_f0;

static_global String gl_shader_ui = R"__Raw--String__(#define Bitmask_14 0x00003fffu

#if defined(VERTEX_SHADER)

layout (location = 0) in vec3 vert_pos;
layout (location = 1) in vec2 vert_tex_uv;
layout (location = 2) in uint vert_packed;

uniform mat4 uni_transform;

out vec2 frag_tex_uv;
out vec4 frag_color;
flat out uint frag_texid;

void main()
{
    gl_Position = vec4(vert_pos, 1.f) * uni_transform;
    frag_tex_uv = vert_tex_uv;
    
    vec3 light = vec3(float((vert_packed & 0xF00u) >> 8u),
                      float((vert_packed & 0x0F0u) >> 4u),
                      float((vert_packed & 0x00Fu)      )) / 15.0;
    
    frag_texid = (vert_packed >> 12u) & Bitmask_14;
    frag_color = vec4(light, 1.0);
}



#elif defined(FRAGMENT_SHADER)

in vec2 frag_tex_uv;
in vec4 frag_color;
flat in uint frag_texid;

uniform sampler2DArray uni_tex_array;
uniform sampler2D uni_tex_atlas;

out vec4 out_color;

void main()
{
    vec4 tex_color;
    if (frag_texid == Bitmask_14)
    {
        tex_color = texture(uni_tex_atlas, frag_tex_uv);
    }
    else
    {
        tex_color = texture(uni_tex_array, vec3(frag_tex_uv, float(frag_texid)));
    }
    
    out_color = tex_color * frag_color;
}

#endif)__Raw--String__"_f0;

static_global String gl_shader_debug = R"__Raw--String__(#if defined(VERTEX_SHADER)

layout (location = 0) in vec3 vert_pos;
layout (location = 1) in uint vert_rgb18_texid8_texpos2;

uniform mat4 uni_transform;

out vec3 frag_tex;
out vec3 frag_color;

void main()
{
    gl_Position = vec4(vert_pos, 1.f) * uni_transform;
    
    frag_tex.x = float((vert_rgb18_texid8_texpos2      ) & 1u);
    frag_tex.y = float((vert_rgb18_texid8_texpos2 >> 1u) & 1u);
    frag_tex.z = float((vert_rgb18_texid8_texpos2 >> 2u) & 0xFFu);
    
    frag_color.r = float((vert_rgb18_texid8_texpos2 >> 22u) & 0x3Fu);
    frag_color.g = float((vert_rgb18_texid8_texpos2 >> 16u) & 0x3Fu);
    frag_color.b = float((vert_rgb18_texid8_texpos2 >> 10u) & 0x3Fu);
    frag_color /= 63.0;
}



#elif defined(FRAGMENT_SHADER)

in vec3 frag_tex;
in vec3 frag_color;

uniform sampler2DArray uni_tex;

out vec4 out_color;

void main()
{
    vec4 tex_color = texture(uni_tex, frag_tex);
    out_color = tex_color * vec4(frag_color, 1.0);
    //out_color = tex_color;
}

#endif)__Raw--String__"_f0;

static_global String gl_shader_line = R"__Raw--String__(#if defined(VERTEX_SHADER)

layout (location = 0) in vec3 vert_pos;
layout (location = 1) in vec3 vert_color;

uniform mat4 uni_transform;

out vec3 frag_color;

void main()
{
    gl_Position = vec4(vert_pos, 1.f) * uni_transform;
    frag_color = vert_color;
}



#elif defined(FRAGMENT_SHADER)

in vec3 frag_color;
out vec4 out_color;

void main()
{
    out_color = vec4(frag_color, 1.0);
}

#endif)__Raw--String__"_f0;

