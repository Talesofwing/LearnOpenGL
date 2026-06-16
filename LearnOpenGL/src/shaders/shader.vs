#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 objectColor;
uniform vec3 lightColor;

uniform vec3 lightPos;
uniform vec3 viewPos;

out vec3 Color;

void main()
{
	vec3 WorldPos = vec3(model * vec4(aPos, 1.0));
	vec3 Normal = aNormal * mat3(transpose(inverse(model)));

	vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - WorldPos);
    vec3 viewDir = normalize(viewPos - WorldPos);
    vec3 reflectDir = reflect(-lightDir, norm);

    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    float diffuseFactor = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diffuseFactor * lightColor;

    float specularStrength = 0.5;
    float specularFactor = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * specularFactor * lightColor;

    Color = (ambient + diffuse + specular) * objectColor;

	gl_Position = projection * view * model * vec4(aPos, 1.0);
};