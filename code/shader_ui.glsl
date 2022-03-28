#define Bitmask_14 0x00003fff

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
    
    vec3 light = vec3(float((vert_packed & 0xF00) >> 8),
                      float((vert_packed & 0x0F0) >> 4),
                      float((vert_packed & 0x00F)      )) / 15.0;
    
    frag_texid = (vert_packed >> 12) & Bitmask_14;
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

#endif