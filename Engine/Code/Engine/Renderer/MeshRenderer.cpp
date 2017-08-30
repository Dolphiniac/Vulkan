#include "Engine/Renderer/MeshRenderer.hpp"
#include "Engine/Actor/Transform.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Core/EngineSystemManager.hpp"
#include "Engine/Renderer/OpenGLExtensions.hpp"


//-----------------------------------------------------------------------------------------------
MeshRenderer::MeshRenderer(Mesh* mesh, Material* material, bool isTransient)
	: m_mesh(mesh)
	, m_material(material)
	, m_transient(isTransient)
{
	glGenVertexArrays(1, &m_vaoID);
	SetMesh(m_mesh);
}

//-----------------------------------------------------------------------------------------------
void MeshRenderer::BindMeshToVAOPCT()
{
	glBindVertexArray(m_vaoID);
	glBindBuffer(GL_ARRAY_BUFFER, m_mesh->m_vboID);

	m_material->BindProperty("inPosition", 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PCT), offsetof(Vertex_PCT, m_position));

	m_material->BindProperty("inColor", 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex_PCT), offsetof(Vertex_PCT, m_color));

	m_material->BindProperty("inUV0", 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_PCT), offsetof(Vertex_PCT, m_texCoords));

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (m_mesh->m_usingIBO)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_mesh->m_iboID);
	}

	glBindVertexArray(NULL);
}


//-----------------------------------------------------------------------------------------------
void MeshRenderer::BindMeshToVAOPCTTB()
{
	glBindVertexArray(m_vaoID);
	glBindBuffer(GL_ARRAY_BUFFER, m_mesh->m_vboID);

	m_material->BindProperty("inPosition",
		3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PCTTB), offsetof(Vertex_PCTTB, m_position));

	m_material->BindProperty("inColor",
		4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex_PCTTB), offsetof(Vertex_PCTTB, m_color));

	m_material->BindProperty("inUV0",
		2, GL_FLOAT, GL_FALSE, sizeof(Vertex_PCTTB), offsetof(Vertex_PCTTB, m_texCoords));

	m_material->BindProperty("inTangent",
		3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PCTTB), offsetof(Vertex_PCTTB, m_tangent));

	m_material->BindProperty("inBitangent",
		3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PCTTB), offsetof(Vertex_PCTTB, m_bitangent));

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (m_mesh->m_usingIBO)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_mesh->m_iboID);
	}

	glBindVertexArray(NULL);
}


//-----------------------------------------------------------------------------------------------
void MeshRenderer::BindMeshToVAOPCTTBB()
{
	glBindVertexArray(m_vaoID);
	glBindBuffer(GL_ARRAY_BUFFER, m_mesh->m_vboID);

	m_material->BindProperty("inPosition",
		3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PCTTBB), offsetof(Vertex_PCTTBB, m_position));

	m_material->BindProperty("inColor",
		4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex_PCTTBB), offsetof(Vertex_PCTTBB, m_color));

	m_material->BindProperty("inUV0",
		2, GL_FLOAT, GL_FALSE, sizeof(Vertex_PCTTBB), offsetof(Vertex_PCTTBB, m_texCoords));

	m_material->BindProperty("inTangent",
		3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PCTTBB), offsetof(Vertex_PCTTBB, m_tangent));

	m_material->BindProperty("inBitangent",
		3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PCTTBB), offsetof(Vertex_PCTTBB, m_bitangent));

	m_material->BindProperty("inBoneWeights",
		4, GL_FLOAT, GL_FALSE, sizeof(Vertex_PCTTBB), offsetof(Vertex_PCTTBB, m_boneWeights));

	m_material->BindProperty("inBoneIndices",
		4, GL_INT, GL_FALSE, sizeof(Vertex_PCTTBB), offsetof(Vertex_PCTTBB, m_boneIndices), true);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (m_mesh->m_usingIBO)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_mesh->m_iboID);
	}

	glBindVertexArray(NULL);
}


