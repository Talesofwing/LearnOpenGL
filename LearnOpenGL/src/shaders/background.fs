#version 330 core

out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube environmentMap;

uniform bool hdrEnabled;
uniform bool gammaEnabled;

void main()
{
    vec3 envColor = texture(environmentMap, TexCoords).rgb;

    // HDR tonemap and gamma correct
    if (hdrEnabled)
        envColor = envColor / (envColor + vec3(1.0));
    if (gammaEnabled)
        envColor = pow(envColor, vec3(1.0 / 2.2));

    FragColor = vec4(envColor, 1.0);
}
