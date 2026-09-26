#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 4) in vec2 aTexCoords;

out vec2 vTexCoords;

uniform mat4 uModel;

// Positions stay in world space: the geometry shader applies the light
// matrix of each cascade.
void main()
{
    vTexCoords = aTexCoords;
    gl_Position = uModel * vec4(aPos, 1.0);
}
