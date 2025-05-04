# Shaders

## Compiler

- spirv_cross
- SPIR-V 1.6

## Compile definitions

These compile definitions are baked into any shader that is compiled by the program at runtime.

```glsl
#define GLSL_VERSION 450
#define EXPAND_VEC4(vec) vec.r, vec.g, vec.b, vec.a
#define EXPAND_VEC3(vec) vec.x, vec.y, vec.z
#define MAX_TEXTURE_SLOTS DeviceManager::max_texture_slots()
#define BINDLESS_TEXTURE_BINDING 10
```

## Accessing Resources

### Textures

- **Required Extensions**

```glsl
#extension GL_EXT_nonuniform_qualifier : enable
```

- **Uniforms**

```glsl
layout(set = 1, binding = BINDLESS_TEXTURE_BINDING) uniform sampler2D textures[];
```

- **Indexing**

```glsl
int  tex_idx = int(nonuniformEXT(v_texinfo.z));
vec4 sampler = textures(tex_idx, v_texinfo.xy);
```
