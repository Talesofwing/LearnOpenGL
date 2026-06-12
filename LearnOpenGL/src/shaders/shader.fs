#version 330 core

in vec3 ourPosition;
in vec3 ourColor;

out vec4 FragColor;

void main()
{
	FragColor = vec4(ourPosition, 1.0);
};