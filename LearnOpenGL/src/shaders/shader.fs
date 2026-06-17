#version 330 core

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
}; 
  
struct Light {
    vec3 position;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Light light;  

uniform Material material;

in vec3 WorldPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 lightColor;

uniform vec3 lightPos;
uniform vec3 viewPos;

out vec4 FragColor;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - WorldPos);
    vec3 viewDir = normalize(viewPos - WorldPos);
    vec3 reflectDir = reflect(-lightDir, norm);

    vec3 diffuseTex = vec3(texture(material.diffuse, TexCoords));
    vec3 specularTex = vec3(texture(material.specular, TexCoords));

    // ambient
    vec3 ambient = light.ambient * diffuseTex;
    // diffuse
    float diffuseFactor = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diffuseFactor * diffuseTex;
    // specular
    float specularFactor = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * specularFactor * specularTex;

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}