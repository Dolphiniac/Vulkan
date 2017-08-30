#version 410 core 

uniform sampler2D gDiffuseTex;
uniform vec4 gFogColor;
uniform float minDistance;
uniform float maxDistance;
uniform vec3 gCamPos;

in vec4 passColor;
in vec2 passUV0;
in vec3 passPos;

out vec4 outColor;


void main( void ) 
{
	vec4 diffuse = texture(gDiffuseTex, passUV0);

	float dist = length(passPos - gCamPos);

	float fogMultiplier = (dist - minDistance) / (maxDistance - minDistance) * (1.f - 0.f) + 0.f;
	fogMultiplier = clamp(fogMultiplier, 0.f, 1.f);

	vec4 finalFogColor = gFogColor * fogMultiplier;
	vec4 finalDiffuse = diffuse * (1.f - fogMultiplier);

	outColor = finalDiffuse + finalFogColor;
}