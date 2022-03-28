#if defined(VERTEX_SHADER)

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

#endif