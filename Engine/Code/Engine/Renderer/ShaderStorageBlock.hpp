#pragma once

#include "Engine/Renderer/GLRenderer.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/Matrix44.hpp"

#include <string>
#include <map>
#include <tuple>


//-----------------------------------------------------------------------------------------------
class ShaderStorageBlock
{
private:
	inline ShaderStorageBlock(const std::string& name);
	void GenerateBuffer();
	std::tuple<int, int> GetPosAndSize(const std::string& fieldName) const;
	int GetTotalOffset() const;
	inline void PushBackInt(const std::string& fieldName);
	inline void PushBackFloat(const std::string& fieldName);
	inline void PushBackVec2(const std::string& fieldName);
	inline void PushBackVec3(const std::string& fieldName);
	inline void PushBackVec4(const std::string& fieldName);
	inline void PushBackMat4(const std::string& fieldName);
	static void SetField(const std::string& blockName, const std::string& fieldName, void* toSet);
	static void GetField(const std::string& blockName, const std::string& fieldName, void* returnValue);

public:
	static void InitializeShaderStorageBlocks();
	static void DestroyShaderStorageBlocks();
	static ShaderStorageBlock* GetBlock(const std::string& blockName);
	inline static void SetInt(const std::string& blockName, const std::string& fieldName, int toSet);
	inline static void SetFloat(const std::string& blockName, const std::string& fieldName, float toSet);
	inline static void SetVec2(const std::string& blockName, const std::string& fieldName, const Vector2& toSet);
	inline static void SetVec3(const std::string& blockName, const std::string& fieldName, const Vector3& toSet);
	inline static void SetVec4(const std::string& blockName, const std::string& fieldName, const Vector4& toSet);
	inline static void SetMat4(const std::string& blockName, const std::string& fieldName, const Matrix44& toSet);
	inline static int GetInt(const std::string& blockName, const std::string& fieldName);
	inline static float GetFloat(const std::string& blockName, const std::string& fieldName);
	inline static Vector2 GetVec2(const std::string& blockName, const std::string& fieldName);
	inline static Vector3 GetVec3(const std::string& blockName, const std::string& fieldName);
	inline static Vector4 GetVec4(const std::string& blockName, const std::string& fieldName);
	inline static Matrix44 GetMat4(const std::string& blockName, const std::string& fieldName);

private:
	static std::map<std::string, ShaderStorageBlock*> s_shaderStorageBlocks;
	static GLuint s_currentBindingPoint;
	std::map<std::string, std::tuple<int, int>> m_posAndSize;
	std::string m_name;

public:
	GLuint m_ssboID;
	GLint m_bindingPoint;
};





//-----------------------------------------------------------------------------------------------
ShaderStorageBlock::ShaderStorageBlock(const std::string& name)
	: m_name(name)
{
	m_bindingPoint = s_currentBindingPoint++;
}


//-----------------------------------------------------------------------------------------------
void ShaderStorageBlock::PushBackInt(const std::string& fieldName)
{
	m_posAndSize[fieldName] = std::make_tuple(GetTotalOffset(), sizeof(int));
}


//-----------------------------------------------------------------------------------------------
void ShaderStorageBlock::PushBackFloat(const std::string& fieldName)
{
	m_posAndSize[fieldName] = std::make_tuple(GetTotalOffset(), sizeof(float));
}


//-----------------------------------------------------------------------------------------------
void ShaderStorageBlock::PushBackVec2(const std::string& fieldName)
{
	m_posAndSize[fieldName] = std::make_tuple(GetTotalOffset(), sizeof(Vector2));
}


//-----------------------------------------------------------------------------------------------
void ShaderStorageBlock::PushBackVec3(const std::string& fieldName)
{
	m_posAndSize[fieldName] = std::make_tuple(GetTotalOffset(), sizeof(Vector4));
}


//-----------------------------------------------------------------------------------------------
void ShaderStorageBlock::PushBackVec4(const std::string& fieldName)
{
	m_posAndSize[fieldName] = std::make_tuple(GetTotalOffset(), sizeof(Vector4));
}


//-----------------------------------------------------------------------------------------------
void ShaderStorageBlock::PushBackMat4(const std::string& fieldName)
{
	m_posAndSize[fieldName] = std::make_tuple(GetTotalOffset(), sizeof(Matrix44));
}


//-----------------------------------------------------------------------------------------------
void ShaderStorageBlock::SetInt(const std::string& blockName, const std::string& fieldName, int toSet)
{
	SetField(blockName, fieldName, (void*)&toSet);
}


//-----------------------------------------------------------------------------------------------
void ShaderStorageBlock::SetFloat(const std::string& blockName, const std::string& fieldName, float toSet)
{
	SetField(blockName, fieldName, (void*)&toSet);
}


//-----------------------------------------------------------------------------------------------
void ShaderStorageBlock::SetVec2(const std::string& blockName, const std::string& fieldName, const Vector2& toSet)
{
	SetField(blockName, fieldName, (void*)&toSet);
}


//-----------------------------------------------------------------------------------------------
void ShaderStorageBlock::SetVec3(const std::string& blockName, const std::string& fieldName, const Vector3& toSet)
{
	Vector4 paddedVec(toSet.x, toSet.y, toSet.z, 0.f);
	SetField(blockName, fieldName, (void*)&paddedVec);
}


//-----------------------------------------------------------------------------------------------
void ShaderStorageBlock::SetVec4(const std::string& blockName, const std::string& fieldName, const Vector4& toSet)
{
	SetField(blockName, fieldName, (void*)&toSet);
}


//-----------------------------------------------------------------------------------------------
void ShaderStorageBlock::SetMat4(const std::string& blockName, const std::string& fieldName, const Matrix44& toSet)
{
	SetField(blockName, fieldName, (void*)&toSet);
}


//-----------------------------------------------------------------------------------------------
int ShaderStorageBlock::GetInt(const std::string& blockName, const std::string& fieldName)
{
	int result;
	GetField(blockName, fieldName, &result);

	return result;
}


//-----------------------------------------------------------------------------------------------
float ShaderStorageBlock::GetFloat(const std::string& blockName, const std::string& fieldName)
{
	float result;
	GetField(blockName, fieldName, &result);

	return result;
}


//-----------------------------------------------------------------------------------------------
Vector2 ShaderStorageBlock::GetVec2(const std::string& blockName, const std::string& fieldName)
{
	Vector2 result;
	GetField(blockName, fieldName, &result);

	return result;
}


//-----------------------------------------------------------------------------------------------
Vector3 ShaderStorageBlock::GetVec3(const std::string& blockName, const std::string& fieldName)
{
	Vector4 result;
	GetField(blockName, fieldName, &result);

	return result.XYZ();
}


//-----------------------------------------------------------------------------------------------
Vector4 ShaderStorageBlock::GetVec4(const std::string& blockName, const std::string& fieldName)
{
	Vector4 result;
	GetField(blockName, fieldName, &result);

	return result;
}


//-----------------------------------------------------------------------------------------------
Matrix44 ShaderStorageBlock::GetMat4(const std::string& blockName, const std::string& fieldName)
{
	Matrix44 result;
	GetField(blockName, fieldName, &result);

	return result;
}