#version 410 core

uniform mat4 gModel;

layout (row_major, std140) uniform GlobalMatrices
{
	mat4 gView;
	mat4 gProj;
};

in vec3 inPosition;
in vec4 inColor;
in vec2 inUV0;


out vec4 passColor;
out vec2 passUV0;

void main( void )
{
	passColor = inColor;
	passUV0 = inUV0;
	
	vec4 pos = vec4(inPosition, 1.f);
	
	pos = pos * gModel * gView * gProj;
	
	gl_Position = pos;
}