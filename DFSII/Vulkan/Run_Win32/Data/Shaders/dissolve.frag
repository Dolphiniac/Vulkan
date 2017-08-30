#version 410 core 

uniform vec4 gDissolveColor;
uniform sampler2D gDiffuseTex;
uniform sampler2D gDissolveTex;

layout (std140) uniform Time
{
	float gTime;
};

in vec4 passColor;
in vec2 passUV0;

out vec4 outColor;


//-----------------------------------------------------------------------------------------------
float RangeMap(float toMap, float inputStart, float inputEnd, float outputStart, float outputEnd)
{
	return (toMap - inputStart) / (inputEnd - inputStart) * (outputEnd - outputStart) + outputStart;
}


//-----------------------------------------------------------------------------------------------
void main( void ) 
{
	vec4 diffuse = texture(gDiffuseTex, passUV0);
	float dissolve = texture(gDissolveTex, passUV0).r;

	float myTime = gTime * .5f;
	myTime = myTime - int(myTime);

	float colorDistFromDissolve = .2f;

	outColor = passColor * diffuse;
	outColor.a = 1.f;

	if (myTime < dissolve && myTime > dissolve - colorDistFromDissolve)
	{
		outColor = mix(outColor, gDissolveColor, RangeMap(myTime, dissolve - colorDistFromDissolve, dissolve, 0.f, 1.f));
	}
	else if (myTime >= dissolve)
	{
		outColor = vec4(0.f);
	}
}