#version 330 core

in VS_OUT {
    vec3 WorldPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 ClipPosLightSpace;
} fs_in;

uniform sampler2D diffuseTexture;
uniform samplerCube shadowCubemap;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform bool gammaCorrectionEnabled;
uniform float far_plane;

out vec4 FragColor;

float ShadowCalculation() {
    vec3 lightToWorld = fs_in.WorldPos - lightPos;
    
    vec3 sampleOffsetDirections[20] = vec3[] 
    (
        vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
        vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
        vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
        vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
        vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
    ); 

    float bias = 0.05;
    float currentDepth = length(lightToWorld) - bias;
    float viewDistance = length(viewPos - fs_in.WorldPos);
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;
    float shadow = 0.0;
    for (int i = 0; i < sampleOffsetDirections.length(); ++i) {
        float closestDepth = texture(shadowCubemap, lightToWorld + sampleOffsetDirections[i] * diskRadius).r * far_plane;
        if (currentDepth > closestDepth)
            shadow += 1.0;
    }

    return shadow / sampleOffsetDirections.length();
}

vec3 BlinnPhong() {
    vec3 n = normalize(fs_in.Normal);

    // ambient
    vec3 ambient = 0.3 * lightColor;

    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.WorldPos);
    float diff = max(dot(lightDir, n), 0.0);
    vec3 diffuse = diff * lightColor;

    // specular
    vec3 viewDir = normalize(viewPos - fs_in.WorldPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(n, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;

    // attenuation
    float distance = length(lightPos - fs_in.WorldPos);
    float attenuation = 1 / (gammaCorrectionEnabled ? distance * distance : distance);

    // shadow
    float shadow = ShadowCalculation();

    return ambient + (1.0 - shadow) * (diffuse + specular) * attenuation;
}

void main()
{
    vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 lighting = BlinnPhong();
    color *= lighting;

    if (gammaCorrectionEnabled)
        color = pow(color, vec3(1.0 / 2.2));
    
    FragColor = vec4(color, 1.0);
}