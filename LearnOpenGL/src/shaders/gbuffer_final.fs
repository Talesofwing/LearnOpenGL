#version 330 core
  
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

struct Light {
    vec3 Position;
    vec3 Color;
    float Radius;
};
const int NR_LIGHTS = 32;
uniform Light lights[NR_LIGHTS];
uniform vec3 viewPos;

out vec4 FragColor;

void main()
{ 
    // retrieve data from gbuffer
    vec3 WorldPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;
    
    // then calculate lighting as usual
    vec3 lighting  = Diffuse * 0.1; // hard-coded ambient component
    vec3 viewDir  = normalize(viewPos - WorldPos);
    for(int i = 0; i < NR_LIGHTS; ++i)
    {
        float distance = length(lights[i].Position  - WorldPos);
        if (distance < lights[i].Radius) {
            // diffuse
            vec3 lightDir = normalize(lights[i].Position - WorldPos);
            vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * lights[i].Color;
            // specular
            vec3 halfwayDir = normalize(lightDir + viewDir);  
            float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
            vec3 specular = lights[i].Color * spec * Specular;

            // attenuation
            float attenuation = 1 / max(distance * distance, 0.0001);

            lighting += (diffuse + specular) * attenuation;
        }
    }

    FragColor = vec4(lighting, 1.0);
}