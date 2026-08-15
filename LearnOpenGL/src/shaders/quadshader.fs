#version 330 core
  
in vec2 TexCoords;

uniform sampler2D screenTexture;

out vec4 FragColor;

void main()
{ 
    vec3 color = texture(screenTexture, TexCoords).rgb;
    FragColor = vec4(color, 1.0);
}