# Shader interface

How data goes from C++ to the shaders.

## Uniform blocks

Data shared by many shaders lives in uniform buffers, declared once in
`res/shaders/common/` and included where needed:

| Block | Content | Updated |
|---|---|---|
| `FrameBlock` | camera matrices, position, planes | once per frame |
| `LightsBlock` | sun, point and spot lights | once per frame |
| `ShadowBlock` | cascade matrices, splits, bias settings | once per frame, by the shadow pass |

Each block has a fixed binding point. The renderer uploads it with
`glNamedBufferSubData` and binds it with `glBindBufferBase`, then every
program that declares the block reads the same memory. Nothing is set per
program.

Before, the same values were set with one `glUniform*` call per value and per
program: 39 calls for the lights alone, repeated for every shader using
them, each one going through the driver's validation. Now it is one copy per
block per frame, the light count is no longer fixed in the shader, and a new
shader only has to `#include` a block to see the data.

Per-draw values (model matrix, material factors) stay plain uniforms. They
change between draws, where a single `glProgramUniform*` is the cheapest
option until instancing or multi-draw moves them to storage buffers.

## Layout

The blocks use `std140`, whose alignment rules differ from C++ (a `vec3` takes
16 bytes, array elements are padded to 16 bytes). To avoid manual padding,
every member is a `vec4`, `ivec4` or `mat4`, which has the same layout on
both sides. The C++ mirrors in `src/rendering/ShaderInterface.h` have
`static_assert`s on their sizes and offsets.

## Shared constants

Binding points, texture units and array sizes are defined once, in
`ShaderInterface.h`. The shader loader inserts them as `#define`s after the
`#version` line of every stage, so the GLSL code uses `FRAME_BLOCK`,
`ALBEDO_UNIT` or `MAX_POINT_LIGHTS` and can never disagree with the C++ side.

Samplers get their unit with `layout(binding = ALBEDO_UNIT)`, so no
`glUniform1i` is needed to connect textures either.

## Includes

GLSL has no `#include`. The loader expands `#include "path"` itself, relative
to the including file, once per file and stage. `#line` directives keep the
driver's error messages pointing at the right file and line: errors read
`<file index>:<line>` and the file table is printed with them.
