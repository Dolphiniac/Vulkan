#pragma once

#include "Engine/Actor/Transform.hpp"
#include "Engine/Renderer/RendererInterface.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/LinearAllocator.hpp"
#include "Engine/Renderer/SpriteRenderer.hpp"
#include "Engine/Renderer/MeshRenderer.hpp"

#include <string>
#include <map>


//-----------------------------------------------------------------------------------------------
class SpriteResource
{
public:
	SpriteResource() = default;
	static class Sprite* GetInstance(const std::string& name, ERenderLayer layer);
	const class Texture* GetTexture() const { return m_texture; }
	static SpriteResource* Register(const std::string& name, const std::string& imageFilePath);
	static void UnloadDatabase();
	static const SpriteResource* GetResource(const std::string& name) { return s_spriteDatabase.at(name); }
	static SpriteResource* GetResourceEditable(const std::string& name) { return s_spriteDatabase.at(name); }
	static void LoadResourcesFromXML(const struct XMLNode& node);
	static void SaveResourcesToXML(const std::string& filepath);

	//-----------------------------------------------------------------------------------------------
	//DELETED COPY CONSTRUCTOR AND ASSIGNMENT OPERATOR - CANNOT COPY BY VALUE
	//-----------------------------------------------------------------------------------------------
	SpriteResource(const SpriteResource& toCopy) = delete;
	void operator=(const SpriteResource& toCopy) = delete;

private:
	std::string m_filepath;
	const class Texture* m_texture;
	static std::map<std::string, SpriteResource*> s_spriteDatabase;
};


//-----------------------------------------------------------------------------------------------
class Sprite : public RendererInterface
{
	friend class SpriteResource;
public:
	~Sprite() { delete m_meshRenderer; delete m_mesh; }
	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	bool ShouldDie() const override { return false; }
	void Enable() { g_spriteRenderer->AddToLayer(this, m_targetLayer); m_isEnabled = true; }
	void Disable() { g_spriteRenderer->RemoveFromLayers(this); m_isEnabled = false; }
	void SetTargetLayer(ERenderLayer layer);

	//-----------------------------------------------------------------------------------------------
	//PASSER-FUNCTIONS FOR MESH RENDERER, ALLOWING INSTANCED UNIFORM OVERRIDES
	//-----------------------------------------------------------------------------------------------
	void SetUniformFloat(const std::string& uniformName, float toSet) { m_meshRenderer->SetUniformFloat(uniformName, toSet); }
	void SetUniformInt(const std::string& uniformName, int toSet) { m_meshRenderer->SetUniformInt(uniformName, toSet); }
	void SetUniformVec3(const std::string& uniformName, const class Vector3& toSet) { m_meshRenderer->SetUniformVec3(uniformName, toSet); }
	void SetUniformVec4(const std::string& uniformName, const class Vector4& toSet) { m_meshRenderer->SetUniformVec4(uniformName, toSet); }
	void SetUniformMatrix44(const std::string& uniformName, const class Matrix44& toSet) { m_meshRenderer->SetUniformMatrix44(uniformName, toSet); }
	void SetUniformFloatArray(const std::string& arrayName, int numElements, float* data) { m_meshRenderer->SetUniformFloatArray(arrayName, numElements, data); }
	void SetUniformIntArray(const std::string& arrayName, int numElements, int* data) { m_meshRenderer->SetUniformIntArray(arrayName, numElements, data); }
	void SetUniformVec3Array(const std::string& arrayName, int numElements, Vector3* data) { m_meshRenderer->SetUniformVec3Array(arrayName, numElements, data); }
	void SetUniformVec4Array(const std::string& arrayName, int numElements, Vector4* data) { m_meshRenderer->SetUniformVec4Array(arrayName, numElements, data); }
	void SetUniformMatrix44Array(const std::string& arrayName, int numElements, Matrix44* data) { m_meshRenderer->SetUniformMatrix44Array(arrayName, numElements, data); }
	void SetUniformTexture(const std::string& samplerName, const class Texture* tex) { m_meshRenderer->SetUniformTexture(samplerName, tex); }
	void BindUniformBlock(const std::string& uniformBlockName) { m_meshRenderer->BindUniformBlock(uniformBlockName); }

public:
	Transform m_transform;
	Sprite* parent = nullptr;

protected:
	Sprite();
	const SpriteResource* m_resource;
	AABB2 m_boundingBoxAtIdentity = AABB2(-.5f, -.5f, .5f, .5f);
	class Material* m_materialOverride = nullptr;
	class MeshRenderer* m_meshRenderer = nullptr;
	mutable class Mesh* m_mesh = nullptr;
	ERenderLayer m_targetLayer;
	bool m_isEnabled = false;
};