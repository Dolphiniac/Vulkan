#include "Engine/Renderer/SpriteRenderer.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Core/EngineSystemManager.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Core/ConsoleCommand.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Renderer/UniformBlock.hpp"


//-----------------------------------------------------------------------------------------------
SpriteRenderer* g_spriteRenderer = nullptr;


//-----------------------------------------------------------------------------------------------
const std::string SpriteRenderer::s_vertexShader =
"#version 410 core\n"
"in vec2 inPosition;\n"
"in vec2 inUV0;\n"
"in vec4 inColor;\n"
"out vec2 passUV0;\n"
"out vec4 passColor;\n"
"layout (row_major, std140) uniform GlobalMatrices\n"
"{\n"
"mat4 gView;\n"
"mat4 gProj;\n"
"};\n"
"uniform mat4 gModel;\n"
"void main()\n"
"{\n"
"passUV0 = inUV0;\n"
"passColor = inColor;\n"
"vec4 pos = vec4(inPosition, 0.f, 1.f);\n"
"pos = pos * gView * gProj;\n"
"gl_Position = pos;\n"
"}\n";

const std::string SpriteRenderer::s_fragmentShader =
"#version 410 core\n"
"uniform sampler2D gDiffuseTex;\n"
"in vec2 passUV0;\n"
"in vec4 passColor;\n"
"out vec4 outColor;\n"
"void main()\n"
"{\n"
"vec4 diffuse = texture(gDiffuseTex, passUV0);\n"
"outColor = passColor * diffuse;\n"
"}\n";


//-----------------------------------------------------------------------------------------------
SpriteRenderer::SpriteRenderer()
	: m_clearColor(BLACK)
	, m_virtualToImportMultiplier(0.f)
{
	m_defaultMaterial = new Material(s_vertexShader, s_fragmentShader, false);
	m_defaultMaterial->BindUniformBlock("GlobalMatrices");
}


//-----------------------------------------------------------------------------------------------
SpriteRenderer::~SpriteRenderer()
{
	delete m_defaultMaterial;
}


//-----------------------------------------------------------------------------------------------
void SpriteRenderer::Update(float deltaSeconds)
{
	for (std::pair<const ERenderLayer, Layer>& p : m_layers)
	{
		p.second.Update(deltaSeconds);
	}
}


//-----------------------------------------------------------------------------------------------
void SpriteRenderer::Render() const
{
	The.Renderer->Clear(m_clearColor);
	The.Renderer->UseZBuffering(false);
	The.Renderer->UseZBufferWriting(false);
	The.Renderer->SetBlendFunction(ALPHABLEND);
	Matrix44 viewTrans;
	viewTrans.SetTranslation(Vector3(-m_cameraPosition.x, -m_cameraPosition.y, 0.f));
	UniformBlock::SetMat4("GlobalMatrices", "gView", viewTrans);

	for (const std::pair<const ERenderLayer, Layer>& p : m_layers)
	{
		p.second.Render();
	}
}


//-----------------------------------------------------------------------------------------------
void SpriteRenderer::SetImportAndVirtualSize(unsigned int importSize, unsigned int virtualUnitsPerDimension)
{
	m_virtualToImportMultiplier = (float)importSize / (float)virtualUnitsPerDimension;
	m_virtualUnitsPerDimension = virtualUnitsPerDimension;
	float halfUnits = (float)m_virtualUnitsPerDimension * .5f;
	m_virtualBounds = AABB2(-halfUnits, -halfUnits, halfUnits, halfUnits);
	Matrix44 proj;
	proj.MakeProjOrthogonal(-10.f, 10.f, -10.f, 10.f, 0.f, .5f);// Orthographic((float)m_virtualUnitsPerDimension, (float)m_virtualUnitsPerDimension);
	UniformBlock::SetMat4("GlobalMatrices", "gProj", proj);
}


//-----------------------------------------------------------------------------------------------
void SpriteRenderer::AddToLayer(RendererInterface* renderable, ERenderLayer layerID)
{
	m_layers[layerID].Add(renderable);
}


//-----------------------------------------------------------------------------------------------
void SpriteRenderer::RemoveFromLayers(RendererInterface* renderable)
{
	for (std::pair<const ERenderLayer, Layer>& p : m_layers)
	{
		p.second.Remove(renderable);
	}
}


//-----------------------------------------------------------------------------------------------
void SpriteRenderer::DisableLayer(ERenderLayer layer)
{
	//We want the layer to be created if it doesn't exist so commands will work
	//even if the layer is created after its disable
	m_layers[layer].Disable();
}


//-----------------------------------------------------------------------------------------------
void SpriteRenderer::EnableLayer(ERenderLayer layer)
{
	m_layers[layer].Enable();
}

static std::pair<std::string, ERenderLayer> s_layerStringConverters[NUM_LAYERS] =
{
	{ "background", BACKGROUND_LAYER },
	{ "environment", ENVIRONMENT_LAYER },
	{ "enemysubentity", ENEMY_SUBENTITY_LAYER },
	{ "enemy", ENEMY_LAYER },
	{ "playersubentity", PLAYER_SUBENTITY_LAYER },
	{ "player", PLAYER_LAYER },
	{ "fx", FX_LAYER }
};


//-----------------------------------------------------------------------------------------------
CONSOLE_COMMAND(Layer, args)
{
	std::string arg = args.GetNextArg();
	bool enable;
	if (arg == "disable")
	{
		enable = false;
	}
	else if (arg == "enable")
	{
		enable = true;
	}
	else
	{
		ConsolePrintf(RED, "Invalid command.  Expected enable / disable as first arg");
		return;
	}
	ERenderLayer layer = NUM_LAYERS;
	std::string layerString = args.GetNextArg();
	for (const std::pair<std::string, ERenderLayer>& converter : s_layerStringConverters)
	{
		if (layerString == converter.first)
		{
			layer = converter.second;
			break;
		}
	}

	if (layer == NUM_LAYERS)
	{
		ConsolePrintf(RED, "Invalid command.  Layer name does not exist.");
		return;
	}

	if (enable)
	{
		g_spriteRenderer->EnableLayer(layer);
	}
	else
	{
		g_spriteRenderer->DisableLayer(layer);
	}
}