#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec3 aBitangent;
layout (location = 4) in vec2 aTexCoords;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;
out mat3 TBN;

uniform mat4 uModel;
uniform mat4 uViewProj;

void main()
{
    // note that we read the multiplication from right to left
    gl_Position = uViewProj * uModel * vec4(aPos, 1.0);
    TexCoords = aTexCoords;
    FragPos = vec3(uModel * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(uModel))) * aNormal;

    // TBN matrix
    vec3 T = normalize(vec3(uModel * vec4(aTangent, 0)));
    vec3 B = normalize(vec3(uModel * vec4(aBitangent, 0)));
    vec3 N = normalize(vec3(uModel * vec4(aNormal, 0)));
    TBN = mat3(T, B, N);
}