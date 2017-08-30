
layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec4 inColor;
layout (location = 2) in vec2 inUV0;

void main()
{
	vec4 pos = vec4(inPosition, 1.f);
	pos = pos * GlobalMatrices.View * GlobalMatrices.Projection;
	gl_Position = pos;
}