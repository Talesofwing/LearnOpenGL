#version 330 core

in VS_OUT {
    vec3 WorldPos;
    vec2 TexCoords;
    vec3 Normal;
    mat3 TBN;
} fs_in;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform bool gammaCorrectionEnabled;

out vec4 FragColor;

vec3 BlinnPhong()
{
    vec3 tNormal = texture(texture_normal1, fs_in.TexCoords).rgb * 2.0 - 1.0;
    vec3 n = normalize(fs_in.TBN * normalize(tNormal));

    vec3 diffuseColor = texture(texture_diffuse1, fs_in.TexCoords).rgb;
    vec3 specularColor = texture(texture_specular1, fs_in.TexCoords).rgb;
    vec3 lightDir = normalize(lightPos - fs_in.WorldPos);
    vec3 viewDir = normalize(viewPos - fs_in.WorldPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float diff = max(dot(lightDir, n), 0.0);
    float spec = pow(max(dot(n, halfwayDir), 0.0), 64.0);
    float distanceToLight = length(lightPos - fs_in.WorldPos);
    float attenuation = 1.0 / (gammaCorrectionEnabled ? distanceToLight * distanceToLight : distanceToLight);

    vec3 ambient = 0.3 * diffuseColor;
    vec3 diffuse = diff * diffuseColor;
    vec3 specular = spec * specularColor;
    return ambient + attenuation * (diffuse + specular);
}

void main()
{
    vec3 color = BlinnPhong();
    if (gammaCorrectionEnabled)
        color = pow(color, vec3(1.0 / 2.2));
    FragColor = vec4(color, 1.0);
}
