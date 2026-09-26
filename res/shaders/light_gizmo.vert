#version 450 core
#include "common/frame.glsl"

layout (location = 0) in vec3 aPos;

uniform mat4 uModel;

void main()
{
    gl_Position = frame.viewProjection * uModel * vec4(aPos, 1.0);
}
