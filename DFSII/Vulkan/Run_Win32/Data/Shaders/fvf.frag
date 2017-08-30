#version 410 core 

#define DIFFUSE_STATE 0
#define UV_STATE 1
#define NORMAL_STATE 2

uniform vec4 gColor;
uniform sampler2D gDiffuseTex;

layout (std140) uniform EffectState
{
	int gEffectState;
};

in vec2 passUV0;
in vec3 passNormal;
in vec4 passColor;
out vec4 outColor;

vec4 TexAsColor(vec2 uv)
{
	return vec4(uv.x, uv.y, 0.f, 1.f);
}

vec4 VectorAsColor(vec3 vec)
{
	vec3 trueVec = (normalize(vec) + vec3(1.f)) / vec3(2.f);
	return vec4(trueVec, 1.f);
}

vec4 NormalAsColor(vec3 normal)
{
	float r = (normal.r + 1.f) / 2.f;
	float g = (normal.g + 1.f) / 2.f;

	return vec4(r, g, normal.b, 1.f);
}

void main() 
{
	vec2 finalUV = vec2(passUV0.x, -passUV0.y);
	if (gEffectState == DIFFUSE_STATE)
	{
		outColor = gColor * passColor * texture(gDiffuseTex, finalUV);
	}
	else if (gEffectState == UV_STATE)
	{
		outColor = TexAsColor(passUV0);
	}
	else
	{
		outColor = VectorAsColor(passNormal);
	}
}