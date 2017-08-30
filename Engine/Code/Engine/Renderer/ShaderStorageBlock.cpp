#include "Engine/Renderer/ShaderStorageBlock.hpp"
#include "Engine/Core/EngineSystemManager.hpp"
#include "Engine/Renderer/OpenGLExtensions.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"


//-----------------------------------------------------------------------------------------------
GLuint ShaderStorageBlock::s_currentBindingPoint = 0;
std::map<std::string, ShaderStorageBlock*> ShaderStorageBlock::s_shaderStorageBlocks;


//-----------------------------------------------------------------------------------------------
void ShaderStorageBlock::GenerateBuffer()
{
	size_t totalSize = 0;
	for each (auto ps in m_posAndSize)
	{
		totalSize += std::get<1>(ps.second);
	}

	m_ssboID = The.Renderer->CreateRenderBuffer(nullptr, 1, totalSize, R_DYNAMIC_COPY, R_SHADER_STORAGE_BUFFER);
}


//-----------------------------------------------------------------------------------------------
std::tuple<int, int> ShaderStorageBlock::GetPosAndSize(const std::string& fieldName) const
{
	auto fieldIter = m_posAndSize.find(fieldName);
	GUARANTEE_OR_DIE(fieldIter != m_posAndSize.end(), Stringf("Field name %s is not an existing field in uniform block %s", fieldName.c_str(), m_name.c_str()));

	return fieldIter->second;
}


//-----------------------------------------------------------------------------------------------
int ShaderStorageBlock::GetTotalOffset() const
{
	int result = 0;

	for each (auto ps in m_posAndSize)
	{
		//Add the size of each existing element to the total offset
		result += std::get<1>(ps.second);
	}

	return result;
}


//-----------------------------------------------------------------------------------------------
void ShaderStorageBlock::SetField(const std::string& blockName, const std::string& fieldName, void* toSet)
{
	ShaderStorageBlock* block = GetBlock(blockName);
	std::tuple<int, int> posAndSize = block->GetPosAndSize(fieldName);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, block->m_bindingPoint, block->m_ssboID);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, std::get<0>(posAndSize), std::get<1>(posAndSize), toSet);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, NULL);
}


//-----------------------------------------------------------------------------------------------
void ShaderStorageBlock::GetField(const std::string& blockName, const std::string& fieldName, void* returnValue)
{
	ShaderStorageBlock* block = GetBlock(blockName);
	std::tuple<int, int> posAndSize = block->GetPosAndSize(fieldName);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, block->m_bindingPoint, block->m_ssboID);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, std::get<0>(posAndSize), std::get<1>(posAndSize), returnValue);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, NULL);
}


//-----------------------------------------------------------------------------------------------
ShaderStorageBlock* ShaderStorageBlock::GetBlock(const std::string& blockName)
{
	auto blockIter = s_shaderStorageBlocks.find(blockName);
	GUARANTEE_OR_DIE(blockIter != s_shaderStorageBlocks.end(), Stringf("Block name %s is not an existing shader storage block", blockName.c_str()));

	return blockIter->second;
}


//-----------------------------------------------------------------------------------------------
void ShaderStorageBlock::InitializeShaderStorageBlocks()
{
	std::string currName;
	ShaderStorageBlock* currBlock;

	currName = "TextIndex";
	currBlock = new ShaderStorageBlock(currName);
	currBlock->PushBackInt("gTextIndex");
	currBlock->GenerateBuffer();
	s_shaderStorageBlocks[currName] = currBlock;
}


//-----------------------------------------------------------------------------------------------
void ShaderStorageBlock::DestroyShaderStorageBlocks()
{
	for (std::pair<const std::string, ShaderStorageBlock*>& p : s_shaderStorageBlocks)
	{
		delete p.second;
	}

	s_shaderStorageBlocks.clear();
}