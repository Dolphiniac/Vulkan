
layout (location = 0) in vec4 passColor;
layout (location = 1) in vec2 passUV0;

layout (location = 0) out vec4 outColor;

uniform sampler2D gDiffuseTex;

void main() 
{
	outColor = passColor * texture(gDiffuseTex, passUV0);
	outColor.a = 1.f;
}