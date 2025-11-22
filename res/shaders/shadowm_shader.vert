#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec3 aBitangent;
layout (location = 4) in vec2 aTexCoords;

uniform mat4 uModel;
uniform mat4 uDirLightViewProj;

void main()
{
    // note that we read the multiplication from right to left
    gl_Position = uDirLightViewProj * uModel * vec4(aPos, 1.0);
}