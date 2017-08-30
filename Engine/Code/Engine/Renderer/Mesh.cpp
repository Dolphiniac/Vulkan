#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Core/EngineSystemManager.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/OpenGLExtensions.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ConsoleCommand.hpp"
#include "Engine/Actor/Actor.hpp"
#include "ThirdParty/XML/xml.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/BinaryReader.hpp"
#include "Engine/Core/BinaryWriter.hpp"
#include "Engine/Core/Profiler.hpp"


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/GL.h>


//-----------------------------------------------------------------------------------------------
Mesh* Mesh::s_defaultMeshes[NUM_MESH_TYPES];
std::vector<MeshBuilder*> loadedMeshes;
int numModels;
class Actor* models = nullptr;


//-----------------------------------------------------------------------------------------------
Mesh::Mesh()
	: m_iboID(0)
	, m_vboID(0)
	, m_usingIBO(false)
	, m_numIndices(0)
	, m_numVertices(0)
	, m_drawShape(R_TRIANGLES)
	, m_vertexType(R_VERTEX_PCT)
{
}


//-----------------------------------------------------------------------------------------------
void Mesh::InitializeIBO(void* data, size_t count, size_t elemSize, Renum usage /* = R_STATIC_DRAW */)
{
	m_iboID = The.Renderer->CreateRenderBuffer(data, count, elemSize, usage);
	m_numIndices = count;
	m_usingIBO = true;
}


//-----------------------------------------------------------------------------------------------
void Mesh::InitializeVBO(void* data, size_t count, size_t elemSize, Renum usage /* = R_STATIC_DRAW */, bool shouldDeleteBuffer)
{
	if (shouldDeleteBuffer)
	{
		glDeleteBuffers(1, &m_vboID);
	}
	m_vboID = The.Renderer->CreateRenderBuffer(data, count, elemSize, usage);
	m_numVertices = count;
}


//-----------------------------------------------------------------------------------------------
void Mesh::UpdateVBO(void* data, size_t count, size_t elemSize, Renum usage)
{
	glBindBuffer(GL_ARRAY_BUFFER, m_vboID);
	glBufferData(m_vboID, count * elemSize, data, usage);
	glBindBuffer(GL_ARRAY_BUFFER, NULL);
}


//-----------------------------------------------------------------------------------------------
void Mesh::Render() const
{
	PROFILE_LOG_SECTION(renderCalls);
	if (m_usingIBO)
	{
		The.Renderer->DrawElementsThinWrapper(m_drawShape, m_numIndices);
	}
	else
	{
		The.Renderer->DrawArraysThinWrapper(m_drawShape, m_numVertices);
	}
}


//-----------------------------------------------------------------------------------------------
void Mesh::InitializeDefaultMeshes()
{
	InitBoxMeshTBN();

	InitSphereMeshTBN();

}


//-----------------------------------------------------------------------------------------------
void Mesh::DestroyDefaultMeshes()
{
	for (Mesh* m : s_defaultMeshes)
	{
		delete m;
	}
}


