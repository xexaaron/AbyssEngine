#version 450 core
#extension GL_EXT_debug_printf : enable


layout(location = 0) in vec3  a_position;
layout(location = 1) in vec3  a_color;
layout(location = 2) in vec3  a_texinfo;


layout(std140, binding = 0) uniform Camera {
    mat4 view_proj;
};

layout(location = 0) out vec4 v_color;
layout(location = 1) out vec3 v_texinfo;

void main() {
    gl_Position = view_proj * vec4(a_position, 1.0);
    v_color     = vec4(a_color, 1.0);
    v_texinfo   = a_texinfo;
    // debugPrintfEXT("VERT: [v_color] = (%f, %f, %f, %f)\n", EXPAND_VEC4(v_color));
}