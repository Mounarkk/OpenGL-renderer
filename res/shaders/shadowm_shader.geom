#version 330 core
#define MAX_CASCADES 8

layout (triangles) in;
layout (triangle_strip, max_vertices = 24) out; // 3 * MAX_CASCADES

in vec2 vTexCoords[];
out vec2 gTexCoords;

uniform mat4 uLightSpaceMatrices[MAX_CASCADES];
uniform int uCascadeCount;

// Broadcasts each triangle to every cascade of the layered framebuffer.
void main()
{
    for (int layer = 0; layer < uCascadeCount; ++layer)
    {
        for (int i = 0; i < 3; ++i)
        {
            // Outputs are undefined after EmitVertex, so gl_Layer is written
            // for every vertex.
            gl_Layer = layer;
            gTexCoords = vTexCoords[i];
            gl_Position = uLightSpaceMatrices[layer] * gl_in[i].gl_Position;
            EmitVertex();
        }
        EndPrimitive();
    }
}
