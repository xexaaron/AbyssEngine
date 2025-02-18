#version 450 core
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_debug_printf : enable

layout(location = 0) in vec4 v_color;
layout(location = 1) in vec3 v_texinfo;
layout(location = 2) in vec2 v_uvs;

layout(set = 1, binding = BINDLESS_TEXTURE_BINDING) uniform sampler2D textures[];

layout(location = 0) out vec4 out_color;

void main() {
    int tex_idx  = int(nonuniformEXT(v_texinfo.z));
    vec4 sampled = texture(textures[tex_idx], v_texinfo.xy * v_uvs); 
    
    // Use the red channel of the texture as the alpha value
    float alpha = sampled.r;
    
    // Preserve the color but apply the sampled alpha
    out_color = vec4(v_color.rgb, v_color.a * alpha);
}