void Mesh::InitSphereMeshTBN()
{
	Vertex_PCTTB sphereVert;
	sphereVert.m_color = WHITE;
	std::vector<Vertex_PCTTB> sphere;

	std::vector<std::tuple<Vector4, Vector4, Vector4>> phiVecs;
	const float phiStep = 10.f;
	for (float phi = 90.f; phi >= -90.f; phi -= phiStep)
	{
		float sinPhi = SinDegrees(phi);
		float cosPhi = CosDegrees(phi);
		Vector4 vec(cosPhi, sinPhi, 0.f, 1.f);
		Vector4 bitangent(-sinPhi, cosPhi, 0.f, 1.f);
		Vector4 tangent(0.f, 0.f, 1.f, 1.f);
		phiVecs.push_back(std::make_tuple(vec, tangent, bitangent));
	}

	const float thetaStep = 10.f;
	uint16_t thetaVecs = 0;
	for (float theta = 0.f, xTexCoord = 0.f; theta <= 360.f; theta += thetaStep, xTexCoord += thetaStep / 360.f, thetaVecs++)
	{
		Matrix44 thetaMat;
		thetaMat.MakeRotationY(theta);
		float yTexCoord = 0.f;
		for (std::vector<std::tuple<Vector4, Vector4, Vector4>>::const_iterator phiIter = phiVecs.begin(); phiIter != phiVecs.end(); phiIter++, yTexCoord += phiStep / 180.f)
		{
			std::tuple<Vector4, Vector4, Vector4> ptb = *phiIter;
			Vector4 pos2 = std::get<0>(ptb);
			Vector4 tan2 = std::get<1>(ptb);
			Vector4 bitan2 = std::get<2>(ptb);

			Vector4 pos3 = pos2 * thetaMat;
			Vector4 tan3 = tan2 * thetaMat;
			Vector4 bitan3 = bitan2 * thetaMat;

			sphereVert.m_position = pos3.XYZ();
			sphereVert.m_tangent = tan3.XYZ();
			sphereVert.m_bitangent = bitan3.XYZ();
			sphereVert.m_texCoords = Vector2(xTexCoord, yTexCoord);

			sphere.push_back(sphereVert);
		}
	}

	s_defaultMeshes[UV_SPHERE] = new Mesh();
	s_defaultMeshes[UV_SPHERE]->InitializeVBO(&sphere[0], sphere.size(), sizeof(sphere[0]), R_STATIC_DRAW);
	s_defaultMeshes[UV_SPHERE]->m_numVertices = sphere.size();
	s_defaultMeshes[UV_SPHERE]->m_vertexType = R_VERTEX_PCTTB;

	std::vector<uint16_t> sphereIndices;
	for (uint16_t startIndex = 0; startIndex < thetaVecs * (phiVecs.size() - 1); startIndex += thetaVecs)
	{
		sphereIndices.push_back(startIndex);
		sphereIndices.push_back(startIndex + 1);
		sphereIndices.push_back(startIndex + thetaVecs);
		for (uint16_t index = startIndex + 1; index < startIndex + thetaVecs - 2; index++)
		{
			sphereIndices.push_back(index);
			sphereIndices.push_back(index + thetaVecs + 1);
			sphereIndices.push_back(index + thetaVecs);
			sphereIndices.push_back(index);
			sphereIndices.push_back(index + 1);
			sphereIndices.push_back(index + thetaVecs + 1);
		}
		sphereIndices.push_back(startIndex + thetaVecs - 2);
		sphereIndices.push_back(startIndex + thetaVecs - 1);
		sphereIndices.push_back(startIndex + (thetaVecs * 2) - 1);
	}

	s_defaultMeshes[UV_SPHERE]->InitializeIBO(&sphereIndices[0], sphereIndices.size(), sizeof(sphereIndices[0]), R_STATIC_DRAW);
	s_defaultMeshes[UV_SPHERE]->m_numIndices = sphereIndices.size();
}

