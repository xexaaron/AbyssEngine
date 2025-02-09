#version 450 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;
layout(location = 2) in int in_tex_idx;

layout(std140, binding = 0) uniform Camera {
    mat4 view_proj;
};

layout(location = 0) out vec3 out_color;
layout(location = 1) out flat int out_tex_idx;

void main() {
    gl_Position = view_proj * vec4(in_position, 1.0);
    out_color = in_color;
    out_tex_idx = in_tex_idx;
}