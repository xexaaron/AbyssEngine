#version 450 core
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_debug_printf : enable

layout(location = 0) in vec3 v_color;
layout(location = 1) in flat float v_tex_idx;

layout (set = 1, binding = BINDLESS_TEXTURE_BINDING) uniform sampler2D textures[];

layout(location = 0) out vec4 out_color;

void main() {
    int tex_idx = int(v_tex_idx);
    out_color = vec4(v_color, 1.0) * texture(textures[1], vec2(0.0, 0.0));
    debugPrintfEXT("Fragment: tex_idx = { float: %f, int: %i }\n", v_tex_idx, tex_idx);
}