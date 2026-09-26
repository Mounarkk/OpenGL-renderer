// Per-frame camera data, mirrors ShaderInterface::FrameUniforms.
layout (std140, binding = FRAME_BLOCK) uniform FrameBlock {
    mat4 view;
    mat4 projection;
    mat4 viewProjection;
    vec4 cameraPosition; // xyz position
    vec4 viewport;       // width, height, near plane, far plane
} frame;
