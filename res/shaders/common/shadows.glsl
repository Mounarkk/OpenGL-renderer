// Cascaded shadow map data, mirrors ShaderInterface::ShadowUniforms.
layout (std140, binding = SHADOW_BLOCK) uniform ShadowBlock {
    mat4 lightSpaceMatrices[CASCADE_COUNT];
    vec4 cascades[CASCADE_COUNT]; // x far plane, y texel size, z texel depth
    vec4 bias;                    // x constant, y slope, z normal offset
    ivec4 flags;                  // x enabled, y PCF, z show cascades
} shadows;
