
layout (input_attachment_index = 0) uniform subpassInput gDiffuseInput;
layout (input_attachment_index = 1) uniform subpassInput gNormalInput;

uniform sampler2D gDepth;
uniform sampler2D gLightmap;

layout (location = 0) out vec4 outColor;

layout (location = 0) in vec2 passUV0;

uniform ViewSelector
{
	float selector;
};

uniform LightmapInfo
{
	layout (row_major) mat4 LightView;
};

float GetEqualityFactor(float a, float b)
{
	//sign(a - b) will be 0.f if they're equal, and 1.f or -1.f if different
	//abs of this will make it 0.f if equal and 1.f if different
	//If we subtract it from 1.f, the values are reversed, returning 1.f if equal and 0.f if different
	return 1.f - abs(sign(a - b));
}

float GetInequalityFactor(float a, float b)
{
	return 1.f - GetEqualityFactor(a, b);
}

float GetLessThanFactor(float a, float b)
{
	return GetEqualityFactor(sign(a - b), -1.f);
}

float GetEqualityFactorWithEpsilon(float a, float b)
{
	float epsilon = .00005f;
	return GetEqualityFactor(GetLessThanFactor(b, a + epsilon) + GetLessThanFactor(a - epsilon, b), 2.f);	//Checks that b < a + epsilon and a - epsilon < b
}

float GetInequalityFactorWithEpsilon(float a, float b)
{
	return 1.f - GetEqualityFactorWithEpsilon(a, b);
}

float GetInRangeFactor(vec2 vec, float rangeBottom, float rangeTop)
{
	float result = GetLessThanFactor(vec.x, rangeTop);
	result *= GetLessThanFactor(rangeBottom, vec.x);
	result *= GetLessThanFactor(vec.y, rangeTop);
	result *= GetLessThanFactor(rangeBottom, vec.y);
	return result;
}

float GetOutOfRangeFactor(vec2 vec, float rangeBottom, float rangeTop)
{
	return 1.f - GetInRangeFactor(vec, rangeBottom, rangeTop);
}

void main()
{
	//First extracting the different views of this scene
	vec4 diffuseColor = subpassLoad(gDiffuseInput);
	vec4 normalColor = subpassLoad(gNormalInput);

	//Since depth is a D32, we need to extract the red to the other channels so it appears as a black to white gradient
	float depth = texture(gDepth, passUV0).r;
	vec4 depthColor = vec4(depth.rrr, 1.f);

	//We'll get the position from the fragment position and depth, using black magic
	vec4 position = vec4(passUV0, depthColor.r, 1.f);
	position.x = position.x * 2.f - 1.f;
	position.y = position.y * 2.f - 1.f;
	position.y = -position.y;
	position.w = 1.f;
	position *= GlobalMatrices.InvProjection;
	position /= position.w;
	position *= GlobalMatrices.InvView;

	//Next, we can go into the light's perspective by multiplying by the light's view and the projection matrix
	vec4 lightPos = position * LightView * GlobalMatrices.Projection;
	lightPos /= lightPos.w;	//NDC
	vec2 lightUV = (lightPos.xy + vec2(1.f)) * .5f;	//Next, we convert the xy of NDC to UV space to sample the lightmap
	lightUV.y = 1.f - lightUV.y;

	vec4 lightDepthColor = texture(gLightmap, lightUV);
	float lightDepth = lightDepthColor.r;
	const float ambientFactor = .25f;
	float inLightFactor = GetEqualityFactorWithEpsilon(lightDepth, lightPos.z);
	float outOfLightFactor = 1.f - inLightFactor;
	vec4 lightColor = diffuseColor * inLightFactor + diffuseColor * ambientFactor * outOfLightFactor;
	
	vec3 worldSpaceNormal = normalColor.xyz * 2.f - vec3(1.f);

	vec3 dirToShadowLight = normalize(-LightView[3].xyz - position.xyz);
	lightColor *= GetLessThanFactor(0.f, dot(worldSpaceNormal, dirToShadowLight));

	//One light, at 0, 0, 0, so the negation of the position vector is the vector from the position to the light
	vec3 dirToLight = normalize(-position.xyz);

	//Simple Lambertian reflectance with intensity of 1
	const float dot3Intensity = .8f;
	vec4 dot3Lighting = vec4(vec3(max(dot(worldSpaceNormal, dirToLight), 0.f)), 0.f) * dot3Intensity * diffuseColor;

	//We'll only show the diffuse G-Buffer if the depth buffer is clear at this fragment or 
	float diffuseOnlyFactor = GetEqualityFactorWithEpsilon(depth, 1.f);
	float lightingFactor = 1.f - diffuseOnlyFactor;	//Inverse of diffuse only, to use as a selector between lit and diffuse
	vec4 diffuseOnlyColor = diffuseColor * diffuseOnlyFactor;

	vec4 litColor = dot3Lighting + lightColor;
	litColor *= lightingFactor;
	vec4 compositeColor = diffuseOnlyColor + litColor;

	//Now, we're going to calculate the fragment in a subdivided state.
	//Using the UV coordinate to subdivide the space into four regions
	float workingX = passUV0.x;
	workingX = workingX * 2.f;
	workingX = ceil(workingX);

	float workingY = passUV0.y;
	workingY = workingY * 2.f;
	workingY = ceil(workingY);

	float leftXFactor = GetEqualityFactor(workingX, 1.f);
	float rightXFactor = GetEqualityFactor(workingX, 2.f);
	float topYFactor = GetEqualityFactor(workingY, 1.f);
	float bottomYFactor = GetEqualityFactor(workingY, 2.f);

	//We can multiply the target color by a 1.f if it's in the correct division for this X or 0.f otherwise, effectively choosing between the three
	float subdivisionOneFactor = leftXFactor * topYFactor;
	float subdivisionTwoFactor = rightXFactor * topYFactor;
	float subdivisionThreeFactor = leftXFactor * bottomYFactor;
	float subdivisionFourFactor = rightXFactor * bottomYFactor;

	vec4 subdivisionOneColor = diffuseColor * subdivisionOneFactor;
	vec4 subdivisionTwoColor = normalColor * subdivisionTwoFactor;
	vec4 subdivisionThreeColor = depthColor * subdivisionThreeFactor;
	vec4 subdivisionFourColor = compositeColor * subdivisionFourFactor;

	vec4 subdivisionColor = subdivisionOneColor + subdivisionTwoColor + subdivisionThreeColor + subdivisionFourColor;

	//Now, we can select from these modes by equating a modulo selector with the mode switches
	//First, we take the selector and make it in the 0-modeCount range
	float actualSelector = float(int(selector) % 5);

	//Selection 0.f will be composite
	vec4 compositeMode = compositeColor * GetEqualityFactor(actualSelector, 0.f);

	//Selection 1.f will be diffuse
	vec4 diffuseMode = diffuseColor * GetEqualityFactor(actualSelector, 1.f);

	//Selection 2.f will be normal
	vec4 normalMode = normalColor * GetEqualityFactor(actualSelector, 2.f);

	//Selection 3.f will be depth
	vec4 depthMode = depthColor * GetEqualityFactor(actualSelector, 3.f);

	//Selection 4.f will be subdivisions
	subdivisionColor = subdivisionColor * GetEqualityFactor(actualSelector, 4.f);

	outColor = subdivisionColor + diffuseMode + normalMode + depthMode + compositeMode;
}