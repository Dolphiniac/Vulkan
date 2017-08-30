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
in vec3 inTangent;
in vec3 inBitangent;


out vec4 passColor;
out vec2 passUV0;
out vec3 passPos;
out vec3 passTangent;
out vec3 passBitangent;

void main( void )
{
	passColor = inColor;
	passUV0 = inUV0;
	
	vec4 pos = vec4(inPosition, 1.f);
	passPos = (pos * gModel).xyz;
	
	pos = pos * gModel * gView * gProj;

	vec4 tangent = vec4(inTangent, 0.f);
	tangent = tangent * gModel;
	passTangent = tangent.xyz;

	vec4 bitangent = vec4(inBitangent, 0.f);
	bitangent = bitangent * gModel;
	passBitangent = bitangent.xyz;
	

	gl_Position = pos;
}