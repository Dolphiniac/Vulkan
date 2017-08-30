
layout (location = 0) in vec4 passColor;
layout (location = 1) in vec2 passUV0;
layout (location = 2) in vec3 passNormal;
layout (location = 3) in vec3 passTangent;
layout (location = 4) in vec3 passBinormal;
layout (location = 5) in vec3 passPosition;

layout (location = 0) out vec4 outDiffuse;
layout (location = 1) out vec4 outNormal;

uniform sampler2D gDiffuseTex;
uniform sampler2D gBumpTex;


void main()
{
	outDiffuse = texture(gDiffuseTex, passUV0);
	outDiffuse = outDiffuse * passColor;

	vec2 normalizedFragCoord = gl_FragCoord.xy;
	normalizedFragCoord.x = normalizedFragCoord.x / 1600.f;
	normalizedFragCoord.y = normalizedFragCoord.y / 900.f;

	vec3 workingTangent = normalize(passTangent);
	vec3 workingBinormal = normalize(passBinormal);
	vec3 workingNormal = normalize(passNormal);

	mat3 tbn = mat3(workingTangent, workingBinormal, workingNormal);
	tbn = transpose(tbn);

	vec2 dx = dFdx(passUV0);
	vec2 dy = dFdy(passUV0);

	vec2 sampleCenter = passUV0;
	vec2 sampleRight = passUV0 + dx;
	vec2 sampleDown = passUV0 + dy;

	float heightCenter = texture(gBumpTex, sampleCenter).r;
	float heightRight = texture(gBumpTex, sampleRight).r;
	float heightDown = texture(gBumpTex, sampleDown).r;

	float deltaRight = heightRight - heightCenter;
	float deltaDown = heightDown - heightCenter;

	vec3 sigS = dFdx(passPosition);
	vec3 sigT = dFdy(passPosition);

	vec3 r1 = cross(sigT, workingNormal);
	vec3 r2 = cross(workingNormal, sigS);

	float det = dot(sigS, r1);

	vec3 surfaceGradient = sign(det) * (deltaRight * r1 + deltaDown * r2);
	vec3 finalNormal = normalize(abs(det) * workingNormal - surfaceGradient);


	//vec4 sampleNormal = texture(gBumpTex, passUV0);
	//sampleNormal = vec4(passUV0, sampleNormal.r, 1.f);
	//vec4 sampleDxNormal = texture(gBumpTex, dx);
	//sampleDxNormal = vec4(passUV0 + dx, sampleDxNormal.r, 1.f);
	//vec4 sampleDyNormal = texture(gBumpTex, dy);
	//sampleDyNormal = vec4(passUV0 + dy, sampleDyNormal.r, 1.f);
	//
	//vec3 tangentSpaceNormal = normalize(cross(sampleDxNormal.xyz - sampleNormal.xyz, sampleDyNormal.xyz - sampleNormal.xyz));
	//vec3 worldSpaceNormal = normalize(tangentSpaceNormal * tbn);
	//vec3 colorNormal = (worldSpaceNormal + vec3(1.f)) * .5f;
	vec3 colorNormal = (finalNormal + vec3(1.f)) * .5f;
	//vec3 colorNormal = (passNormal + vec3(1.f)) * .5f;
	outNormal = vec4(colorNormal, 1.f);
}