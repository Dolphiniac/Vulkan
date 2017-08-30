#version 410 core 

uniform sampler2D gDiffuseTex;
uniform sampler2D gNormalTex;
uniform vec4 gAmbientLight;
uniform vec3 gLightPosition;
uniform float gMaxDistance;

in vec4 passColor;
in vec2 passUV0;
in vec3 passPos;
in vec3 passTangent;
in vec3 passBitangent;

out vec4 outColor;


void main( void ) 
{
	vec2 finalUV = vec2(passUV0.x, -passUV0.y);
	vec4 diffuse = texture(gDiffuseTex, finalUV);
	vec3 normalMap = texture(gNormalTex, finalUV).xyz;

	float dist = length(gLightPosition - passPos);
	float distIntensity = 1.f - smoothstep(0.f, gMaxDistance, dist);

	/*if (passUV0.x > .5f)
	{
		outColor = diffuse;
	}
	else
	{
		outColor = normalMap;
	}*/

	vec3 lightIntensity = gAmbientLight.rgb * gAmbientLight.a;

	vec3 surfaceTangent = normalize(passTangent);
	vec3 surfaceBitangent = normalize(passBitangent);
	vec3 surfaceNormal = cross(surfaceBitangent, surfaceTangent);
	surfaceBitangent = cross(surfaceTangent, surfaceNormal);

	mat3 tbn = mat3(surfaceTangent, surfaceBitangent, surfaceNormal);
	tbn = transpose(tbn);

	vec3 normal = normalMap * vec3(2.f, 2.f, 1.f) - vec3(1.f, 1.f, 0.f);
	normal = normalize(normal);
	normal = normal * tbn;
	//Transform basis
	//x -> right
	//y -> up
	//z -> -forward
	//normal = normalize(vec3(normal.x, normal.y, -normal.z));

	vec3 vecToLight = gLightPosition - passPos;

	vecToLight = normalize(vecToLight);
	float datDot = dot(normal, vecToLight);
	datDot = datDot * distIntensity;
	lightIntensity += vec3(datDot);

	lightIntensity = clamp(lightIntensity, vec3(0.f), vec3(1.f));


	outColor = vec4(lightIntensity, 1.f) * diffuse;
}