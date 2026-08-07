#version 330 core

in VS_OUT {
    vec3 WorldPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D depthMap;

uniform float height_scale;

out vec4 FragColor;

vec2 ParallaxMapping(vec3 viewDir) {
    const float minLayers = 8.0;
    const float maxLayers = 32;
    float numLayers = mix(maxLayers, minLayers, max(dot(vec3(0.0, 0.0, 1.0), viewDir), 0.0));
    float layerDepth = 1.0 / numLayers;
    float currentLayerDepth = 0.0;
    vec2 P = viewDir.xy * height_scale;
    vec2 deltaTexCoords = P / numLayers;

    vec2 currentTexCoords = fs_in.TexCoords;
    float currentDepthMapValue = texture(depthMap, currentTexCoords).r;
    while (currentLayerDepth < currentDepthMapValue) {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(depthMap, currentTexCoords).r;
        // get depth of next layer
        currentLayerDepth += layerDepth;
    }

    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    float afterDepth = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(depthMap, prevTexCoords).r - currentLayerDepth + layerDepth;

    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;

    // Normal Parallax Mapping
    //float height = texture(depthMap, fs_in.TexCoords).r;
    //vec2 p = viewDir.xy * (height * height_scale);
    //return fs_in.TexCoords - p;
}

void main()
{
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec2 texCoords = ParallaxMapping(viewDir);
    if (texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
        discard;

    vec3 diffuse = texture(diffuseMap, texCoords).rgb;
    vec3 normal = texture(normalMap, texCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);

    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float diff = max(dot(lightDir, normal), 0.0);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    float attenuation = 1.0 / length(fs_in.TangentLightPos - fs_in.TangentFragPos);

    vec3 result = 0.05 * diffuse + attenuation * (2.0 * diff * diffuse + 0.2 * spec);
    FragColor = vec4(result, 1.0);
}
