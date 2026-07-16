#version 330 core

in VS_OUT {
    vec3 WorldPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform sampler2D texture1;

out vec4 FragColor;

void main()
{
    //FragColor = texture(texture1, fs_in.TexCoords);
    FragColor = vec4(0.0, 1.0, 0.0, 1.0);
}