#version 330 core

in VS_OUT {
    vec3 WorldPos;
    vec2 TexCoords;
    mat3 TBN;
} fs_in;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform vec3 lightPos;
uniform vec3 viewPos;

out vec4 FragColor;

void main()
{
    vec3 tNormal = texture(normalMap, fs_in.TexCoords).rgb * 2.0 - 1.0;
    vec3 normal = normalize(fs_in.TBN * normalize(tNormal));
    vec3 color = texture(diffuseMap, fs_in.TexCoords).rgb;

    vec3 lightDir = normalize(lightPos - fs_in.WorldPos);
    vec3 viewDir = normalize(viewPos - fs_in.WorldPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float diff = max(dot(lightDir, normal), 0.0);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    float attenuation = 1.0 / length(lightPos - fs_in.WorldPos);

    vec3 result = 0.05 * color + attenuation * (2.0 * diff * color + 0.2 * spec);
    FragColor = vec4(result, 1.0);
}
