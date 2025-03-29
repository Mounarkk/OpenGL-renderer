#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;

uniform mat4 uModel;
uniform mat4 uViewProj;

void main()
{
    // note that we read the multiplication from right to left
    gl_Position = uViewProj * uModel * vec4(aPos, 1.0);
    TexCoords = aTexCoords;
    FragPos = vec3(uModel * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(uModel))) * aNormal;
}