#version 450 core
#extension GL_EXT_debug_printf : enable

layout(location = 0) in vec3  a_position;
layout(location = 1) in vec4  a_color;
layout(location = 2) in vec3  a_texinfo;
layout(location = 3) in vec2  a_uvs;


layout(std140, binding = 0) uniform Camera {
    mat4 view_proj;
};

layout(location = 0) out vec4 v_color;
layout(location = 1) out vec3 v_texinfo;
layout(location = 2) out vec2 v_uvs;

void main() {
    gl_Position = view_proj * vec4(a_position, 1.0);
    v_color     = a_color;
    v_texinfo   = a_texinfo;
    v_uvs       = a_uvs;
    // debugPrintfEXT("VERT: [v_color] = (%f, %f, %f, %f)\n", EXPAND_VEC4(v_color));
}