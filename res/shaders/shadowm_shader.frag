#version 450 core

in vec2 gTexCoords;

layout (binding = ALBEDO_UNIT) uniform sampler2D uAlbedoMap;

// Depth is written by the fixed pipeline. Only cut-out texels are rejected.
void main()
{
    if (texture(uAlbedoMap, gTexCoords).a < 0.5)
        discard;
}
