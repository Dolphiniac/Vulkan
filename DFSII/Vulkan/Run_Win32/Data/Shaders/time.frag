#version 410 core 

uniform vec4 gColor;
uniform sampler2D gDiffuseTex;
uniform float gTime;
uniform int gEffectState;

in vec4 passColor;
in vec2 passUV0;

out vec4 outColor;


void main( void ) 
{
	vec4 diffuse = texture(gDiffuseTex, passUV0);

	float warpColor;
	if (gEffectState == 1 || gEffectState == 3)
	{
		warpColor = (sin(gTime) + 1.f) * .5f;
	}
	else
	{
		warpColor = 1.f;
	}

	outColor = passColor * gColor * diffuse * warpColor;
}