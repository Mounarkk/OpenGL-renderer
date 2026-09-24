#version 330 core
out vec4 FragColor;

// Unlit marker showing where a light is and which color it emits.
uniform vec3 uColor;

void main()
{
    FragColor = vec4(uColor, 1.0);
}
