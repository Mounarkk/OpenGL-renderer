#version 450 core
#include "common/lights.glsl"

out vec4 FragColor;

in vec3 TexCoords;

layout (binding = SKYBOX_UNIT) uniform samplerCube uSkybox;

// Angular radius of the disc, larger than the real sun to be easy to spot
const float SUN_RADIUS = radians(1.5);
const vec3 SUN_COLOR = vec3(1.0, 0.95, 0.8);

void main()
{
    vec3 direction = normalize(TexCoords);
    vec3 color = texture(uSkybox, direction).rgb;

    // The disc is drawn where the directional light comes from
    if (lights.counts.z != 0)
    {
        vec3 toSun = -normalize(lights.sun.direction.xyz);
        float cosAngle = dot(direction, toSun);
        float disc = smoothstep(cos(SUN_RADIUS), cos(SUN_RADIUS * 0.8), cosAngle);
        float glow = pow(max(cosAngle, 0.0), 400.0) * 0.6;
        color = mix(color, SUN_COLOR, clamp(disc + glow, 0.0, 1.0));
    }

    FragColor = vec4(color, 1.0);
}
