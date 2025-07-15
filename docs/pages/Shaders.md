# Shaders

## Compiler

- spirv_cross
- SPIR-V 1.6

## Compile definitions

These compile definitions are baked into any shader that is compiled by the program at runtime.

```c title="Compile Definitions" linenums="1"
#define GLSL_VERSION 450
#define EXPAND_VEC4(vec) vec.r, vec.g, vec.b, vec.a
#define EXPAND_VEC3(vec) vec.x, vec.y, vec.z
#define MAX_TEXTURE_SLOTS aby::vk::DeviceManager::max_texture_slots()
#define BINDLESS_TEXTURE_BINDING aby::vk::BINDLESS_TEXTURE_BINDING
```

## Accessing Resources

### Textures

- **Required Extensions**

```glsl title="Extensions" linenums="1"
#extension GL_EXT_nonuniform_qualifier : enable
```

- **Uniforms**

```glsl title="Uniforms" linenums="1"
layout(set = 1, binding = BINDLESS_TEXTURE_BINDING) uniform sampler2D textures[];
```

- **Indexing**

```glsl title
int  tex_idx = int(nonuniformEXT(v_texinfo.z));
vec4 sampler = textures(tex_idx, v_texinfo.xy);
```
