
layout (location = 0) in vec3 passPosition;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outNormal;

uniform samplerCube gSkyboxTex;

void main()
{
	//vec3 workingPos = normalize(passPosition);
	//workingPos = (workingPos + vec3(1.f)) * .5f;
	//
	//outColor = vec4(workingPos, 1.f);

	outColor = texture(gSkyboxTex, passPosition);
	outNormal = vec4(vec3(0.f), 1.f);
}