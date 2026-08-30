#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};
uniform mat4 model;

uniform bool invertedNormals;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vs_out;

void main()
{
    vec4 viewPos = view * model * vec4(aPos, 1.0);
    vs_out.FragPos = viewPos.xyz;
    vs_out.TexCoords = aTexCoords;
    vs_out.Normal = aNormal * inverse(mat3(view * model));
    vs_out.Normal = vs_out.Normal * (invertedNormals ? -1 : 1);

    gl_Position = projection * viewPos;
}
