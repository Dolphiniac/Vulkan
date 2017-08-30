#version 410 core

uniform mat4 gModel;
uniform mat4 gView;
uniform mat4 gProj;
uniform float gTime;
uniform int gEffectState;

in vec3 inPosition;
in vec4 inColor;
in vec2 inUV0;


out vec4 passColor;
out vec2 passUV0;

void main( void )
{
	passColor = inColor;
	passUV0 = inUV0;
	
	vec4 pos = vec4(inPosition, 1.f);

	mat4 model = gModel;

	if (gEffectState == 2 || gEffectState == 3)
	{
		model[0][3] *= sin(gTime);
	}
	
	pos = pos * model * gView * gProj;

	/*if (gEffectState == 2 || gEffectState == 3)
	{
		pos.x = pos.x * sin(gTime);
	}*/
	
	gl_Position = pos;
}