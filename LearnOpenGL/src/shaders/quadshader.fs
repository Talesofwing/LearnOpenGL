#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;

const float xOffset = 1.0 / 800.0;
const float yOffset = 1.0 / 600.0;

vec2 offsets[9] = vec2[](
    vec2(-xOffset,  yOffset), // top-left
    vec2( 0.0f,    yOffset), // top-center
    vec2( xOffset,  yOffset), // top-right
    vec2(-xOffset,  0.0f),   // center-left
    vec2( 0.0f,    0.0f),   // center-center
    vec2( xOffset,  0.0f),   // center-right
    vec2(-xOffset, -yOffset), // bottom-left
    vec2( 0.0f,   -yOffset), // bottom-center
    vec2( xOffset, -yOffset)  // bottom-right
);

float kernel[9] = float[](
    -1, -1, -1,
    -1,  9, -1,
    -1, -1, -1
);

// Gaussian Blur Kernel
float blurKernel[9] = float[](
    1.0 / 16, 2.0 / 16, 1.0 / 16,
    2.0 / 16, 4.0 / 16, 2.0 / 16,
    1.0 / 16, 2.0 / 16, 1.0 / 16  
);

float edgeKernel[9] = float[](
    1.0,  1.0, 1.0,
    1.0, -8.0, 1.0,
    1.0,  1.0, 1.0
);

float sharpenKernel[9] = float[](
     0.0, -1.0,  0.0,
    -1.0,  5.0, -1.0,
     0.0, -1.0,  0.0
);

float strongSharpenKernel[9] = float[](
    -1.0, -1.0, -1.0,
    -1.0,  9.0, -1.0,
    -1.0, -1.0, -1.0
);

float embossKernel[9] = float[](
    -2.0, -1.0,  0.0,
    -1.0,  1.0,  1.0,
     0.0,  1.0,  2.0
);

void main()
{ 
    vec3 sampleTex[9];
    for (int i = 0; i < 9; ++i) {
        sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
    }

    vec3 col = vec3(0.0);
    for (int i = 0; i < 9; ++i) {
        col += sampleTex[i] * embossKernel[i];
    }
        
    FragColor = vec4(col, 1.0);
}