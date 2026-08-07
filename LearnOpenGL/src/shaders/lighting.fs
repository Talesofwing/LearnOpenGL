#version 330 core

in VS_OUT {
    vec3 WorldPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

struct Light {
    vec3 Position;
    vec3 Color;
};
uniform Light lights[16];

uniform sampler2D diffuseTexture;
uniform vec3 viewPos;

out vec4 FragColor;

void main()
{
    vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
    // ambient
    vec3 ambient = 0.0 * color;
    // lighting
    vec3 lighting = vec3(0.0);
    for (int i = 0; i < 4; ++i) {
        // diffuse
        vec3 lightDir = normalize(lights[i].Position - fs_in.WorldPos);
        float diff = max(dot(lightDir, normal), 0.0);
        vec3 diffuse = lights[i].Color * diff * color;      
        vec3 result = diffuse;        
        float distance = length(fs_in.WorldPos - lights[i].Position);
        result *= 1.0 / (distance * distance);
        lighting += result;
    }

    FragColor = vec4(ambient + lighting, 1.0);
}