//-----------------------------------------------------------------------------------------------
void Mesh::InitBoxMeshTBN()
{
	Vertex_PCTTB box[24] =
	{
		Vertex_PCTTB(Vector3(-.5f, -.5f, -.5f), Vector2(0.f, 1.f), WHITE, Vector3(1.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f)),
		Vertex_PCTTB(Vector3(-.5f, .5f, -.5f), Vector2(0.f, 0.f), WHITE, Vector3(1.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f)),
		Vertex_PCTTB(Vector3(.5f, -.5f, -.5f), Vector2(1.f, 1.f), WHITE, Vector3(1.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f)),
		Vertex_PCTTB(Vector3(.5f, .5f, -.5f), Vector2(1.f, 0.f), WHITE, Vector3(1.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f)),

		Vertex_PCTTB(Vector3(-.5f, -.5f, .5f), Vector2(1.f, 1.f),WHITE, Vector3(-1.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f)),
		Vertex_PCTTB(Vector3(-.5f, .5f, .5f), Vector2(1.f, 0.f), WHITE, Vector3(-1.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f)),
		Vertex_PCTTB(Vector3(.5f, -.5f, .5f), Vector2(0.f, 1.f), WHITE, Vector3(-1.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f)),
		Vertex_PCTTB(Vector3(.5f, .5f, .5f), Vector2(0.f, 0.f), WHITE, Vector3(-1.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f)),

		Vertex_PCTTB(Vector3(-.5f, -.5f, -.5f), Vector2(1.f, 1.f), WHITE, Vector3(0.f, 0.f, -1.f), Vector3(0.f, 1.f, 0.f)),
		Vertex_PCTTB(Vector3(-.5f, .5f, -.5f), Vector2(1.f, 0.f), WHITE, Vector3(0.f, 0.f, -1.f), Vector3(0.f, 1.f, 0.f)),
		Vertex_PCTTB(Vector3(-.5f, -.5f, .5f), Vector2(0.f, 1.f), WHITE, Vector3(0.f, 0.f, -1.f), Vector3(0.f, 1.f, 0.f)),
		Vertex_PCTTB(Vector3(-.5f, .5f, .5f), Vector2(0.f, 0.f), WHITE, Vector3(0.f, 0.f, -1.f), Vector3(0.f, 1.f, 0.f)),

		Vertex_PCTTB(Vector3(.5f, -.5f, -.5f), Vector2(0.f, 1.f), WHITE, Vector3(0.f, 0.f, 1.f), Vector3(0.f, 1.f, 0.f)),
		Vertex_PCTTB(Vector3(.5f, .5f, -.5f), Vector2(0.f, 0.f), WHITE, Vector3(0.f, 0.f, 1.f), Vector3(0.f, 1.f, 0.f)),
		Vertex_PCTTB(Vector3(.5f, -.5f, .5f), Vector2(1.f, 1.f), WHITE, Vector3(0.f, 0.f, 1.f), Vector3(0.f, 1.f, 0.f)),
		Vertex_PCTTB(Vector3(.5f, .5f, .5f), Vector2(1.f, 0.f), WHITE, Vector3(0.f, 0.f, 1.f), Vector3(0.f, 1.f, 0.f)),

		Vertex_PCTTB(Vector3(-.5f, .5f, -.5f), Vector2(0.f, 1.f), WHITE, Vector3(1.f, 0.f, 0.f), Vector3(0.f, 0.f, 1.f)),
		Vertex_PCTTB(Vector3(-.5f, .5f, .5f), Vector2(0.f, 0.f), WHITE, Vector3(1.f, 0.f, 0.f), Vector3(0.f, 0.f, 1.f)),
		Vertex_PCTTB(Vector3(.5f, .5f, .5f), Vector2(1.f, 0.f), WHITE, Vector3(1.f, 0.f, 0.f), Vector3(0.f, 0.f, 1.f)),
		Vertex_PCTTB(Vector3(.5f, .5f, -.5f), Vector2(1.f, 1.f), WHITE, Vector3(1.f, 0.f, 0.f), Vector3(0.f, 0.f, 1.f)),

		Vertex_PCTTB(Vector3(-.5f, -.5f, -.5f), Vector2(1.f, 1.f), WHITE, Vector3(-1.f, 0.f, 0.f), Vector3(0.f, 0.f, 1.f)),
		Vertex_PCTTB(Vector3(-.5f, -.5f, .5f), Vector2(1.f, 0.f), WHITE, Vector3(-1.f, 0.f, 0.f), Vector3(0.f, 0.f, 1.f)),
		Vertex_PCTTB(Vector3(.5f, -.5f, .5f), Vector2(0.f, 0.f), WHITE, Vector3(-1.f, 0.f, 0.f), Vector3(0.f, 0.f, 1.f)),
		Vertex_PCTTB(Vector3(.5f, -.5f, -.5f), Vector2(0.f, 1.f), WHITE, Vector3(-1.f, 0.f, 0.f), Vector3(0.f, 0.f, 1.f))
	};
	s_defaultMeshes[BOX] = new Mesh();
	s_defaultMeshes[BOX]->InitializeVBO(box, 24, sizeof(box[0]), R_STATIC_DRAW);
	s_defaultMeshes[BOX]->m_vertexType = R_VERTEX_PCTTB;
	s_defaultMeshes[BOX]->m_numVertices = 24;

	uint16_t indices[] =
	{
		0, 2, 1,
		2, 3, 1,
		6, 4, 7,
		4, 5, 7,
		10, 8, 11,
		8, 9, 11,
		12, 14, 13,
		14, 15, 13,
		16, 19, 17,
		19, 18, 17,
		21, 22, 20,
		22, 23, 20
	};
	s_defaultMeshes[BOX]->InitializeIBO(indices, 36, sizeof(indices[0]), R_STATIC_DRAW);
	s_defaultMeshes[BOX]->m_numIndices = 36;
}


