
layout (location = 0) in vec3 inPosition;

uniform ObjectLocal
{
	layout (row_major) mat4 Model;
};

uniform LightmapInfo
{
	layout (row_major) mat4 LightView;
};

void main()
{
	vec4 pos = vec4(inPosition, 1.f);
	pos = pos * Model * LightView * GlobalMatrices.Projection;
	//pos = pos * Model * GlobalMatrices.View * GlobalMatrices.Projection;
	
	gl_Position = pos;
}