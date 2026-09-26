#version 450 core
#include "common/frame.glsl"

layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

void main()
{
    TexCoords = aPos;
    // Dropping the translation keeps the sky infinitely far away
    vec4 pos = frame.projection * mat4(mat3(frame.view)) * vec4(aPos, 1.0);
    // z = w puts the sky on the far plane after the perspective divide
    gl_Position = pos.xyww;
}
