// Monte Carlo helpers for the image based lighting precomputations.
// Needs math.glsl.

// Point i of a Hammersley set of the given size: points spread evenly over
// the unit square, which makes integrals converge much faster than random
// points do
vec2 hammersley(uint i, uint count)
{
    // Radical inverse in base 2: the bits of i mirrored after the point
    float radicalInverse = float(bitfieldReverse(i)) * 2.3283064365386963e-10;
    return vec2(float(i) / float(count), radicalInverse);
}

// Half vector around n, distributed like the GGX lobe: its probability
// density is D(h) * dot(n, h). Samples go where the BRDF is large.
vec3 importanceSampleGGX(vec2 xi, vec3 n, float alpha)
{
    float a2 = alpha * alpha;
    float phi = 2.0 * PI * xi.x;
    float cosTheta = sqrt((1.0 - xi.y) / (1.0 + (a2 - 1.0) * xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    vec3 up = abs(n.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, n));
    vec3 bitangent = cross(n, tangent);
    return normalize(tangent * (sinTheta * cos(phi)) +
                     bitangent * (sinTheta * sin(phi)) + n * cosTheta);
}
