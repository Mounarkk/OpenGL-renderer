#version 450 core
out vec4 FragColor;

in vec2 TexCoords;

layout (binding = SCREEN_UNIT) uniform sampler2D uScreenTexture;

uniform float uExposure;   // linear multiplier, 2^stops
uniform bool uToneMapping;

// Lighting is computed in linear space, the window expects sRGB.
const float GAMMA = 2.2;

// Fit of the ACES filmic curve (Narkowicz 2015). Compresses any HDR value
// into [0, 1] with a soft shoulder instead of clipping, and adds a little
// contrast in the midtones.
vec3 acesFilmic(vec3 x)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main()
{
    vec3 hdr = texture(uScreenTexture, TexCoords).rgb * uExposure;
    vec3 color = uToneMapping ? acesFilmic(hdr) : clamp(hdr, 0.0, 1.0);
    FragColor = vec4(pow(color, vec3(1.0 / GAMMA)), 1.0);
}
