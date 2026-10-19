// Direction through the center of a texel of a cube map face, for compute
// shaders writing cube maps as image layers. Inverse of the face selection
// table of the OpenGL specification (8.13, "Cube Map Texture Selection"):
// layer 0 to 5 is +X, -X, +Y, -Y, +Z, -Z, and the first row is t = 0.
vec3 cubeMapDirection(ivec3 texel, int size)
{
    vec2 st = (vec2(texel.xy) + 0.5) / float(size) * 2.0 - 1.0;
    vec3 direction;
    switch (texel.z)
    {
    case 0: direction = vec3(1.0, -st.y, -st.x); break;
    case 1: direction = vec3(-1.0, -st.y, st.x); break;
    case 2: direction = vec3(st.x, 1.0, st.y); break;
    case 3: direction = vec3(st.x, -1.0, -st.y); break;
    case 4: direction = vec3(st.x, -st.y, 1.0); break;
    default: direction = vec3(-st.x, -st.y, -1.0); break;
    }
    return normalize(direction);
}
