#include "Engine/Renderer/Material.hpp"
#include "Engine/Core/EngineSystemManager.hpp"
#include "Engine/Renderer/OpenGLExtensions.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/UniformBlock.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/ShaderStorageBlock.hpp"

#include <algorithm>


//-----------------------------------------------------------------------------------------------
std::map<std::string, Material*> Material::s_materialRegistry;


//-----------------------------------------------------------------------------------------------
Material::Material(const std::string& vertexShaderName, const std::string& fragmentShaderName, bool usingFiles /*= true*/)
	: m_samplerID(0)
	, m_shaderProgramID(0)
{
	std::string vertexShaderPath = Stringf("Data/Shaders/%s.vert", vertexShaderName.c_str());
	std::string fragmentShaderPath = Stringf("Data/Shaders/%s.frag", fragmentShaderName.c_str());

	GLuint vs = LoadShader((usingFiles) ? vertexShaderPath : vertexShaderName, R_VERTEX_SHADER, usingFiles);
	GLuint fs = LoadShader((usingFiles) ? fragmentShaderPath : fragmentShaderName, R_FRAGMENT_SHADER, usingFiles);

	m_shaderProgramID = CreateAndLinkProgram(vs, fs, fragmentShaderPath);

	glDeleteShader(vs);
	glDeleteShader(fs);

	CreateSampler(R_NEAREST, R_NEAREST, R_REPEAT, R_REPEAT);
}


//-----------------------------------------------------------------------------------------------
Material::Material(const Material* otherMat)
{
	m_samplerID = otherMat->m_samplerID;
	m_shaderProgramID = otherMat->m_shaderProgramID;
}


//-----------------------------------------------------------------------------------------------
void Material::CreateSampler(Renum minFilter, Renum magFilter, Renum uWrap, Renum vWrap)
{
	GLuint samplerID;
	glGenSamplers(1, &samplerID);
	ASSERT_OR_DIE(samplerID, "Couldn't create sampler");

	glSamplerParameteri(samplerID, GL_TEXTURE_MIN_FILTER, minFilter);
	glSamplerParameteri(samplerID, GL_TEXTURE_MAG_FILTER, magFilter);
	glSamplerParameteri(samplerID, GL_TEXTURE_WRAP_S, uWrap);
	glSamplerParameteri(samplerID, GL_TEXTURE_WRAP_T, vWrap);

	m_samplerID = samplerID;
}


//-----------------------------------------------------------------------------------------------
Material::Material(const std::string& uniformShaderName)
{
	*this = Material(uniformShaderName, uniformShaderName);
}


//-----------------------------------------------------------------------------------------------
GLuint Material::LoadShader(const std::string& filename, GLenum shaderType, bool usingFiles)
{
	GLuint shaderID = glCreateShader(shaderType);
	ASSERT_OR_DIE(shaderID, "Couldn't create shader");
	std::vector<char> buffer;
	if (usingFiles)
	{
		LoadBinaryFileToBuffer(filename, buffer);
		char* glBuff = &buffer[0];


		GLint srcLen = buffer.size();
		glShaderSource(shaderID, 1, &glBuff, &srcLen);
	}
	else
	{
		const char* glBuff = filename.c_str();
		GLint srcLen = filename.size();
		glShaderSource(shaderID, 1, &glBuff, &srcLen);
	}

	glCompileShader(shaderID);

	GLint status;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);
	if (!status)
	{
		The.Renderer->LogShaderError(shaderID, filename);
		glDeleteShader(shaderID);
		return 0;
	}

	return shaderID;
}


//-----------------------------------------------------------------------------------------------
GLuint Material::CreateAndLinkProgram(GLuint vs, GLuint fs, const std::string& fragFile)
{
	GLuint programID = glCreateProgram();
	ASSERT_OR_DIE(programID, "Couldn't create program");

	glAttachShader(programID, vs);
	glAttachShader(programID, fs);

	glLinkProgram(programID);

	GLint status;
	glGetProgramiv(programID, GL_LINK_STATUS, &status);

	if (status)
	{
		glDetachShader(programID, vs);
		glDetachShader(programID, fs);
	}
	else
	{
		The.Renderer->LogProgramError(programID, fragFile);
		glDeleteProgram(programID);
		return NULL;
	}

	return programID;
}


//-----------------------------------------------------------------------------------------------
bool Material::SetUniformFloat(const std::string& uniformName, float toSet)
{
	glUseProgram(m_shaderProgramID);
	GLint loc = glGetUniformLocation(m_shaderProgramID, uniformName.c_str());

	if (loc >= 0)
	{
		glUniform1fv(loc, 1, (GLfloat*)&toSet);
		glUseProgram(NULL);
		return true;
	}

	glUseProgram(NULL);
	return false;
}


