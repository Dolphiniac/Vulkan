#version 410 core

layout (row_major, std140) uniform GlobalMatrices
{
	mat4 gView;
	mat4 gProj;
};

layout (std140) uniform Time
{
	float gTime;
};

layout (std140) uniform EffectState
{
	int gEffectState;
};

void main(void)
{
	mat4 view = gView;
	mat4 proj = gProj;
}