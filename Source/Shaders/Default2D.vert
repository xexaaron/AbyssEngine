#version 450 core
#extension GL_EXT_debug_printf : enable

layout(location = 0) in vec3  a_position;
layout(location = 1) in vec3  a_color;
layout(location = 2) in float a_tex_idx;

layout(location = 0) out vec3 v_color;
layout(location = 1) out flat float v_tex_idx;

void main() {
    gl_Position = vec4(a_position, 1.0);
    v_color   = a_color;
    v_tex_idx = a_tex_idx;
}