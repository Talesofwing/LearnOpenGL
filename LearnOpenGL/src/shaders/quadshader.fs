#version 330 core
  
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float near_plane;
uniform float far_plane;

out vec4 FragColor;

float LinearizeDepth(float depth) {
    float z = depth * 2.0 - 1.0;
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main()
{ 
    float depthValue = texture(screenTexture, TexCoords).r;
    //depthValue = LinearizeDepth(depthValue);  // perspective
    FragColor = vec4(vec3(depthValue), 1.0);
}