//-----------------------------------------------------------------------------------------------
bool Material::SetUniformInt(const std::string& uniformName, int toSet)
{
	glUseProgram(m_shaderProgramID);
	GLint loc = glGetUniformLocation(m_shaderProgramID, uniformName.c_str());

	if (loc >= 0)
	{
		glUniform1iv(loc, 1, (GLint*)&toSet);
		glUseProgram(NULL);
		return true;
	}

	glUseProgram(NULL);
	return false;
}


//-----------------------------------------------------------------------------------------------
bool Material::SetUniformVec3(const std::string& uniformName, const Vector3& toSet)
{
	glUseProgram(m_shaderProgramID);
	GLint loc = glGetUniformLocation(m_shaderProgramID, uniformName.c_str());

	if (loc >= 0)
	{
		glUniform3fv(loc, 1, (GLfloat*)&toSet);
		glUseProgram(NULL);
		return true;
	}

	glUseProgram(NULL);
	return false;
}


//-----------------------------------------------------------------------------------------------
bool Material::SetUniformVec4(const std::string& uniformName, const Vector4& toSet)
{
	glUseProgram(m_shaderProgramID);
	GLint loc = glGetUniformLocation(m_shaderProgramID, uniformName.c_str());

	if (loc >= 0)
	{
		glUniform4fv(loc, 1, (GLfloat*)&toSet);
		glUseProgram(NULL);
		return true;
	}

	glUseProgram(NULL);
	return false;
}


//-----------------------------------------------------------------------------------------------
bool Material::SetUniformMatrix44(const std::string& uniformName, const class Matrix44& toSet)
{
	glUseProgram(m_shaderProgramID);
	GLint loc = glGetUniformLocation(m_shaderProgramID, uniformName.c_str());

	if (loc >= 0)
	{
		glUniformMatrix4fv(loc, 1, GL_TRUE, (GLfloat*)&toSet);
		glUseProgram(NULL);
		return true;
	}

	glUseProgram(NULL);
	return false;
}


//-----------------------------------------------------------------------------------------------
bool Material::SetUniformFloatArray(const std::string& arrayName, int numElements, float* data)
{
	glUseProgram(m_shaderProgramID);
	std::string uniformName = arrayName + "[0]";
	GLint loc = glGetUniformLocation(m_shaderProgramID, uniformName.c_str());

	if (loc >= 0)
	{
		glUniform1fv(loc, numElements, (GLfloat*)data);
		glUseProgram(NULL);
		return true;
	}

	glUseProgram(NULL);
	return false;
}


//-----------------------------------------------------------------------------------------------
bool Material::SetUniformIntArray(const std::string& arrayName, int numElements, int* data)
{
	glUseProgram(m_shaderProgramID);
	std::string uniformName = arrayName + "[0]";
	GLint loc = glGetUniformLocation(m_shaderProgramID, uniformName.c_str());

	if (loc >= 0)
	{
		glUniform1iv(loc, numElements, (GLint*)data);
		glUseProgram(NULL);
		return true;
	}

	glUseProgram(NULL);
	return false;
}


//-----------------------------------------------------------------------------------------------
bool Material::SetUniformVec3Array(const std::string& arrayName, int numElements, Vector3* data)
{
	glUseProgram(m_shaderProgramID);
	std::string uniformName = arrayName + "[0]";
	GLint loc = glGetUniformLocation(m_shaderProgramID, uniformName.c_str());

	if (loc >= 0)
	{
		glUniform3fv(loc, numElements, (GLfloat*)data);
		glUseProgram(NULL);
		return true;
	}

	glUseProgram(NULL);
	return false;
}


//-----------------------------------------------------------------------------------------------
bool Material::SetUniformVec4Array(const std::string& arrayName, int numElements, Vector4* data)
{
	glUseProgram(m_shaderProgramID);
	std::string uniformName = arrayName + "[0]";
	GLint loc = glGetUniformLocation(m_shaderProgramID, uniformName.c_str());

	if (loc >= 0)
	{
		glUniform4fv(loc, numElements, (GLfloat*)data);
		glUseProgram(NULL);
		return true;
	}

	glUseProgram(NULL);
	return false;
}


//-----------------------------------------------------------------------------------------------
bool Material::SetUniformMatrix44Array(const std::string& arrayName, int numElements, Matrix44* data)
{
	glUseProgram(m_shaderProgramID);
	std::string uniformName = arrayName + "[0]";
	GLint loc = glGetUniformLocation(m_shaderProgramID, uniformName.c_str());

	if (loc >= 0)
	{
		glUniformMatrix4fv(loc, numElements, GL_TRUE, (GLfloat*)data);
		glUseProgram(NULL);
		return true;
	}

	glUseProgram(NULL);
	return false;
}


