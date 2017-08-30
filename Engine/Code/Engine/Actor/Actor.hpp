#pragma once

#include "Engine/Actor/Transform.hpp"
#include "Engine/Renderer/MeshRenderer.hpp"
#include <map>

#define NULL_MESH_RENDERER_RETURN_FALSE if (m_meshRenderers.empty()) return false
#define NULL_MESH_RENDERER_RETURN if (m_meshRenderers.empty()) return


//-----------------------------------------------------------------------------------------------
class Actor
{
public:
	virtual void Tick(float deltaSeconds);	//Must override to set updating behavior
	void Render();	//Renders this actor using its MeshRenderers if applicable
	virtual bool ShouldRender() const;	//Override to set culling options for this actor
	static Actor* LoadActorFromXML(const struct XMLNode& node);
	virtual void Update(float deltaSeconds) { deltaSeconds; }
	~Actor();


	//-----------------------------------------------------------------------------------------------
	//PASSER-FUNCTIONS FROM MESH RENDERER
	//-----------------------------------------------------------------------------------------------
	void SetUniformFloat(const std::string& uniformName, float toSet) { NULL_MESH_RENDERER_RETURN; for (MeshRenderer*  m_meshRenderer: m_meshRenderers) m_meshRenderer->SetUniformFloat(uniformName, toSet); }
	void SetUniformInt(const std::string& uniformName, int toSet) { NULL_MESH_RENDERER_RETURN; for (MeshRenderer* m_meshRenderer : m_meshRenderers) m_meshRenderer->SetUniformInt(uniformName, toSet); }
	void SetUniformVec3(const std::string& uniformName, const class Vector3& toSet) { NULL_MESH_RENDERER_RETURN; for (MeshRenderer* m_meshRenderer : m_meshRenderers) m_meshRenderer->SetUniformVec3(uniformName, toSet); }
	void SetUniformVec4(const std::string& uniformName, const class Vector4& toSet) { NULL_MESH_RENDERER_RETURN; for (MeshRenderer* m_meshRenderer : m_meshRenderers) m_meshRenderer->SetUniformVec4(uniformName, toSet); }
	void SetUniformMatrix44(const std::string& uniformName, const class Matrix44& toSet) { NULL_MESH_RENDERER_RETURN; for (MeshRenderer* m_meshRenderer : m_meshRenderers) m_meshRenderer->SetUniformMatrix44(uniformName, toSet); }
	void SetUniformFloatArray(const std::string& arrayName, int numElements, float* data) { NULL_MESH_RENDERER_RETURN; for (MeshRenderer* m_meshRenderer : m_meshRenderers) m_meshRenderer->SetUniformFloatArray(arrayName, numElements, data); }
	void SetUniformIntArray(const std::string& arrayName, int numElements, int* data) { NULL_MESH_RENDERER_RETURN; for (MeshRenderer* m_meshRenderer : m_meshRenderers) m_meshRenderer->SetUniformIntArray(arrayName, numElements, data); }
	void SetUniformVec3Array(const std::string& arrayName, int numElements, Vector3* data) { NULL_MESH_RENDERER_RETURN; for (MeshRenderer* m_meshRenderer : m_meshRenderers) m_meshRenderer->SetUniformVec3Array(arrayName, numElements, data); }
	void SetUniformVec4Array(const std::string& arrayName, int numElements, Vector4* data) { NULL_MESH_RENDERER_RETURN; for (MeshRenderer* m_meshRenderer : m_meshRenderers) m_meshRenderer->SetUniformVec4Array(arrayName, numElements, data); }
	void SetUniformMatrix44Array(const std::string& arrayName, int numElements, Matrix44* data) { NULL_MESH_RENDERER_RETURN; for (MeshRenderer* m_meshRenderer : m_meshRenderers) m_meshRenderer->SetUniformMatrix44Array(arrayName, numElements, data); }
	void SetUniformTexture(const std::string& samplerName, class Texture* tex) { NULL_MESH_RENDERER_RETURN; for (MeshRenderer* m_meshRenderer : m_meshRenderers) m_meshRenderer->SetUniformTexture(samplerName, tex); }
	void SetMatrices(const class Matrix44& model, const Matrix44& view, const Matrix44& projection) { NULL_MESH_RENDERER_RETURN; for (MeshRenderer* m_meshRenderer : m_meshRenderers) m_meshRenderer->SetMatrices(model, view, projection); }
	void BindUniformBlock(const std::string& uniformBlockName) { NULL_MESH_RENDERER_RETURN; for (MeshRenderer* m_meshRenderer : m_meshRenderers) m_meshRenderer->BindUniformBlock(uniformBlockName); }
	Material* GetMaterial(int index = 0) const { if ((int)m_meshRenderers.size() <= index) return nullptr; return m_meshRenderers[index]->GetMaterial(); }
	Mesh* GetMesh(int index = 0) const { if ((int)m_meshRenderers.size() <= index) return nullptr; return m_meshRenderers[index]->GetMesh(); }
	void SetMesh(class Mesh* mesh) { NULL_MESH_RENDERER_RETURN; for (MeshRenderer* m_meshRenderer : m_meshRenderers) m_meshRenderer->SetMesh(mesh); }
	void SetMaterial(class Material* mat) { NULL_MESH_RENDERER_RETURN; for (MeshRenderer* m_meshRenderer : m_meshRenderers) m_meshRenderer->SetMaterial(mat); }
	void SetMesh(class Mesh* mesh, int index) { NULL_MESH_RENDERER_RETURN; if ((int)m_meshRenderers.size() <= index) return; m_meshRenderers[index]->SetMesh(mesh); }
	void SetMaterial(class Material* mat, int index) { NULL_MESH_RENDERER_RETURN; if ((int)m_meshRenderers.size() <= index) return; m_meshRenderers[index]->SetMaterial(mat); }

public:
	Transform m_transform;
	std::vector<MeshRenderer*> m_meshRenderers;
	class Skeleton* skeleton = nullptr;
	std::map<std::string, class Motion*> m_motions;
};