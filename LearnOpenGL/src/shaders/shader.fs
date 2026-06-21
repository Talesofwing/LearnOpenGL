#version 330 core

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
}; 
uniform Material material;

in vec3 WorldPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;

out vec4 FragColor;

// ===== Directional Light =====
struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform DirLight dirLight;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    // ambient shading
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    // diffuse shading
    vec3 diffuse = light.diffuse * max(dot(normal, lightDir), 0.0) * vec3(texture(material.diffuse, TexCoords));
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 specular = light.specular * pow(max(dot(viewDir, reflectDir), 0.0), material.shininess) * vec3(texture(material.specular, TexCoords));
    // combine results
    return ambient + diffuse + specular;
}
// ===== Directional Light =====

// ===== Point Light =====
struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
#define NR_POINT_LIGHTS 4
uniform PointLight pointLights[NR_POINT_LIGHTS];

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - WorldPos);
    // ambient shading
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    // diffuse shading
    vec3 diffuse = light.diffuse * max(dot(normal, lightDir), 0.0) * vec3(texture(material.diffuse, TexCoords));
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 specular = light.specular * pow(max(dot(viewDir, reflectDir), 0.0), material.shininess) * vec3(texture(material.specular, TexCoords));    
    // attenuation
    float distance = length(light.position - WorldPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance)); 
    // combine results
    return (ambient + diffuse + specular) * attenuation;
}
// ===== Point Light =====

// ===== Spot Light =====
struct SpotLight {
    vec3 position;
    vec3 direction;
    float innerCutOff;
    float outerCutOff;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};
uniform SpotLight spotLight;

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - WorldPos);
    // ambient shading
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    // diffuse shading
    vec3 diffuse = light.diffuse * max(dot(normal, lightDir), 0.0) * vec3(texture(material.diffuse, TexCoords));
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 specular = light.specular * pow(max(dot(viewDir, reflectDir), 0.0), material.shininess) * vec3(texture(material.specular, TexCoords));    
    // attenuation
    float distance = length(light.position - WorldPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance)); 
    // spot light intensity
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.innerCutOff - light.outerCutOff;
    float intensity = smoothstep(light.outerCutOff, light.innerCutOff, theta);
    // combine results
    return (ambient + diffuse + specular) * attenuation * intensity;
}
// ===== Spot Light =====

void main()
{
    // properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - WorldPos);

    // phase 1: Directional light
    vec3 result = CalcDirLight(dirLight, norm, viewDir);
    // phase 2: Point lights
    for (int i = 0; i < NR_POINT_LIGHTS; ++i)
        result += CalcPointLight(pointLights[i], norm, viewDir);
    // phase 3: Spot light
    result += CalcSpotLight(spotLight, norm, viewDir);

    FragColor = vec4(result, 1.0);
}