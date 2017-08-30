#version 410 core

layout (row_major, std140) uniform GlobalMatrices
{
	mat4 gView;
	mat4 gProj;
};
uniform mat4 gModel;

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

	vec4 tangent = vec4(inTangent, 0.f) * gModel;
	passTangent = tangent.xyz;

	vec4 bitangent = vec4(inBitangent, 0.f) * gModel;
	passBitangent = bitangent.xyz;
	
	pos = pos * gModel * gView * gProj;
	

	gl_Position = pos;
}