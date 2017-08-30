
layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec4 inColor;
layout (location = 2) in vec2 inUV0;
layout (location = 3) in vec4 inNormal;
layout (location = 4) in vec3 inTangent;

layout (location = 0) out vec4 passColor;
layout (location = 1) out vec2 passUV0;
layout (location = 2) out vec3 passNormal;
layout (location = 3) out vec3 passTangent;
layout (location = 4) out vec3 passBinormal;
layout (location = 5) out vec3 passPosition;

uniform ObjectLocal
{
	layout (row_major) mat4 Model;
};

void main()
{
	vec4 pos = vec4(inPosition, 1.f);
	pos = pos * Model * GlobalMatrices.View * GlobalMatrices.Projection;
	gl_Position = pos;
	passColor = inColor;

	passUV0 = inUV0;

	passTangent = normalize(inTangent);
	passNormal = normalize(inNormal.xyz);
	passBinormal = normalize(inNormal.w /* fSign */ * cross(passNormal, passTangent));

	passPosition = inPosition;
}