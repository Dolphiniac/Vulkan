#pragma once

#include "Engine/Renderer/GLRenderer.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/Matrix44.hpp"

#include <map>
#include <tuple>
#include <string>


//-----------------------------------------------------------------------------------------------
class UniformBlock
{
private:
	inline UniformBlock(const std::string& name);
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

public:
	static void InitializeUniformBlocks();
	static void DestroyUniformBlocks();
	static UniformBlock* GetBlock(const std::string& blockName);
	inline static void SetInt(const std::string& blockName, const std::string& fieldName, int toSet);
	inline static void SetFloat(const std::string& blockName, const std::string& fieldName, float toSet);
	inline static void SetVec2(const std::string& blockName, const std::string& fieldName, const Vector2& toSet);
	inline static void SetVec3(const std::string& blockName, const std::string& fieldName, const Vector3& toSet);
	inline static void SetVec4(const std::string& blockName, const std::string& fieldName, const Vector4& toSet);
	inline static void SetMat4(const std::string& blockName, const std::string& fieldName, const Matrix44& toSet);

private:
	static std::map<std::string, UniformBlock*> s_uniformBlocks;
	static GLuint s_currentBindingPoint;
	std::map<std::string, std::tuple<int, int>> m_posAndSize;
	std::string m_name;

public:
	GLuint m_uboID;
	GLint m_bindingPoint;
};










//-----------------------------------------------------------------------------------------------
UniformBlock::UniformBlock(const std::string& name)
	: m_name(name)
{
	m_bindingPoint = s_currentBindingPoint++;
}


//-----------------------------------------------------------------------------------------------
void UniformBlock::PushBackInt(const std::string& fieldName)
{
	m_posAndSize[fieldName] = std::make_tuple(GetTotalOffset(), sizeof(int));
}


//-----------------------------------------------------------------------------------------------
void UniformBlock::PushBackFloat(const std::string& fieldName)
{
	m_posAndSize[fieldName] = std::make_tuple(GetTotalOffset(), sizeof(float));
}


//-----------------------------------------------------------------------------------------------
void UniformBlock::PushBackVec2(const std::string& fieldName)
{
	m_posAndSize[fieldName] = std::make_tuple(GetTotalOffset(), sizeof(Vector2));
}


//-----------------------------------------------------------------------------------------------
void UniformBlock::PushBackVec3(const std::string& fieldName)
{
	m_posAndSize[fieldName] = std::make_tuple(GetTotalOffset(), sizeof(Vector4));
}


//-----------------------------------------------------------------------------------------------
void UniformBlock::PushBackVec4(const std::string& fieldName)
{
	m_posAndSize[fieldName] = std::make_tuple(GetTotalOffset(), sizeof(Vector4));
}


//-----------------------------------------------------------------------------------------------
void UniformBlock::PushBackMat4(const std::string& fieldName)
{
	m_posAndSize[fieldName] = std::make_tuple(GetTotalOffset(), sizeof(Matrix44));
}


//-----------------------------------------------------------------------------------------------
void UniformBlock::SetInt(const std::string& blockName, const std::string& fieldName, int toSet)
{
	SetField(blockName, fieldName, (void*)&toSet);
}


//-----------------------------------------------------------------------------------------------
void UniformBlock::SetFloat(const std::string& blockName, const std::string& fieldName, float toSet)
{
	SetField(blockName, fieldName, (void*)&toSet);
}


//-----------------------------------------------------------------------------------------------
void UniformBlock::SetVec2(const std::string& blockName, const std::string& fieldName, const Vector2& toSet)
{
	SetField(blockName, fieldName, (void*)&toSet);
}


//-----------------------------------------------------------------------------------------------
void UniformBlock::SetVec3(const std::string& blockName, const std::string& fieldName, const Vector3& toSet)
{
	Vector4 paddedVec(toSet.x, toSet.y, toSet.z, 0.f);
	SetField(blockName, fieldName, (void*)&paddedVec);
}


//-----------------------------------------------------------------------------------------------
void UniformBlock::SetVec4(const std::string& blockName, const std::string& fieldName, const Vector4& toSet)
{
	SetField(blockName, fieldName, (void*)&toSet);
}


//-----------------------------------------------------------------------------------------------
void UniformBlock::SetMat4(const std::string& blockName, const std::string& fieldName, const Matrix44& toSet)
{
	SetField(blockName, fieldName, (void*)&toSet);
}