//-----------------------------------------------------------------------------------------------
void MeshRenderer::BindMeshToVAOText()
{
	glBindVertexArray(m_vaoID);
	glBindBuffer(GL_ARRAY_BUFFER, m_mesh->m_vboID);

	m_material->BindProperty("inPosition",
		3, GL_FLOAT, GL_FALSE, sizeof(Vertex_Text), offsetof(Vertex_Text, m_position));

	m_material->BindProperty("inColor",
		4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex_Text), offsetof(Vertex_Text, m_color));
	
	m_material->BindProperty("inUV0",
		2, GL_FLOAT, GL_FALSE, sizeof(Vertex_Text), offsetof(Vertex_Text, m_texCoords));

	m_material->BindProperty("inNormalizedGlyphPosition",
		2, GL_FLOAT, GL_FALSE, sizeof(Vertex_Text), offsetof(Vertex_Text, m_normalizedGlyphPosition));

	m_material->BindProperty("inNormalizedStringPosition",
		2, GL_FLOAT, GL_FALSE, sizeof(Vertex_Text), offsetof(Vertex_Text, m_normalizedStringPosition));

	m_material->BindProperty("inNormalizedFragPosition",
		1, GL_FLOAT, GL_FALSE, sizeof(Vertex_Text), offsetof(Vertex_Text, m_normalizedFragPosition));

	m_material->BindProperty("inGlyphIndex",
		1, GL_INT, GL_FALSE, sizeof(Vertex_Text), offsetof(Vertex_Text, m_glyphIndex), true);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (m_mesh->m_usingIBO)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_mesh->m_iboID);
	}

	glBindVertexArray(NULL);
}


//-----------------------------------------------------------------------------------------------
void MeshRenderer::BindMeshToVAOPCT2()
{
	glBindVertexArray(m_vaoID);
	glBindBuffer(GL_ARRAY_BUFFER, m_mesh->m_vboID);

	m_material->BindProperty("inPosition",
		2, GL_FLOAT, GL_FALSE, sizeof(Vertex2_PCT), offsetof(Vertex2_PCT, m_position));

	m_material->BindProperty("inColor",
		4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex2_PCT), offsetof(Vertex2_PCT, m_color));

	m_material->BindProperty("inUV0",
		2, GL_FLOAT, GL_FALSE, sizeof(Vertex2_PCT), offsetof(Vertex2_PCT, m_texCoords));

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (m_mesh->m_usingIBO)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_mesh->m_iboID);
	}

	glBindVertexArray(NULL);
}


//-----------------------------------------------------------------------------------------------
void MeshRenderer::UpdateModelMatrix(const Transform& transform)
{
	Matrix44 model;
	model.MakeTransformationMatrix(transform.uniformScale, transform.rotation, transform.position);

	m_material->SetUniformMatrix44("gModel", model);
}


//-----------------------------------------------------------------------------------------------
void MeshRenderer::UpdateVBO(void* data, size_t count, size_t elemSize, Renum usage)
{
	glBindVertexArray(m_vaoID);
	glBindVertexArray(NULL);
	m_mesh->UpdateVBO(data, count, elemSize, usage);
}


void MeshRenderer::SetMesh(class Mesh* mesh)
{
	m_mesh = mesh;
	switch (m_mesh->m_vertexType)
	{
	case R_VERTEX_PCTTB:
		BindMeshToVAOPCTTB();
		break;
	case R_VERTEX_PCT:
		BindMeshToVAOPCT();
		break;
	case R_VERTEX_PCTN:
		BindMeshToVAOPCTN();
		break;
	case R_VERTEX_PCTTBB:
		BindMeshToVAOPCTTBB();
		break;
	case R_VERTEX_TEXT:
		BindMeshToVAOText();
		break;
	case R_VERTEX2_PCT:
		BindMeshToVAOPCT2();
		break;
	}	
}


//-----------------------------------------------------------------------------------------------
void MeshRenderer::SetMaterial(class Material* mat)
{
	m_material = mat;
	switch (m_mesh->m_vertexType)
	{
	case R_VERTEX_PCTTB:
		BindMeshToVAOPCTTB();
		break;
	case R_VERTEX_PCT:
		BindMeshToVAOPCT();
		break;
	case R_VERTEX_PCTN:
		BindMeshToVAOPCTN();
		break;
	}
}


#include "Engine/Core/Profiler.hpp"
//-----------------------------------------------------------------------------------------------
void MeshRenderer::Render() const
{
	PROFILE_LOG_SECTION(meshRendererRender);
	if (!m_material)
		return;

	if (!m_mesh)
		return;

	glBindVertexArray(m_vaoID);
	SetUniformsOnMaterial();
	m_material->UseProgramAndBindTextures();
	m_mesh->Render();
	m_material->UnbindTextures();
	glUseProgram(NULL);
	glBindVertexArray(NULL);
	glActiveTexture(GL_TEXTURE0);
}


//-----------------------------------------------------------------------------------------------
MeshRenderer::~MeshRenderer()
{
	glDeleteVertexArrays(1, &m_vaoID);
	if (m_transient)
	{
		SAFE_DELETE(m_mesh);
	}
}

