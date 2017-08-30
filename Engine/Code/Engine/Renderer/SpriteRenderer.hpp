#pragma once

#include "Engine/Renderer/Rgba.hpp"
#include "Engine/Math/AABB2.hpp"
#include <string>
#include <vector>
#include <map>
#include "Engine/Renderer/RendererInterface.hpp"


//-----------------------------------------------------------------------------------------------
extern class SpriteRenderer* g_spriteRenderer;


//-----------------------------------------------------------------------------------------------
enum ERenderLayer
{
	BACKGROUND_LAYER,
	ENVIRONMENT_LAYER,
	ENEMY_SUBENTITY_LAYER,
	ENEMY_LAYER,
	PLAYER_SUBENTITY_LAYER,
	PLAYER_LAYER,
	FX_LAYER,
	UI_LAYER,
	NUM_LAYERS
};


//-----------------------------------------------------------------------------------------------
struct Layer
{
	void Add(RendererInterface* renderable) { renderables.push_back(renderable); }
	void Remove(RendererInterface* renderable)
	{
		for (auto iter = renderables.begin(); iter != renderables.end(); iter++)
		{
			if (*iter == renderable)
			{
				renderables.erase(iter);
				break;
			}
		}
	}
	void Update(float deltaSeconds)
	{
		for (auto renderIter = renderables.begin(); renderIter != renderables.end();)
		{
			RendererInterface* renderable = *renderIter;
			renderable->Update(deltaSeconds);
			if (renderable->ShouldDie())
			{
				delete renderable;
				renderables.erase(renderIter++);
			}
			else
			{
				renderIter++;
			}
		}
	}
	void Render() const
	{
		if (!m_isEnabled)
		{
			return;
		}
		for (RendererInterface* renderable : renderables)
		{
			renderable->Render();
		}
	}
	bool IsEmpty() const { return renderables.empty(); }
	~Layer()
	{
		for (RendererInterface* renderable : renderables)
		{
			delete renderable;
		}
	}
	void Enable() { m_isEnabled = true; }
	void Disable() { m_isEnabled = false; }

private:
	std::vector<RendererInterface*> renderables;
	bool m_isEnabled = true;
};


//-----------------------------------------------------------------------------------------------
class SpriteRenderer
{
	static const std::string s_vertexShader;
	static const std::string s_fragmentShader;
public:
	SpriteRenderer();
	~SpriteRenderer();
	void SetClearColor(const Rgba& color) { m_clearColor = color; }
	void Update(float deltaSeconds);
	void Render() const;
	void SetImportAndVirtualSize(unsigned int importSize, unsigned int virtualUnitsPerDimension);
	void AddToLayer(RendererInterface* renderable, ERenderLayer layerID);
	void RemoveFromLayers(RendererInterface* renderable);
	void EnableLayer(ERenderLayer layer);
	void DisableLayer(ERenderLayer layer);
	AABB2 GetBoundingBox() const { return AABB2(m_virtualBounds.mins + m_cameraPosition, m_virtualBounds.maxs + m_cameraPosition); }
	Vector2 GetPosition() const { return m_cameraPosition; }
	void SetPosition(const Vector2& newPosition) { m_cameraPosition = newPosition; }
	class Material* GetMaterial() const { return m_defaultMaterial; }

private:
	Rgba m_clearColor;
	class Material* m_defaultMaterial;
	float m_virtualToImportMultiplier;
	unsigned int m_virtualUnitsPerDimension;
	AABB2 m_virtualBounds;
	std::map<ERenderLayer, Layer> m_layers;
	Vector2 m_cameraPosition;
};