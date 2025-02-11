#version 450 core
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_debug_printf : enable
#define EXPAND_VEC4(vec) vec.r, vec.g, vec.b, vec.a
#define EXPAND_VEC3(vec) vec.x, vec.y, vec.z

layout(location = 0) in vec4 v_color;
layout(location = 1) in vec3 v_texinfo;

layout(set = 1, binding = BINDLESS_TEXTURE_BINDING) uniform sampler2D textures[];

layout(location = 0) out vec4 out_color;

void main() {
    int tex_idx = int(nonuniformEXT(v_texinfo.z));
    out_color = texture(textures[tex_idx], v_texinfo.xy) * v_color;
    //debugPrintfEXT("FRAG: [v_color] = (%f, %f, %f, %f)", EXPAND_VEC4(v_color));
    //debugPrintfEXT("FRAG: [v_texinfo] = texcoord(%f, %f), tex_idx(%f) \n", EXPAND_VEC3(v_texinfo));
}