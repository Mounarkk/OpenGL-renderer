#version 450 core
#include "common/frame.glsl"

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec3 aBitangent;
layout (location = 4) in vec2 aTexCoords;

out VS_OUT {
    vec3 fragPos;
    vec2 texCoords;
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
} vs_out;

uniform mat4 uModel;

void main()
{
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    // Inverse transpose keeps normals perpendicular under non-uniform scaling
    mat3 normalMatrix = transpose(inverse(mat3(uModel)));

    vs_out.fragPos = worldPos.xyz;
    vs_out.texCoords = aTexCoords;
    vs_out.normal = normalMatrix * aNormal;
    vs_out.tangent = mat3(uModel) * aTangent;
    vs_out.bitangent = mat3(uModel) * aBitangent;

    gl_Position = frame.viewProjection * worldPos;
}
