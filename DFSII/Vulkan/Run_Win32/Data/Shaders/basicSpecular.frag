#version 410 core 

uniform sampler2D gDiffuseTex;
uniform sampler2D gNormalTex;
uniform sampler2D gEmissiveTex;
uniform vec4 gAmbientLight;
uniform vec3 gCamPos;
uniform float gSpecularExponent;
uniform int gLightCount;

uniform vec3 gLightPositions[16];
uniform vec3 gLightColorsRGB[16];
uniform float gLightBrightnesses[16];
uniform float gLightInnerRadii[16];
uniform float gLightOuterRadii[16];
uniform vec3 gLightForwards[16];
uniform float gLightInnerApertures[16];
uniform float gLightOuterApertures[16];
uniform float gLightHasForward[16];

uniform sampler2D gSpecularTex;

in vec4 passColor;
in vec2 passUV0;
in vec3 passPos;
in vec3 passTangent;
in vec3 passBitangent;

out vec4 outColor;


//------------------------------------------------------------------------
vec4 VectorAsColor( vec3 vec ) 
{
	return vec4( (vec + vec3(1.0f)) * vec3(.5f), 1.0f );
}


//-----------------------------------------------------------------------------------------------
struct SurfaceLight
{
	vec3 surface;
	vec3 specular;
};


//-----------------------------------------------------------------------------------------------
SurfaceLight CalculateSurfaceLight(vec3 surfacePosition, vec3 surfaceNormal,
	vec3 vecToEye,
	vec3 lightPosition,
	vec3 lightForward,
	vec3 lightColor,
	float lightHasForward, //1 if light is directional, 0 otherwise
	float specularIntensity,
	float lightInnerRadius,
	float lightOuterRadius,
	float lightBrightness,
	float lightInnerAperture,
	float lightOuterAperture)
{
	vec3 vecToLight = lightPosition - surfacePosition;
	float distToLight = length(vecToLight);

	vec3 dirToLight = mix(vecToLight / distToLight, -lightForward, lightHasForward);
	distToLight = mix(distToLight, dot(vecToLight, -lightForward), lightHasForward);

	vec3 halfVec = normalize(normalize(dirToLight) + normalize(vecToEye));
	float angle = dot(lightForward, -dirToLight);

	float distAttenuation = mix(lightBrightness, 0.f, smoothstep(lightInnerRadius, lightOuterRadius, distToLight));
	float angleAttenuation = mix(lightBrightness, 0.f, smoothstep(lightInnerAperture, lightOuterAperture, angle));

	float attenuation = distAttenuation * angleAttenuation;

	float dot3Factor =	max(dot(surfaceNormal, dirToLight), 0.f) * attenuation;
	vec3 dot3Color = lightColor * dot3Factor;

	float specularFactor = max(dot(surfaceNormal, halfVec), 0.f);
	specularFactor = pow(specularFactor, gSpecularExponent) * specularIntensity	* attenuation;
	vec3 specularColor = lightColor * specularFactor;

	SurfaceLight result;
	result.surface = dot3Color;
	result.specular = specularColor;

	return result;
}