#ifndef __USING_UWP
//-----------------------------------------------------------------------------------------------
std::string GetDiffuseNameFromXML(const std::string& saveFileName, const std::string& matName)
{
	std::string filepath = "Data/MaterialMappings/" + saveFileName + ".xml";
	std::string texPath = "Data/Textures/" + saveFileName + "/";
	XMLNode root = XMLNode::parseFile(filepath.c_str());
	root = root.getChildNode();
	int searchHandle = 0;
	for (int i = 0; i < root.nChildNode("Mapping"); i++)
	{
		XMLNode mapping = root.getChildNode("Mapping", &searchHandle);
		if (!strcmp(mapping.getAttribute("material"), matName.c_str()))
		{
			return texPath + mapping.getAttribute("diffuse");
		}
	}

	return "";
}

//-----------------------------------------------------------------------------------------------
std::string GetNormalNameFromXML(const std::string& saveFileName, const std::string& matName)
{
	std::string filepath = "Data/MaterialMappings/" + saveFileName + ".xml";
	std::string texPath = "Data/Textures/" + saveFileName + "/";
	XMLNode root = XMLNode::parseFile(filepath.c_str());
	root = root.getChildNode();
	const char* defaultNormal = root.getAttribute("normal");
	int searchHandle = 0;
	for (int i = 0; i < root.nChildNode("Mapping"); i++)
	{
		XMLNode mapping = root.getChildNode("Mapping", &searchHandle);
		if (!strcmp(mapping.getAttribute("material"), matName.c_str()))
		{
			const char* normal = mapping.getAttribute("normal");
			if (!normal)
			{
				return texPath + defaultNormal;
			}
			else
			{
				return texPath + normal;
			}
		}
	}

	return "";
}
#endif

// CONSOLE_COMMAND(MeshLoad, args)
// {
// 	std::string filename = args.GetNextArg();
// 	std::string filePath = "Data/Models/" + filename + ".model";
// 	if (FindFilesWith("Data/Models", filename + ".model").empty())
// 	{
// 		ConsolePrintf(RED, "File %s does not exist", filePath.c_str());
// 		return;
// 	}
// 	loadedMeshes = MeshBuilder::ReadFromFile(filePath);
// 	numModels = loadedMeshes.size();
// 	models = new Actor[numModels];
// 	Transform t;
// 	//t.position.z = 2.f;
// 	for (int i = 0; i < numModels; i++)
// 	{
// 		Material* mat = new Material("basicLights");
// 		mat->BindUniformBlock("GlobalMatrices");
// 		mat->SetUniformVec4("gAmbientLight", Vector4(1.f, 1.f, 1.f, .2f));
// 		mat->SetUniformFloat("gMaxDistance", 3.f);
// 		std::string diffuseName = GetDiffuseNameFromXML(filename, loadedMeshes[i]->m_materialName);
// 		mat->SetUniformTexture("gDiffuseTex", Texture::CreateOrGetTexture(diffuseName));
// 		std::string normalName = GetNormalNameFromXML(filename, loadedMeshes[i]->m_materialName);
// 		mat->SetUniformTexture("gNormalTex", Texture::CreateOrGetTexture(normalName));
// 		Mesh* thisMesh = loadedMeshes[i]->ConstructMesh(R_VERTEX_PCTTB);
// 		models[i].m_meshRenderer = new MeshRenderer(thisMesh, mat);
// 		models[i].m_transform = t;
// 	}
// }

CONSOLE_COMMAND(MeshSave, args)
{
	std::string filename = args.GetNextArg();
	std::string filepath = "Data/Actors/" + filename + "/" + filename + ".model";
	BinaryWriter writer;
	for (MeshBuilder* mb : loadedMeshes)
	{
		MeshBuilder::Write(mb, writer);
	}
	writer.WriteBufferToFile(filepath);
	ConsolePrintf(WHITE, "Success!  Mesh saved to %s", filepath.c_str());
}


//-----------------------------------------------------------------------------------------------
Mesh::~Mesh()
{
	glDeleteBuffers(1, &m_vboID);
}