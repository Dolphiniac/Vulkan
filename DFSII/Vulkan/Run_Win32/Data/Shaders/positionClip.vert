
layout (location = 0) in vec3 inPosition;

layout(location = 0) out vec2 passUV0;

void main()
{
	vec2 workingUV = inPosition.xy;
	workingUV.y = -workingUV.y;
	workingUV = workingUV + vec2(1.f);
	workingUV = workingUV * .5f;

	passUV0 = workingUV;
	gl_Position = vec4(inPosition, 1.f);
}