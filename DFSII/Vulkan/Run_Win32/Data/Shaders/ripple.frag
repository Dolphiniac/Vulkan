#version 410 core 

uniform sampler2D gDiffuseTex;
uniform float gTime;

in vec4 passColor;
in vec2 passUV0;

out vec4 outColor;
//Inspired by FakeRipple (ushiostarfish)

void main( void ) 
{

	float w = .5f - passUV0.x;
	float h = .5f - passUV0.y;
	float distFromCenter = sqrt(w * w + h * h);

	float sinArg = distFromCenter * 10.f - gTime * 10.f;
	float slope = cos(sinArg);
	vec4 diffuse = texture(gDiffuseTex, passUV0 + normalize(vec2(w, h)) * slope * .05f);

	outColor = diffuse;
}