#define Bitmask_1 0x00000001u
#define Bitmask_2 0x00000003u
#define Bitmask_3 0x00000007u
#define Bitmask_4 0x0000000fu
#define Bitmask_5 0x0000001fu
#define Bitmask_6 0x0000003fu
#define Bitmask_7 0x0000007fu
#define Bitmask_8 0x000000ffu
#define Bitmask_9 0x000001ff
#define Bitmask_10 0x000003ff
#define Bitmask_11 0x000007ff
#define Bitmask_12 0x00000fff
#define Bitmask_13 0x00001fff
#define Bitmask_14 0x00003fff
#define Bitmask_15 0x00007fff
#define Bitmask_16 0x0000ffff
#define Bit_1  0x00000001
#define Bit_2  0x00000002
#define Bit_3  0x00000004
#define Bit_4  0x00000008
#define Bit_5  0x00000010
#define Bit_6  0x00000020
#define Bit_7  0x00000040
#define Bit_8  0x00000080


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
    
    
    uint packed_pos = (face_data.y << 8 | face_data.x);
    vec3 pos;
    pos.x = float((packed_pos.x    ) & Bitmask_5);
    pos.y = float((packed_pos >>  5) & Bitmask_5);
    pos.z = float((packed_pos >> 10) & Bitmask_5);
    
    vec3 pos_offset;
    pos_offset.x = float((v_packed     ) & 1);
    pos_offset.y = float((v_packed >> 1) & 1);
    pos_offset.z = float((v_packed >> 2) & 1);
    pos += pos_offset;
    
    f_ambient = 1.0 - ((float((v_packed >> 3) & Bitmask_2) / (3.0)) * 0.53);
    
    f_voxel_p = pos + uni_mesh_translate;
    gl_Position = vec4(f_voxel_p, 1.f) * uni_world_transform;
    
    f_face_data = ((face_data.w << 9) |
                   (face_data.z << 1) |
                   (face_data.y >> 7));
}



#elif defined(FRAGMENT_SHADER)

smooth in vec3 f_voxel_p;
smooth in float f_ambient;
flat in uint f_face_data;

uniform sampler2DArray uni_tex;
uniform vec3 uni_fog_translate;
uniform float uni_mesh_age_t;

out vec4 out_color;


const vec3 tex_perpendiculars[12] =
{
    // x axis (S)
    {-1, 0, 0}, // up
    {-1, 0, 0}, // down
    { 1, 0, 0}, // N
    {-1, 0, 0}, // S
    { 0, 0,-1}, // E
    { 0, 0, 1}, // W
    
    // y axis (T)
    { 0, 0, 1}, // up
    { 0, 0,-1}, // down
    { 0, 1, 0}, // N
    { 0, 1, 0}, // S
    { 0, 1, 0}, // E
    { 0, 1, 0}, // W
};

const float face_colors[6] =
{
    1.f,   // up
    0.71f, // down
    0.81f, // N
    0.81f, // S
    0.91f, // E
    0.91f, // W
};


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
    uint normal_index = ((f_face_data >> 14) & Bitmask_3);
    vec3 texgen_s = tex_perpendiculars[normal_index];
    vec3 texgen_t = tex_perpendiculars[normal_index + 6];
    
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

#endif