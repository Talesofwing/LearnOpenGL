#version 330 core

in vec2 TexCoords;

uniform sampler2D hdrBuffer;

uniform bool gammaCorrectionEnabled;
uniform float exposure;

out vec4 FragColor;

void main()
{
    const float gamma = 2.2;
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);

    if (gammaCorrectionEnabled) {
        mapped = pow(mapped, vec3(1.0 / gamma));
    }

    FragColor = vec4(mapped, 1.0);
}