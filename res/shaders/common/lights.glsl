// Lights of the scene, mirrors ShaderInterface::LightUniforms.

struct DirectionalLight {
    vec4 direction; // xyz direction the light travels in
    vec4 radiance;  // rgb color * intensity
    vec4 ambient;   // rgb ambient term of the scene
};

struct PointLight {
    vec4 positionRange; // xyz position, w range
    vec4 radiance;      // rgb color * intensity
};

struct SpotLight {
    vec4 positionRange;     // xyz position, w range
    vec4 directionOuterCos; // xyz direction, w cos(outer half angle)
    vec4 radianceInnerCos;  // rgb color * intensity, w cos(inner half angle)
};

layout (std140, binding = LIGHTS_BLOCK) uniform LightsBlock {
    DirectionalLight sun;
    ivec4 counts; // x point lights, y spot lights, z has sun
    PointLight pointLights[MAX_POINT_LIGHTS];
    SpotLight spotLights[MAX_SPOT_LIGHTS];
} lights;

// Inverse square falloff, windowed so that it reaches exactly zero at the
// range (Karis 2013, "Real Shading in Unreal Engine 4"). The +1 avoids the
// singularity at the light position.
float rangeAttenuation(float distance, float range)
{
    float ratio = distance / range;
    float window = clamp(1.0 - ratio * ratio * ratio * ratio, 0.0, 1.0);
    return window * window / (distance * distance + 1.0);
}
