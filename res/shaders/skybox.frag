#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;
uniform vec3 uSunDirection; // towards the sun

// Angular radius of the disc, larger than the real sun to be easy to spot
const float SUN_RADIUS = radians(1.5);
const vec3 SUN_COLOR = vec3(1.0, 0.95, 0.8);

void main()
{
    vec3 direction = normalize(TexCoords);
    vec3 color = texture(skybox, direction).rgb;

    float cosAngle = dot(direction, normalize(uSunDirection));
    float disc = smoothstep(cos(SUN_RADIUS), cos(SUN_RADIUS * 0.8), cosAngle);
    float glow = pow(max(cosAngle, 0.0), 400.0) * 0.6;

    FragColor = vec4(mix(color, SUN_COLOR, clamp(disc + glow, 0.0, 1.0)), 1.0);
}
