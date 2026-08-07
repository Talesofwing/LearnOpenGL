#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

layout (std140) uniform Matrices
{
	mat4 projection;
	mat4 view;
};
uniform mat4 model;
uniform bool reverse_normals;

out VS_OUT {
	vec3 WorldPos;
	vec3 Normal;
	vec2 TexCoords;
} vs_out;

void main()
{
	vs_out.WorldPos = vec3(model * vec4(aPos, 1.0));
	if (reverse_normals) {
		vs_out.Normal = -aNormal * mat3(inverse(model));
	} else {
		vs_out.Normal = aNormal * mat3(inverse(model));
	}
	vs_out.TexCoords = aTexCoords;

	gl_Position = projection * view * model * vec4(aPos, 1.0);
}