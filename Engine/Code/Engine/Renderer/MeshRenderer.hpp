#pragma once

#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Math/Matrix44.hpp"

#include <map>


//-----------------------------------------------------------------------------------------------
class MeshRenderer
{
public:
	MeshRenderer(class Mesh* mesh, class Material* material, bool isTransient = false);

	void SetMesh(class Mesh* mesh);
	void SetMaterial(class Material* mat);
	void BindMeshToVAOPCT();
	void BindMeshToVAOPCTN();
	void BindMeshToVAOPCTTB();
	void BindMeshToVAOPCTTBB();
	void BindMeshToVAOText();
	void BindMeshToVAOPCT2();
	void Render() const;
	void UpdateModelMatrix(const class Transform& transform);
	void UpdateVBO(void* data, size_t count, size_t elemSize, Renum usage);
	Material* GetMaterial() const { return m_material;}
	Mesh* GetMesh() const { return m_mesh; }
	~MeshRenderer();


	//-----------------------------------------------------------------------------------------------
	//PASSER-FUNCTIONS FROM MATERIAL
	//-----------------------------------------------------------------------------------------------
// 	bool SetUniformFloat(const std::string& uniformName, float toSet) { return m_material->SetUniformFloat(uniformName, toSet); }
// 	bool SetUniformInt(const std::string& uniformName, int toSet) { return m_material->SetUniformInt(uniformName, toSet); }
// 	bool SetUniformVec3(const std::string& uniformName, const class Vector3& toSet) { return m_material->SetUniformVec3(uniformName, toSet); }
// 	bool SetUniformVec4(const std::string& uniformName, const class Vector4& toSet) { return m_material->SetUniformVec4(uniformName, toSet); }
// 	bool SetUniformMatrix44(const std::string& uniformName, const class Matrix44& toSet) { return m_material->SetUniformMatrix44(uniformName, toSet); }
// 	bool SetUniformFloatArray(const std::string& arrayName, int numElements, float* data) { return m_material->SetUniformFloatArray(arrayName, numElements, data); }
// 	bool SetUniformIntArray(const std::string& arrayName, int numElements, int* data) { return m_material->SetUniformIntArray(arrayName, numElements, data); }
// 	bool SetUniformVec3Array(const std::string& arrayName, int numElements, Vector3* data) { return m_material->SetUniformVec3Array(arrayName, numElements, data); }
// 	bool SetUniformVec4Array(const std::string& arrayName, int numElements, Vector4* data) { return m_material->SetUniformVec4Array(arrayName, numElements, data); }
// 	bool SetUniformMatrix44Array(const std::string& arrayName, int numElements, Matrix44* data) { return m_material->SetUniformMatrix44Array(arrayName, numElements, data); }
	inline void SetUniformFloat(const std::string& uniformName, float toSet);
	inline void SetUniformInt(const std::string& uniformName, int toSet);
	inline void SetUniformVec3(const std::string& uniformName, const class Vector3& toSet);
	inline void SetUniformVec4(const std::string& uniformName, const class Vector4& toSet);
	inline void SetUniformMatrix44(const std::string& uniformName, const class Matrix44& toSet);
	inline void SetUniformFloatArray(const std::string& arrayName, int numElements, float* data);
	inline void SetUniformIntArray(const std::string& arrayName, int numElements, int* data);
	inline void SetUniformVec3Array(const std::string& arrayName, int numElements, Vector3* data);
	inline void SetUniformVec4Array(const std::string& arrayName, int numElements, Vector4* data);
	inline void SetUniformMatrix44Array(const std::string& arrayName, int numElements, Matrix44* data);
	void SetUniformTexture(const std::string& samplerName, const class Texture* tex) { m_samplers[samplerName] = tex; }
	void SetMatrices(const class Matrix44& model, const Matrix44& view, const Matrix44& projection) { m_material->SetMatrices(model, view, projection); }
	void BindUniformBlock(const std::string& uniformBlockName) { m_material->BindUniformBlock(uniformBlockName); }

private:
	void SetUniformsOnMaterial() const;

private:
	Material* m_material;
	Mesh* m_mesh;
	unsigned int m_vaoID;
	bool m_transient;

	//This is the only thing we want mutable, because we want to discard data that's rejected by the material
	mutable std::map<std::string, int> m_intUniforms;
	mutable std::map<std::string, float> m_floatUniforms;
	mutable std::map<std::string, Vector3> m_vec3Uniforms;
	mutable std::map<std::string, Vector4> m_vec4Uniforms;
	mutable std::map<std::string, Matrix44> m_mat4Uniforms;
	mutable std::map<std::string, const Texture*> m_samplers;
	mutable std::map<std::string, std::vector<int>> m_intUniformArrays;
	mutable std::map<std::string, std::vector<float>> m_floatUniformArrays;
	mutable std::map<std::string, std::vector<Vector3>> m_vec3UniformArrays;
	mutable std::map<std::string, std::vector<Vector4>> m_vec4UniformArrays;
	mutable std::map<std::string, std::vector<Matrix44>> m_mat4UniformArrays;
};


//-----------------------------------------------------------------------------------------------
void MeshRenderer::SetUniformFloat(const std::string& uniformName, float toSet)
{
	m_floatUniforms[uniformName] = toSet;
}



//-----------------------------------------------------------------------------------------------
void MeshRenderer::SetUniformInt(const std::string& uniformName, int toSet)
{
	m_intUniforms[uniformName] = toSet;
}


//-----------------------------------------------------------------------------------------------
void MeshRenderer::SetUniformVec3(const std::string& uniformName, const Vector3& toSet)
{
	m_vec3Uniforms[uniformName] = toSet;
}


//-----------------------------------------------------------------------------------------------
void MeshRenderer::SetUniformVec4(const std::string& uniformName, const Vector4& toSet)
{
	m_vec4Uniforms[uniformName] = toSet;
}


//-----------------------------------------------------------------------------------------------
void MeshRenderer::SetUniformMatrix44(const std::string& uniformName, const Matrix44& toSet)
{
	m_mat4Uniforms[uniformName] = toSet;
}


//-----------------------------------------------------------------------------------------------
void MeshRenderer::SetUniformFloatArray(const std::string& arrayName, int numElements, float* data)
{
	m_floatUniformArrays[arrayName] = std::vector<float>(data, data + numElements);
}


//-----------------------------------------------------------------------------------------------
void MeshRenderer::SetUniformIntArray(const std::string& arrayName, int numElements, int* data)
{
	m_intUniformArrays[arrayName] = std::vector<int>(data, data + numElements);
}


//-----------------------------------------------------------------------------------------------
void MeshRenderer::SetUniformVec3Array(const std::string& arrayName, int numElements, Vector3* data)
{
	m_vec3UniformArrays[arrayName] = std::vector<Vector3>(data, data + numElements);
}


//-----------------------------------------------------------------------------------------------
void MeshRenderer::SetUniformVec4Array(const std::string& arrayName, int numElements, Vector4* data)
{
	m_vec4UniformArrays[arrayName] = std::vector<Vector4>(data, data + numElements);
}


//-----------------------------------------------------------------------------------------------
void MeshRenderer::SetUniformMatrix44Array(const std::string& arrayName, int numElements, Matrix44* data)
{
	m_mat4UniformArrays[arrayName] = std::vector<Matrix44>(data, data + numElements);
}