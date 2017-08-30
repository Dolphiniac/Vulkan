#version 410 core

uniform mat4 gModel;

layout (row_major, std140) uniform GlobalMatrices
{
	mat4 gView;
	mat4 gProj;
};

in vec3 inPosition;
in vec2 inUV0;
in vec4 inColor;
in vec3 inNormal;

out vec2 passUV0;
out vec4 passColor;
out vec3 passNormal;


void main()
{
	passUV0 = inUV0;
	passColor = inColor;
	passNormal = (vec4(inNormal, 0.f) * gModel).xyz;
	vec4 pos = vec4(inPosition, 1.f);
	pos = pos * gModel * gView * gProj;
	
	gl_Position = pos;
}