#version 450 core
#include "common/shadows.glsl"

// One invocation per cascade (geometry shader instancing), each writing the
// triangle into its own layer of the shadow map array.
layout (triangles, invocations = CASCADE_COUNT) in;
layout (triangle_strip, max_vertices = 3) out;

in vec2 vTexCoords[];
out vec2 gTexCoords;

// Bit i is set when the mesh overlaps cascade i (CPU culling)
uniform uint uCascadeMask;

void main()
{
    int layer = gl_InvocationID;
    if ((uCascadeMask & (1u << layer)) == 0u)
        return;

    for (int i = 0; i < 3; ++i)
    {
        gl_Layer = layer;
        gTexCoords = vTexCoords[i];
        gl_Position = shadows.lightSpaceMatrices[layer] * gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}
