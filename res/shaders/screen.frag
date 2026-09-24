#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

// Lighting is computed in linear space, the window expects sRGB.
const float GAMMA = 2.2;

void main()
{
    vec3 color = clamp(texture(screenTexture, TexCoords).rgb, 0.0, 1.0);
    FragColor = vec4(pow(color, vec3(1.0 / GAMMA)), 1.0);
}
