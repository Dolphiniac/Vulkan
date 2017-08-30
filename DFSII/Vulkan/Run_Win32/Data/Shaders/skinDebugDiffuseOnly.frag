#version 410 core

uniform sampler2D gDiffuseTex;
uniform bool gIsDebugging;

in vec4 passColor;
in vec2 passUV0;

out vec4 outColor;


void main()
{
	vec2 flipUV0 = vec2(passUV0.x, -passUV0.y);
	vec4 diffuse = texture(gDiffuseTex, flipUV0);

	if (!gIsDebugging)
	{
		outColor = diffuse;
	}
	else
	{
		outColor = passColor;
	}
	//outColor = diffuse;
}