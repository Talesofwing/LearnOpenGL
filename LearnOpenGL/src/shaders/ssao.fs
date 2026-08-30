#version 330 core

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];
uniform mat4 projection;

const vec2 noiseScale = vec2(800.0/4.0, 600.0/4.0);
const float radius = 0.5;
const float bias = 0.025;

out float FragColor;

void main() {
    vec3 fragPos = texture(gPosition, TexCoords).xyz;
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);

    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));  // èÔò™ tangent ï˚å¸
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for (int i = 0; i < 64; ++i) {
        // get sample position
        vec3 samplePos = TBN * samples[i];  // from tangent to view-space
        samplePos = fragPos + samplePos * radius;

        // from view to screen space
        vec4 offset = vec4(samplePos, 1.0);
        offset = projection * offset;       // from view to clip-space
        offset.xyz /= offset.w;             // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5;// transform to range 0.0 - 1.0

        float sampleDepth = texture(gPosition, offset.xy).z;
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }
    FragColor = 1.0 - (occlusion / 64.0);
}