void main( void ) 
{
	vec4 diffuse = texture(gDiffuseTex, passUV0);
	vec4 emissive = texture(gEmissiveTex, passUV0);
	vec3 normalMap = texture(gNormalTex, passUV0).xyz;

//	float dist = length(gLightPosition - passPos);
//	float distIntensity = 1.f - smoothstep(0.f, gMaxDistance, dist);

	float specularIntensity = texture(gSpecularTex, passUV0).r;

	vec3 ambientColor = gAmbientLight.rgb * gAmbientLight.a;
	vec3 emissiveColor = emissive.rgb * emissive.a;

	vec3 surfaceTangent = normalize(passTangent);
	vec3 surfaceBitangent = normalize(passBitangent);
	vec3 surfaceNormal = cross(surfaceBitangent, surfaceTangent);
	surfaceBitangent = cross(surfaceTangent, surfaceNormal);

	mat3 tbn = mat3(surfaceTangent, surfaceBitangent, surfaceNormal);
	tbn = transpose(tbn);

	vec3 normal = (normalMap * vec3(2.f, 2.f, 1.f)) - vec3(1.f, 1.f, 0.f);
	normal = normalize(normal);
	normal = normal * tbn;

	vec3 vecToEye = gCamPos - passPos;
	float distToEye = length(vecToEye);
	vec3 dirToEye = vecToEye / distToEye;

	vec3 dot3Light = vec3(0.f);
	vec3 specularLight = vec3(0.f);

	for (int i = 0; i < gLightCount; i++)
	{
		SurfaceLight light = CalculateSurfaceLight(passPos, normal,
		dirToEye,
		gLightPositions[i],
		gLightForwards[i],
		gLightColorsRGB[i],
		gLightHasForward[i],
		specularIntensity,
		gLightInnerRadii[i],
		gLightOuterRadii[i],
		gLightBrightnesses[i],
		gLightInnerApertures[i],
		gLightOuterApertures[i]);

		dot3Light += light.surface;
		specularLight += light.specular;
	}

	vec3 surfaceLight = ambientColor + dot3Light + emissiveColor;
	surfaceLight = clamp(surfaceLight, vec3(0.f), vec3(1.f));

	vec4 finalColor = diffuse * vec4(surfaceLight, 1.f) + vec4(specularLight, 0.f);
	finalColor = clamp(finalColor, vec4(0.f), vec4(1.f));

	outColor = finalColor;
	
	
	
	
	
//	vec3 vecToEye = normalize(gCamPos - passPos);
//	vec3 vecToLight = normalize(gLightPosition - passPos);

//	vec3 halfwayVector = normalize(vecToEye + vecToLight);

	/*if (passUV0.x > .5f)
	{
		outColor = diffuse;
	}
	else
	{
		outColor = normalMap;
	}*/

//	vec3 lightIntensity = gAmbientLight.rgb * gAmbientLight.a;
//
//	vec3 surfaceTangent = normalize(passTangent);
//	vec3 surfaceBitangent = normalize(passBitangent);
//	vec3 surfaceNormal = cross(surfaceBitangent, surfaceTangent);
//	surfaceBitangent = cross(surfaceTangent, surfaceNormal);
//
//	mat3 tbn = mat3(surfaceTangent, surfaceBitangent, surfaceNormal);
//	tbn = transpose(tbn);
//
//	vec3 normal = normalMap * vec3(2.f, 2.f, 1.f) - vec3(1.f, 1.f, 0.f);
//	normal = normalize(normal);
//	normal = normal * tbn;
//	//Transform basis
//	//x -> right
//	//y -> up
//	//z -> -forward
//	//normal = normalize(vec3(normal.x, normal.y, -normal.z));
//
//	vecToLight = normalize(vecToLight);
//	float datDot = dot(normal, vecToLight);
//	datDot = datDot * distIntensity;
//	lightIntensity += vec3(datDot);
//
//	lightIntensity = clamp(lightIntensity, vec3(0.f), vec3(1.f));
//
//	vec4 specularColor = vec4(1.f) * distIntensity * specularIntensity * pow(clamp(dot(halfwayVector, normal), 0.f, 1.f), gShininess);
//
//
//	diffuse = vec4(lightIntensity, 1.f) * diffuse + specularColor;
//
//	vec3 emissive3 = emissive.rgb * emissive.a;
//
//	vec3 diffuse3 = diffuse.rgb + emissive3;
//
//	diffuse = vec4(diffuse3.rgb, diffuse.a);
//
//	float distFog = length(passPos - gCamPos);
//
//	float fogMultiplier = (distFog - minDistance) / (maxDistance - minDistance) * (1.f - 0.f) + 0.f;
//	fogMultiplier = clamp(fogMultiplier, 0.f, 1.f);
//
//	vec4 finalFogColor = gFogColor * fogMultiplier;
//	vec4 finalDiffuse = diffuse * (1.f - fogMultiplier);
//
//	float dissolveTime = gTime - int(gTime);
//
//	float dissolveFactor = texture(gDissolveTex, passUV0).r;
//
//	outColor = finalDiffuse + finalFogColor;
//
//	if (gEffectState == 1)
//	{
//		float dissolveMin = dissolveFactor - .05f;
//		if (dissolveTime >= dissolveFactor)
//		{
//			outColor = vec4(0.f);
//		}
//		else if (dissolveTime >= dissolveMin)
//		{
//			float dissolvedness = (dissolveTime - dissolveMin) / (dissolveFactor - dissolveMin) * (1.f - 0.f) + 0.f;
//			vec4 finalDissolveColor = gDissolveColor * dissolvedness;
//	
//			finalDiffuse = outColor * (1.f - dissolvedness);
//			outColor = finalDiffuse + finalDissolveColor;
//		}
//	}

}