#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

uniform mat4 model;

uniform vec3 lightPos;
uniform vec3 viewPos;

out VS_OUT {
    vec3 WorldPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} vs_out;

void main()
{
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 N = normalize(normalMatrix * aNormal);
    vec3 T = normalize(mat3(model) * aTangent);
    T = normalize(T - N * dot(N, T));
    vec3 B = normalize(cross(N, T));

    mat3 inverse_TBN = transpose(mat3(T, B, N));

    vs_out.WorldPos = vec3(model * vec4(aPos, 1.0));
    vs_out.TexCoords = aTexCoords;
    vs_out.TangentLightPos = inverse_TBN * lightPos;
    vs_out.TangentViewPos = inverse_TBN * viewPos;
    vs_out.TangentFragPos = inverse_TBN * vs_out.WorldPos;

    gl_Position = projection * view * vec4(vs_out.WorldPos, 1.0);
}
