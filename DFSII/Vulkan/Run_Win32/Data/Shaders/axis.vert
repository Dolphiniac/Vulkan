#version 410 core

layout (std140) uniform DISBLOCK
{
	vec3 viewV;
	mat4 projVV;
};

in vec3 inPosition;
in vec2 inUV0;
in vec4 inColor;

layout (std140) uniform DATBLOCK
{
	mat4 view;
	mat4 proj;
};

uniform mat4 gView;
uniform mat4 gProj;

out vec4 passColor;

void main(void)
{
	passColor = inColor;

	vec4 pos = vec4(inPosition, 1.f);

	gl_Position = pos * gView * gProj;
}