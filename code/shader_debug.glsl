#if defined(VERTEX_SHADER)

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

#endif