#version 410 core

uniform mat4 gModel;

layout (row_major, std140) uniform GlobalMatrices
{
	mat4 gView;
	mat4 gProj;
};

uniform mat4 gSkinningMatrices[200];
uniform bool gIsDebugging;

in vec3 inPosition;
in vec4 inColor;
in vec2 inUV0;

in ivec4 inBoneIndices;
in vec4 inBoneWeights;

out vec4 passColor;
out vec2 passUV0;


//-----------------------------------------------------------------------------------------------
void main()
{
	mat4 bone0 = gSkinningMatrices[inBoneIndices.x];
	mat4 bone1 = gSkinningMatrices[inBoneIndices.y];
	mat4 bone2 = gSkinningMatrices[inBoneIndices.z];
	mat4 bone3 = gSkinningMatrices[inBoneIndices.w];

	mat4 boneTransform = inBoneWeights.x * bone0
	 + inBoneWeights.y * bone1
	  + inBoneWeights.z * bone2
	   + inBoneWeights.w * bone3;

	mat4 toWorld = boneTransform * gModel;

	if (gIsDebugging)
	{
		passColor = vec4(inBoneWeights.xyz, 1.f);
	}
	else
	{
		passColor = vec4(1.f);
	}
	passUV0 = inUV0;
	
	gl_Position = vec4(inPosition, 1.f) * toWorld * gView * gProj;
}