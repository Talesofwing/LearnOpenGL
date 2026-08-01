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

out VS_OUT {
    vec3 WorldPos;
    vec2 TexCoords;
    vec3 Normal;
    mat3 TBN;
} vs_out;

void main()
{
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 N = normalize(normalMatrix * aNormal);
    vec3 T = normalize(mat3(model) * aTangent);
    T = normalize(T - N * dot(N, T));

    float handedness = dot(cross(aNormal, aTangent), aBitangent) < 0.0 ? -1.0 : 1.0;
    vec3 B = normalize(handedness * cross(N, T));

    vs_out.WorldPos = vec3(model * vec4(aPos, 1.0));
    vs_out.TexCoords = aTexCoords;
    vs_out.Normal = N;
    vs_out.TBN = mat3(T, B, N);

    gl_Position = projection * view * vec4(vs_out.WorldPos, 1.0);
}
