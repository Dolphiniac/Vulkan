
layout (location = 0) in vec3 inPosition;

layout (location = 0) out vec3 passPosition;

void main()
{
	passPosition = inPosition;
	vec4 pos = vec4(inPosition, 0.f);
	pos = pos * GlobalMatrices.View;
	pos.w = 1.f;
	pos = pos * GlobalMatrices.Projection;

	gl_Position = pos;
}