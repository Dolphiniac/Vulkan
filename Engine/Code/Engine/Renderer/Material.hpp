#pragma once

#include "Engine/Renderer/GLRenderer.hpp"

#include <vector>
#include <tuple>
#include <string>
#include <map>


//-----------------------------------------------------------------------------------------------
class Material
{
	friend class Mesh;
	friend class MeshRenderer;

public:
	static Material* CreateOrGetMaterial(const std::string& uniformShaderName);
	bool SetUniformFloat(const std::string& uniformName, float toSet);
	bool SetUniformInt(const std::string& uniformName, int toSet);
	bool SetUniformVec3(const std::string& uniformName, const class Vector3& toSet);
	bool SetUniformVec4(const std::string& uniformName, const class Vector4& toSet);
	bool SetUniformMatrix44(const std::string& uniformName, const class Matrix44& toSet);
	bool SetUniformFloatArray(const std::string& arrayName, int numElements, float* data);
	bool SetUniformIntArray(const std::string& arrayName, int numElements, int* data);
	bool SetUniformVec3Array(const std::string& arrayName, int numElements, Vector3* data);
	bool SetUniformVec4Array(const std::string& arrayName, int numElements, Vector4* data);
	bool SetUniformMatrix44Array(const std::string& arrayName, int numElements, Matrix44* data);
	void SetUniformTexture(const std::string& samplerName, const class Texture* tex);
	void SetMatrices(const class Matrix44& model, const Matrix44& view, const Matrix44& projection);
	void BindUniformBlock(const std::string& uniformBlockName);
	void BindShaderStorageBlock(const std::string& shaderStorageBlockName);
	static void DestroyMaterials();
	Material(const std::string& vertexShaderName, const std::string& fragmentShaderName, bool usingFiles = true);
	Material(const Material* otherMat);

private:
	Material(const std::string& uniformShaderName);
	void BindProperty(const std::string& name, GLint count, GLenum type, GLboolean normalize, GLsizei stride, GLsizei offset, bool isInteger = false);
	void CreateSampler(Renum minFilter, Renum magFilter, Renum uWrap, Renum vWrap);
	GLuint LoadShader(const std::string& filename, GLenum shaderType, bool usingFiles);
	GLuint CreateAndLinkProgram(GLuint vs, GLuint fs, const std::string& fragFile);
	void UseProgramAndBindTextures();
	void UnbindTextures();
	void KillProgram();

public:
	unsigned int m_shaderProgramID;

private:
	static std::map<std::string, Material*> s_materialRegistry;
	unsigned int m_samplerID;
	std::vector<std::tuple<std::string, const Texture*>> m_textures;
};