//-----------------------------------------------------------------------------------------------
void Material::SetUniformTexture(const std::string& samplerName, const Texture* tex)
{
	m_textures.erase(std::remove_if(m_textures.begin(), m_textures.end(),
		[&](std::tuple<std::string, const Texture*> uniform)
	{
		std::string name = std::get<std::string>(uniform);
		return name == samplerName;
	}), m_textures.end());
	m_textures.push_back(std::make_tuple(samplerName, tex));
}


//-----------------------------------------------------------------------------------------------
void Material::SetMatrices(const Matrix44& model, const Matrix44& view, const Matrix44& projection)
{
	SetUniformMatrix44("gModel", model);
	SetUniformMatrix44("gView", view);
	SetUniformMatrix44("gProj", projection);
}


//-----------------------------------------------------------------------------------------------
void Material::BindProperty(const std::string& name, GLint count, GLenum type, GLboolean normalize, GLsizei stride, GLsizei offset, bool isInteger)
{
	GLint posBind = glGetAttribLocation(m_shaderProgramID, name.c_str());
	if (posBind >= 0)
	{
		glEnableVertexAttribArray(posBind);
		if (isInteger)
		{
			glVertexAttribIPointer(posBind, count, type, stride, (GLvoid*)offset);
		}
		else
		{
			glVertexAttribPointer(posBind, count, type, normalize, stride, (GLvoid*)offset);
		}
	}
}

#include "Engine/Core/Profiler.hpp"
//-----------------------------------------------------------------------------------------------
void Material::UseProgramAndBindTextures()
{
	PROFILE_LOG_SECTION(bindTextures);
	int texIndex = 0;
	for (auto texIter = m_textures.begin(); texIter != m_textures.end(); texIter++, texIndex++)
	{
		glActiveTexture(GL_TEXTURE0 + texIndex);
		const Texture* currTex = std::get<const Texture*>(*texIter);
		The.Renderer->BindTexture(currTex);
		glBindSampler(texIndex, m_samplerID);
		SetUniformInt(std::get<std::string>(*texIter), texIndex);
	}
	The.Renderer->UseProgram(m_shaderProgramID);
}


//-----------------------------------------------------------------------------------------------
void Material::UnbindTextures()
{
	int texIndex = 0;
	Texture* notex = Texture::CreateOrGetTexture("NoTex");
	for (auto texIter = m_textures.begin(); texIter != m_textures.end(); texIter++, texIndex++)
	{
		glActiveTexture(GL_TEXTURE0 + texIndex);
		The.Renderer->BindTexture(notex);
	}
}


//-----------------------------------------------------------------------------------------------
void Material::BindUniformBlock(const std::string& uniformBlockName)
{
	UniformBlock* block = UniformBlock::GetBlock(uniformBlockName);
	int blockIndex = glGetUniformBlockIndex(m_shaderProgramID, uniformBlockName.c_str());
	glUniformBlockBinding(m_shaderProgramID, blockIndex, block->m_bindingPoint);
}


//-----------------------------------------------------------------------------------------------
void Material::BindShaderStorageBlock(const std::string& shaderStorageBlockName)
{
	ShaderStorageBlock* block = ShaderStorageBlock::GetBlock(shaderStorageBlockName);
	int blockIndex = glGetProgramResourceIndex(m_shaderProgramID, GL_SHADER_STORAGE_BUFFER, shaderStorageBlockName.c_str());
	glShaderStorageBlockBinding(m_shaderProgramID, blockIndex, block->m_bindingPoint);
}


//-----------------------------------------------------------------------------------------------
void Material::KillProgram()
{
	glDeleteProgram(m_shaderProgramID);
}


//-----------------------------------------------------------------------------------------------
STATIC Material* Material::CreateOrGetMaterial(const std::string& uniformShaderName)
{
	auto matIter = s_materialRegistry.find(uniformShaderName);
	
	if (matIter == s_materialRegistry.end())
	{
		Material* newMat = new Material(uniformShaderName);
		
		s_materialRegistry[uniformShaderName] = newMat;

		return newMat;
	}

	return matIter->second;
}


//-----------------------------------------------------------------------------------------------
void Material::DestroyMaterials()
{
	for (std::pair<const std::string, Material*>& mat : s_materialRegistry)
	{
		SAFE_DELETE(mat.second);
	}
	s_materialRegistry.clear();
}