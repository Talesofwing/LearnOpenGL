#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform vec3 NormalColor;

void main()
{
    FragColor = vec4(NormalColor, 1.0);
}