#include "Engine/Core/Profiler.hpp"
//-----------------------------------------------------------------------------------------------
void MeshRenderer::SetUniformsOnMaterial() const
{
	PROFILE_LOG_SECTION(setUniforms);
	for (auto iter = m_intUniforms.begin(); iter != m_intUniforms.end();)
	{
		bool success = m_material->SetUniformInt(iter->first, iter->second);
		if (!success)
		{
			m_intUniforms.erase(iter++);
		}
		else
		{
			iter++;
		}
	}
	for (auto iter = m_floatUniforms.begin(); iter != m_floatUniforms.end();)
	{
		bool success = m_material->SetUniformFloat(iter->first, iter->second);
		if (!success)
		{
			m_floatUniforms.erase(iter++);
		}
		else
		{
			iter++;
		}
	}
	for (auto iter = m_vec3Uniforms.begin(); iter != m_vec3Uniforms.end();)
	{
		bool success = m_material->SetUniformVec3(iter->first, iter->second);
		if (!success)
		{
			m_vec3Uniforms.erase(iter++);
		}
		else
		{
			iter++;
		}
	}
	for (auto iter = m_vec4Uniforms.begin(); iter != m_vec4Uniforms.end();)
	{
		bool success = m_material->SetUniformVec4(iter->first, iter->second);
		if (!success)
		{
			m_vec4Uniforms.erase(iter++);
		}
		else
		{
			iter++;
		}
	}
	for (auto iter = m_mat4Uniforms.begin(); iter != m_mat4Uniforms.end();)
	{
		bool success = m_material->SetUniformMatrix44(iter->first, iter->second);
		if (!success)
		{
			m_mat4Uniforms.erase(iter++);
		}
		else
		{
			iter++;
		}
	}
	for (auto iter = m_intUniformArrays.begin(); iter != m_intUniformArrays.end();)
	{
		bool success = m_material->SetUniformIntArray(iter->first, iter->second.size(), &iter->second[0]);
		if (!success)
		{
			m_intUniformArrays.erase(iter++);
		}
		else
		{
			iter++;
		}
	}
	for (auto iter = m_floatUniformArrays.begin(); iter != m_floatUniformArrays.end();)
	{
		bool success = m_material->SetUniformFloatArray(iter->first, iter->second.size(), &iter->second[0]);
		if (!success)
		{
			m_floatUniformArrays.erase(iter++);
		}
		else
		{
			iter++;
		}
	}
	for (auto iter = m_vec3UniformArrays.begin(); iter != m_vec3UniformArrays.end();)
	{
		bool success = m_material->SetUniformVec3Array(iter->first, iter->second.size(), &iter->second[0]);
		if (!success)
		{
			m_vec3UniformArrays.erase(iter++);
		}
		else
		{
			iter++;
		}
	}
	for (auto iter = m_vec4UniformArrays.begin(); iter != m_vec4UniformArrays.end();)
	{
		bool success = m_material->SetUniformVec4Array(iter->first, iter->second.size(), &iter->second[0]);
		if (!success)
		{
			m_vec4UniformArrays.erase(iter++);
		}
		else
		{
			iter++;
		}
	}
	for (auto iter = m_mat4UniformArrays.begin(); iter != m_mat4UniformArrays.end();)
	{
		bool success = m_material->SetUniformMatrix44Array(iter->first, iter->second.size(), &iter->second[0]);
		if (!success)
		{
			m_mat4UniformArrays.erase(iter++);
		}
		else
		{
			iter++;
		}
	}
	for (std::pair<const std::string, const Texture*>& p : m_samplers)
	{
		m_material->SetUniformTexture(p.first, p.second);
	}
}


//-----------------------------------------------------------------------------------------------
void MeshRenderer::BindMeshToVAOPCTN()
{
	glBindVertexArray(m_vaoID);
	glBindBuffer(GL_ARRAY_BUFFER, m_mesh->m_vboID);

	m_material->BindProperty("inPosition",
		3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PCTN), offsetof(Vertex_PCTN, m_position));

	m_material->BindProperty("inColor",
		4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex_PCTN), offsetof(Vertex_PCTN, m_color));

	m_material->BindProperty("inUV0",
		2, GL_FLOAT, GL_FALSE, sizeof(Vertex_PCTN), offsetof(Vertex_PCTN, m_texCoords));

	m_material->BindProperty("inNormal",
		3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PCTN), offsetof(Vertex_PCTN, m_normal));

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (m_mesh->m_usingIBO)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_mesh->m_iboID);
	}

	glBindVertexArray(NULL);
}

