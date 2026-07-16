#version 330 core

in VS_OUT {
    vec3 WorldPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform sampler2D texture1;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform bool blinn;
uniform bool isGamma;

out vec4 FragColor;

vec3 BlinnPhong(vec3 normal, vec3 worldPos) {
    // ambient
    //vec3 ambient = 0.05 * lightColor;

    // diffuse
    vec3 lightDir = normalize(lightPos - worldPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;

    // specular
    vec3 viewDir = normalize(viewPos - worldPos);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = spec * lightColor;

    // simple attnuation
    float max_distance = 1.5;
    float distance = length(lightPos - worldPos);
    float attenuation = 1.0 / (isGamma ? distance * distance : distance);

    diffuse *= attenuation;
    specular *= attenuation;

    return diffuse + specular;
}

void main()
{
    vec3 color = texture(texture1, fs_in.TexCoords).rgb;
    vec3 lighting = BlinnPhong(normalize(fs_in.Normal), fs_in.WorldPos);
    color *= lighting;
    if (isGamma)
        color = pow(color, vec3(1.0 / 2.2));
    
    FragColor = vec4(color, 1.0);
}