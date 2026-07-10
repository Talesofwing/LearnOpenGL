#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main()
{
    //FragColor = texture(texture1, TexCoords);
    //FragColor = vec4(1, 0, 0, 1);
    //if (gl_FragCoord.x < 400)
        //FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    //else
        //FragColor = vec4(0.0, 1.0, 0.0, 1.0);

    if (gl_FrontFacing)
        FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    else
        FragColor = vec4(0.0, 1.0, 0.0, 1.0);
}