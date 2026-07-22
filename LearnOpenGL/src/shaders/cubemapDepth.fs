#version 330 core

in vec4 WorldPos;

uniform vec3 lightPos;
uniform float far_plane;

void main() {
    float lightDistance = length(WorldPos.xyz - lightPos);

    // map to [0, 1]
    lightDistance = lightDistance / far_plane;

    gl_FragDepth = lightDistance;
}