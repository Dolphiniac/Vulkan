#include "Engine/Model/MeshBuilder.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "ThirdParty/MikkT/mikktspace.h"
#include "Engine/Core/BinaryWriter.hpp"
#include "Engine/Core/BinaryReader.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Text/TextBox.hpp"
#include "Quantum/Hephaestus/VertexType.h"

#pragma warning(disable: 4700)


//-----------------------------------------------------------------------------------------------
void MeshBuilder::AddVertex(const Vector3& position)
{
	m_mask |= (1 << POSITION);
	Vertex_Master toAdd;
	toAdd.m_position = position;
	toAdd.m_color = m_color;
	toAdd.m_texCoords = m_uv;
	toAdd.m_tangent = m_tangent;
	toAdd.m_bitangent = m_bitangent;
	toAdd.m_normal = m_normal;
	toAdd.m_boneIndices = m_boneIndices;
	toAdd.m_boneWeights = m_boneWeights;
	toAdd.m_normalizedGlyphCoords = m_normalizedGlyphCoords;
	toAdd.m_normalizedStringCoords = m_normalizedStringCoords;
	toAdd.m_normalizedFragCoords = m_normalizedFragCoords;
	toAdd.m_glyphIndex = m_glyphIndex;
	toAdd.m_floatColor = m_floatColor;

	m_vertices.push_back(toAdd);
}


//-----------------------------------------------------------------------------------------------
void MeshBuilder::AddQuadIndices(int topLeft, int topRight, int bottomLeft, int bottomRight)
{
	m_indices.push_back((unsigned short)topLeft);
	m_indices.push_back((unsigned short)bottomLeft);
	m_indices.push_back((unsigned short)bottomRight);
	m_indices.push_back((unsigned short)bottomRight);
	m_indices.push_back((unsigned short)topRight);
	m_indices.push_back((unsigned short)topLeft);
	m_usingIbo = true;

	m_mask |= (1 << INDEX);
}


//-----------------------------------------------------------------------------------------------
MeshBuilder* MeshBuilder::Combine(MeshBuilder** meshBuilders, int numBuilders)
{
	ASSERT_OR_DIE(numBuilders >= 0, "Bad MeshBuilder array");
	std::string name = meshBuilders[0]->m_name;
	MeshBuilder* result = new MeshBuilder(name);
	result->Begin(meshBuilders[0]->m_drawMode, meshBuilders[0]->m_usingIbo);

	for (int i = 0; i < numBuilders; i++)
	{
		*result += meshBuilders[i];
	}

	return result;
}


//-----------------------------------------------------------------------------------------------
int MeshBuilder::MikkTGetNumFaces(const struct SMikkTSpaceContext* context)
{
	MeshBuilder* mb = (MeshBuilder*)context->m_pUserData;
	return mb->m_vertices.size() / 3;
}


//-----------------------------------------------------------------------------------------------
int MeshBuilder::MikkTGetNumVerticesOnFace(const struct SMikkTSpaceContext* context, int iFace)
{
	context; iFace;
	return 3;
}


//-----------------------------------------------------------------------------------------------
void MeshBuilder::MikkTGetPosition(const struct SMikkTSpaceContext* context, float* fvPosOut, int iFace, int iVert)
{
	MeshBuilder* mb = (MeshBuilder*)context->m_pUserData;
	int vertIndex = iFace * 3 + iVert;
	memcpy(fvPosOut, &mb->m_vertices[vertIndex].m_position, sizeof(Vector3));
}


//-----------------------------------------------------------------------------------------------
void MeshBuilder::MikkTGetUV(const struct SMikkTSpaceContext* context, float* fvTexcOut, int iFace, int iVert)
{
	MeshBuilder* mb = (MeshBuilder*)context->m_pUserData;
	int vertIndex = iFace * 3 + iVert;
	memcpy(fvTexcOut, &mb->m_vertices[vertIndex].m_texCoords, sizeof(Vector2));
}


//-----------------------------------------------------------------------------------------------
void MeshBuilder::MikkTGetNormal(const struct SMikkTSpaceContext* context, float* fvNormOut, int iFace, int iVert)
{
	MeshBuilder* mb = (MeshBuilder*)context->m_pUserData;
	int vertIndex = iFace * 3 + iVert;
	memcpy(fvNormOut, &mb->m_vertices[vertIndex].m_normal, sizeof(Vector3));
}


//-----------------------------------------------------------------------------------------------
void MeshBuilder::MikkTSetTangent(const struct SMikkTSpaceContext* context, const float* fvTangent, float fSign, int iFace, int iVert)
{
	MeshBuilder* mb = (MeshBuilder*)context->m_pUserData;
	int vertIndex = iFace * 3 + iVert;
	float* tanLoc = (float*)&mb->m_vertices[vertIndex].m_tangent;
	memcpy(tanLoc, fvTangent, sizeof(Vector3));
	mb->m_vertices[vertIndex].m_bitanSign = fSign;
}


//-----------------------------------------------------------------------------------------------
void MeshBuilder::GenerateTangentSpace(bool overrideTangent)
{
	if ((m_mask & (1 << TANGENT)) != 0 && !overrideTangent)
	{
		return;
	}
	if ((m_mask & (1 << NORMAL)) == 0)
	{
		ERROR_RECOVERABLE("Can't generate tangent space without normals");
	}
	SMikkTSpaceInterface mikkT;
	mikkT.m_getNumFaces = MikkTGetNumFaces;
	mikkT.m_getNumVerticesOfFace = MikkTGetNumVerticesOnFace;
	mikkT.m_getPosition = MikkTGetPosition;
	mikkT.m_getTexCoord = MikkTGetUV;
	mikkT.m_getNormal = MikkTGetNormal;
	mikkT.m_setTSpaceBasic = MikkTSetTangent;
	mikkT.m_setTSpace = nullptr;

	SMikkTSpaceContext context;
	context.m_pUserData = this;
	context.m_pInterface = &mikkT;

	genTangSpaceDefault(&context);
	for (Vertex_Master& vm : m_vertices)
	{
		vm.m_bitangent = vm.m_bitanSign * Vector3::Cross(vm.m_normal, vm.m_tangent);
	}

	m_mask |= (1 << TANGENT);
	m_mask |= (1 << BITANGENT);
}


//-----------------------------------------------------------------------------------------------
void MeshBuilder::Write(MeshBuilder* mb, BinaryWriter& writer)
{
	writer.Write(mb->m_name);
	writer.Write(MESH_BUILDER_VERSION);
	writer.Write(mb->m_mask);
	writer.Write((int)mb->m_drawMode);
	writer.Write((int)mb->m_vertices.size());
	writer.Write((int)mb->m_indices.size());
	for (Vertex_Master& vm : mb->m_vertices)
	{
		if ((mb->m_mask & (1 << POSITION)) != 0)
		{
			writer.Write(vm.m_position);
		}
		if ((mb->m_mask & (1 << TEX_COORDS)) != 0)
		{
			writer.Write(vm.m_texCoords);
		}
		if ((mb->m_mask & (1 << COLOR)) != 0)
		{
			writer.Write(vm.m_color);
		}
		if ((mb->m_mask & (1 << TANGENT)) != 0)
		{
			writer.Write(vm.m_tangent);
		}
		if ((mb->m_mask & (1 << BITANGENT)) != 0)
		{
			writer.Write(vm.m_bitangent);
		}
		if ((mb->m_mask & (1 << NORMAL)) != 0)
		{
			writer.Write(vm.m_normal);
		}
		if ((mb->m_mask & (1 << BONE)) != 0)
		{
			writer.Write(vm.m_boneWeights);
			writer.Write(vm.m_boneIndices);
		}
	}
	if ((mb->m_mask & (1 << INDEX)) != 0)
	{
		for (unsigned short us : mb->m_indices)
		{
			writer.Write(us);
		}
	}
	if ((mb->m_mask & (1 << MATERIAL)) != 0)
	{
		writer.Write(mb->m_materialName);
	}
}


//-----------------------------------------------------------------------------------------------
std::vector<MeshBuilder*> MeshBuilder::ReadFromFile(const std::string& filePath)
{
	BinaryReader reader(filePath);
	std::vector<MeshBuilder*> result;
	for (;;)
	{
		if (reader.IsEmpty())
		{
			break;
		}
		std::string name;
		reader.Read(name);
		MeshBuilder* thisBuilder = new MeshBuilder(name);
		reader.Read(thisBuilder->m_version);
		reader.Read(thisBuilder->m_mask);
		int drawMode;
		reader.Read(drawMode);
		thisBuilder->m_drawMode = (unsigned int)drawMode;
		int vertSize;
		int indSize;
		reader.Read(vertSize);
		reader.Read(indSize);
		thisBuilder->m_usingIbo = indSize > 0;

		thisBuilder->m_vertices.reserve(vertSize);
		thisBuilder->m_vertices.reserve(indSize);
		for (int i = 0; i < vertSize; i++)
		{
			Vertex_Master vm;
			if ((thisBuilder->m_mask & (1 << POSITION)) != 0)
			{
				reader.Read(vm.m_position);
			}
			if ((thisBuilder->m_mask & (1 << TEX_COORDS)) != 0)
			{
				reader.Read(vm.m_texCoords);
			}
			if ((thisBuilder->m_mask & (1 << COLOR)) != 0)
			{
				reader.Read(vm.m_color);
			}
			if ((thisBuilder->m_mask & (1 << TANGENT)) != 0)
			{
				reader.Read(vm.m_tangent);
			}
			if ((thisBuilder->m_mask & (1 << BITANGENT)) != 0)
			{
				reader.Read(vm.m_bitangent);
			}
			if ((thisBuilder->m_mask & (1 << NORMAL)) != 0)
			{
				reader.Read(vm.m_normal);
			}
			if ((thisBuilder->m_mask & (1 << BONE)) != 0)
			{
				reader.Read(vm.m_boneWeights);
				reader.Read(vm.m_boneIndices);
			}
			thisBuilder->m_vertices.push_back(vm);
		}
		if ((thisBuilder->m_mask & (1 << INDEX)) != 0)
		{
			for (int i = 0; i < indSize; i++)
			{
				unsigned short us;
				reader.Read(us);
				thisBuilder->m_indices.push_back(us);
			}
		}
		if ((thisBuilder->m_mask & (1 << MATERIAL)) != 0)
		{
			reader.Read(thisBuilder->m_materialName);
		}


		result.push_back(thisBuilder);
	}

	return result;
}


//-----------------------------------------------------------------------------------------------
void MeshBuilder::operator+=(MeshBuilder* other)
{
	for (Vertex_Master vm : other->m_vertices)
	{
		m_vertices.push_back(vm);
	}

	for (unsigned short us : other->m_indices)
	{
		m_indices.push_back(us);
	}
}


//-----------------------------------------------------------------------------------------------
Mesh* MeshBuilder::ConstructMesh(Renum vertexType)
{
	Mesh* result = new Mesh();
	
	switch (vertexType)
	{
	case R_VERTEX_PCT:
		CopyDataPCT(result);
		break;
	case R_VERTEX_PCTTB:
		CopyDataPCTTB(result);
		break;
	case R_VERTEX_PCTN:
		CopyDataPCTN(result);
		break;
	case R_VERTEX_PCTTBB:
		CopyDataPCTTBB(result);
		break;
	case R_VERTEX_TEXT:
		CopyDataText(result);
		break;
	default:
		ERROR_AND_DIE("Unsupported vertex type");
	}

	if (m_usingIbo)
	{
		result->InitializeIBO(&m_indices[0], m_indices.size(), sizeof(m_indices[0]));
		result->m_numIndices = m_indices.size();
	}

	return result;
}


//-----------------------------------------------------------------------------------------------
void MeshBuilder::CopyDataPCT(class Mesh* mesh)
{
	int numVertices = m_vertices.size();

	unsigned char* buffer = new unsigned char[sizeof(Vertex_PCT) * numVertices];

	for (int i = 0; i < numVertices; i++)
	{
		Vertex_Master pureVertex = m_vertices[i];
		Vertex_PCT vertex;
		vertex.m_position = pureVertex.m_position;
		vertex.m_color = pureVertex.m_color;
		vertex.m_texCoords = pureVertex.m_texCoords;
		memcpy(buffer + i * sizeof(Vertex_PCT), &vertex, sizeof(Vertex_PCT));
	}
	mesh->InitializeVBO(buffer, numVertices, sizeof(Vertex_PCT));
	mesh->m_numVertices = numVertices;
	mesh->m_vertexType = R_VERTEX_PCT;
}


//-----------------------------------------------------------------------------------------------
void MeshBuilder::CopyDataPCTTB(class Mesh* mesh)
{
	int numVertices = m_vertices.size();

	unsigned char* buffer = new unsigned char[sizeof(Vertex_PCTTB) * numVertices];

	for (int i = 0; i < numVertices; i++)
	{
		Vertex_Master pureVertex = m_vertices[i];
		Vertex_PCTTB vertex;
		vertex.m_position = pureVertex.m_position;
		vertex.m_color = pureVertex.m_color;
		vertex.m_texCoords = pureVertex.m_texCoords;
		vertex.m_tangent = pureVertex.m_tangent;
		vertex.m_bitangent = pureVertex.m_bitangent;
		memcpy(buffer + i * sizeof(Vertex_PCTTB), &vertex, sizeof(Vertex_PCTTB));
	}
	mesh->InitializeVBO(buffer, numVertices, sizeof(Vertex_PCTTB));
	mesh->m_numVertices = numVertices;
	mesh->m_vertexType = R_VERTEX_PCTTB;
}


//-----------------------------------------------------------------------------------------------
void MeshBuilder::CopyDataPCTN(class Mesh* mesh)
{
	int numVertices = m_vertices.size();

	unsigned char* buffer = new unsigned char[sizeof(Vertex_PCTN) * numVertices];

	for (int i = 0; i < numVertices; i++)
	{
		Vertex_Master pureVertex = m_vertices[i];
		Vertex_PCTN vertex;
		vertex.m_position = pureVertex.m_position;
		vertex.m_color = pureVertex.m_color;
		vertex.m_texCoords = pureVertex.m_texCoords;
		vertex.m_normal = pureVertex.m_normal;
		memcpy(buffer + i * sizeof(Vertex_PCTN), &vertex, sizeof(Vertex_PCTN));
	}
	mesh->InitializeVBO(buffer, numVertices, sizeof(Vertex_PCTN));
	mesh->m_numVertices = numVertices;
	mesh->m_vertexType = R_VERTEX_PCTN;
}


//-----------------------------------------------------------------------------------------------
void MeshBuilder::CopyDataPCTTBB(class Mesh* mesh)
{
	int numVertices = m_vertices.size();
	unsigned char* buffer = new unsigned char[sizeof(Vertex_PCTTBB) * numVertices];

	for (int i = 0; i < numVertices; i++)
	{
		Vertex_Master pureVertex = m_vertices[i];
		Vertex_PCTTBB vertex;
		vertex.m_position = pureVertex.m_position;
		vertex.m_color = pureVertex.m_color;
		vertex.m_texCoords = pureVertex.m_texCoords;
		vertex.m_tangent = pureVertex.m_tangent;
		vertex.m_bitangent = pureVertex.m_bitangent;
		vertex.m_boneWeights = pureVertex.m_boneWeights;
		vertex.m_boneIndices = pureVertex.m_boneIndices;
		memcpy(buffer + i * sizeof(Vertex_PCTTBB), &vertex, sizeof(Vertex_PCTTBB));
	}
	mesh->InitializeVBO(buffer, numVertices, sizeof(Vertex_PCTTBB));
	mesh->m_numVertices = numVertices;
	mesh->m_vertexType = R_VERTEX_PCTTBB;
	delete[] buffer;
}


//-----------------------------------------------------------------------------------------------
void MeshBuilder::CopyDataText(class Mesh* mesh)
{
	int numVertices = m_vertices.size();
	unsigned char* buffer = new unsigned char[sizeof(Vertex_Text) * numVertices];
	
	for (int i = 0; i < numVertices; i++)
	{
		Vertex_Master pureVertex = m_vertices[i];
		Vertex_Text vertex;
		vertex.m_position = pureVertex.m_position;
		vertex.m_color = pureVertex.m_color;
		vertex.m_texCoords = pureVertex.m_texCoords;
		vertex.m_normalizedGlyphPosition = pureVertex.m_normalizedGlyphCoords;
		vertex.m_normalizedStringPosition = pureVertex.m_normalizedStringCoords;
		vertex.m_normalizedFragPosition = pureVertex.m_normalizedFragCoords;
		vertex.m_glyphIndex = pureVertex.m_glyphIndex;
		memcpy(buffer + i * sizeof(Vertex_Text), &vertex, sizeof(Vertex_Text));
	}

	mesh->InitializeVBO(buffer, numVertices, sizeof(Vertex_Text));
	mesh->m_numVertices = numVertices;
	mesh->m_vertexType = R_VERTEX_TEXT;
}


//-----------------------------------------------------------------------------------------------
void MeshBuilder::CopyInterleavedMeshData(EVertexType vertexType, void** ppOutVerts, uint32* pOutNumVerts, uint32* pOutDataSize)
{
	ASSERT_OR_DIE(ppOutVerts, "Expected storage for vertices\n");

	uint32 numVerts = m_vertices.size();

	if (pOutNumVerts)
	{
		*pOutNumVerts = numVerts;
	}
	uint32 stride;
	std::vector<uint32> offsetsTarget;
	std::vector<uint32> offsetsSource;
	std::vector<uint32> sizes;
	switch (vertexType)
	{
	case H_VERTEX_TYPE_PCT:
		stride = sizeof(HVertexPCT);
		offsetsSource.push_back(offsetof(Vertex_Master, m_position));
		offsetsSource.push_back(offsetof(Vertex_Master, m_floatColor));
		offsetsSource.push_back(offsetof(Vertex_Master, m_texCoords));
		offsetsTarget.push_back(offsetof(HVertexPCT, position));
		offsetsTarget.push_back(offsetof(HVertexPCT, color));
		offsetsTarget.push_back(offsetof(HVertexPCT, uv));
		sizes.push_back(sizeof(float3));
		sizes.push_back(sizeof(float4));
		sizes.push_back(sizeof(float2));
		break;
	case H_VERTEX_TYPE_PCTN:
		stride = sizeof(HVertexPCTN);
		offsetsSource.push_back(offsetof(Vertex_Master, m_position));
		offsetsSource.push_back(offsetof(Vertex_Master, m_floatColor));
		offsetsSource.push_back(offsetof(Vertex_Master, m_texCoords));
		offsetsSource.push_back(offsetof(Vertex_Master, m_normal));
		offsetsTarget.push_back(offsetof(HVertexPCTN, position));
		offsetsTarget.push_back(offsetof(HVertexPCTN, color));
		offsetsTarget.push_back(offsetof(HVertexPCTN, uv));
		offsetsTarget.push_back(offsetof(HVertexPCTN, normal));
		sizes.push_back(sizeof(float3));
		sizes.push_back(sizeof(float4));
		sizes.push_back(sizeof(float2));
		sizes.push_back(sizeof(float3));
		break;
	case H_VERTEX_TYPE_PCTNT:
		stride = sizeof(HVertexPCTNT);
		offsetsSource.push_back(offsetof(Vertex_Master, m_position));
		offsetsSource.push_back(offsetof(Vertex_Master, m_floatColor));
		offsetsSource.push_back(offsetof(Vertex_Master, m_texCoords));
		offsetsSource.push_back(offsetof(Vertex_Master, m_normal));
		offsetsSource.push_back(offsetof(Vertex_Master, m_tangent));
		offsetsSource.push_back(offsetof(Vertex_Master, m_bitanSign));
		offsetsTarget.push_back(offsetof(HVertexPCTNT, position));
		offsetsTarget.push_back(offsetof(HVertexPCTNT, color));
		offsetsTarget.push_back(offsetof(HVertexPCTNT, uv));
		offsetsTarget.push_back(offsetof(HVertexPCTNT, normal));
		offsetsTarget.push_back(offsetof(HVertexPCTNT, tangent));
		offsetsTarget.push_back(offsetof(HVertexPCTNT, normal) + offsetof(float4, w));

		sizes.push_back(sizeof(float3));
		sizes.push_back(sizeof(float4));
		sizes.push_back(sizeof(float2));
		sizes.push_back(sizeof(float3));
		sizes.push_back(sizeof(float3));
		sizes.push_back(sizeof(float));
		break;
	case H_VERTEX_TYPE_P:
		stride = sizeof(HVertexP);
		offsetsSource.push_back(offsetof(Vertex_Master, m_position));
		//offsetsSource.push_back(offsetof(Vertex_Master, m_position));
		offsetsTarget.push_back(offsetof(HVertexP, position));
		//offsetsTarget.push_back(sizeof(float3));
		sizes.push_back(sizeof(float3));
		//sizes.push_back(sizeof(float));
		break;
	default:
		ERROR_AND_DIE("Unrecognized vertex type\n");
	}
	uint32 dataSize = stride * numVerts;
	uint32 numOffsets = offsetsSource.size();

	if (pOutDataSize)
	{
		*pOutDataSize = dataSize;
	}

	*ppOutVerts = malloc(dataSize);

	char* workingTarget = (char*)*ppOutVerts;
	for (uint32 vertexIndex = 0; vertexIndex < numVerts; vertexIndex++)
	{
		char* workingSource = (char*)&m_vertices[vertexIndex];
		char* workingTarget = (char*)*ppOutVerts + (vertexIndex * stride);
		for (uint32 elementIndex = 0; elementIndex < numOffsets; elementIndex++)
		{
			uint32 offsetSource = offsetsSource[elementIndex];
			uint32 offsetTarget = offsetsTarget[elementIndex];
			uint32 size = sizes[elementIndex];

			memcpy(workingTarget + offsetTarget, workingSource + offsetSource, size);
		}
	}
}


//-----------------------------------------------------------------------------------------------
void MeshBuilder::CopyIndexData(void** ppOutIndices, uint32* pOutNumIndices)
{
	ASSERT_OR_DIE(ppOutIndices, "Must provide pointer for output indices\n");
	uint32 numIndices = m_indices.size();
	if (pOutNumIndices)
	{
		*pOutNumIndices = numIndices;
	}
	uint32 bytes = sizeof(uint16) * numIndices;
	*ppOutIndices = malloc(sizeof(uint16) * m_indices.size());
	memcpy(*ppOutIndices, &m_indices[0], bytes);
}


//-----------------------------------------------------------------------------------------------
void MeshBuilder::CalculateNormalsFromFaces()
{
	ASSERT_OR_DIE(!m_indices.empty(), "Cannot calculate normals without reused vertices\n");

	uint32 indexCount = m_indices.size();
	ASSERT_OR_DIE(indexCount % 3 == 0, "Mesh is not triangulated\n");
	uint32 faceCount = indexCount / 3;
	uint32 vertexCount = m_vertices.size();

	FOR_COUNT(vertexIndex, vertexCount)
	{
		//Zero out the normals, as we'll be adding them together
		Vertex_Master& thisVert = m_vertices[vertexIndex];
		thisVert.m_normal = Vector3::Zero;
	}

	FOR_COUNT(faceIndex, faceCount)
	{
		uint32 startIndex = faceIndex * 3;

		uint16 index1 = m_indices[startIndex];
		uint16 index2 = m_indices[startIndex + 1];
		uint16 index3 = m_indices[startIndex + 2];

		Vertex_Master& vert1 = m_vertices[index1];
		Vertex_Master& vert2 = m_vertices[index2];
		Vertex_Master& vert3 = m_vertices[index3];

		Vector3 normalVector = Vector3::Cross(vert2.m_position - vert1.m_position, vert3.m_position - vert1.m_position);

		normalVector.Normalize();

		vert1.m_normal += normalVector;
		vert2.m_normal += normalVector;
		vert3.m_normal += normalVector;
	}

	FOR_COUNT(vertexIndex, vertexCount)
	{
		//Normalize the normal vector sums
		Vertex_Master& thisVert = m_vertices[vertexIndex];
		thisVert.m_normal.Normalize();
	}
	
	m_mask |= (1 << NORMAL);
}


//-----------------------------------------------------------------------------------------------
static bool DoVerticesMatch(const Vertex_Master& left, const Vertex_Master& right)
{
	const float epsilon = .001f;
	if (right.m_position.x < left.m_position.x + epsilon && right.m_position.x > left.m_position.x - epsilon)
	{
		if (right.m_position.y < left.m_position.y + epsilon && right.m_position.y > left.m_position.y - epsilon)
		{
			if (right.m_position.z < left.m_position.z + epsilon && right.m_position.z > left.m_position.z - epsilon)
			{
				if (right.m_texCoords.x < left.m_texCoords.x + epsilon && right.m_texCoords.x > left.m_texCoords.x - epsilon)
				{
					if (right.m_texCoords.y < left.m_texCoords.y + epsilon && right.m_texCoords.y > left.m_texCoords.y - epsilon)
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
void MeshBuilder::GenerateIndexData()
{
	std::vector<Vertex_Master> finalVerts;
	ASSERT_OR_DIE(m_indices.empty(), "Index data already exists\n");

	const uint32 k_invalid = ~0U;
	uint32 lastGeneratedIndex = k_invalid;

	uint32 inputCount = m_vertices.size();

	FOR_COUNT(inputIndex, inputCount)
	{
		Vertex_Master& thisInputVert = m_vertices[inputIndex];

		uint32 outputCount = finalVerts.size();
		
		uint32 foundVertex = k_invalid;
		FOR_COUNT(outputIndex, outputCount)
		{
			Vertex_Master& thisOutputVert = finalVerts[outputIndex];

			if (DoVerticesMatch(thisInputVert, thisOutputVert))
			{
				foundVertex = outputIndex;
				break;
			}
		}

		if (foundVertex == k_invalid)
		{
			finalVerts.push_back(thisInputVert);
			if (lastGeneratedIndex == k_invalid)
			{
				m_indices.push_back(0U);
			}
			else
			{
				m_indices.push_back(lastGeneratedIndex + 1);
			}
			
			lastGeneratedIndex = m_indices.back();
		}
		else
		{
			m_indices.push_back(foundVertex);
		}
	}

	m_vertices = finalVerts;
}


//-----------------------------------------------------------------------------------------------
std::vector<MeshBuilder*> MeshBuilder::CombineByMaterial(MeshBuilder** meshes, int numElements)
{
	std::vector<MeshBuilder*> result;
	std::vector<std::string> materials;
	for (int meshIter = 0; meshIter < numElements; meshIter++)
	{
		std::string matName = meshes[meshIter]->m_materialName;
		bool found = false;
		for (auto matIter = materials.begin(); matIter != materials.end(); matIter++)
		{
			if (matName == *matIter)
			{
				*result[matIter - materials.begin()] += meshes[meshIter];
				found = true;
			}
		}
		if (!found)
		{
			std::string name = meshes[meshIter]->m_name;
			MeshBuilder* newMeshBuilder = new MeshBuilder(name);
			newMeshBuilder->m_mask = meshes[meshIter]->m_mask;
			newMeshBuilder->Begin(meshes[meshIter]->m_drawMode, meshes[meshIter]->m_usingIbo);
			*newMeshBuilder += meshes[meshIter];
			newMeshBuilder->m_materialName = matName;
			result.push_back(newMeshBuilder);
			materials.push_back(matName);
		}
		delete meshes[meshIter];
	}

	return result;
}


//-----------------------------------------------------------------------------------------------
MeshBuilder* BuildSurfacePatch(float startX, float endX, int xSections, float startY, float endY, int ySections, PatchFunction CalculatePosition, void* userData)
{
	ASSERT_OR_DIE(xSections > 0, "Patch call invalid.  Must have x sections greater than 0");
	ASSERT_OR_DIE(ySections > 0, "Patch call invalid.  Must have y sections greater than 0");

	MeshBuilder* mb = new MeshBuilder("");
	mb->Begin(R_TRIANGLES, true);

	//ASK WHY THIS HAPPENED!!!!  Why did the ys and the winding order have to switch?
	std::swap(startY, endY);

	int xVertCount = xSections + 1;
	int yVertCount = ySections + 1;

	float xRange = endX - startX;
	float yRange = endY - startY;
	float xStep = xRange / (float)xSections;
	float yStep = yRange / (float)ySections;

	float uStep = 1.f / (float)xSections;
	float vStep = 1.f / (float)ySections;

	int startIndex = 0;

	float x, y;
	float u, v;

	y = startY;
	v = 0.f;

	float delta = .01f;

	for (int iy = 0; iy < yVertCount; iy++)
	{
		x = startX;
		u = 0.f;

		for (int ix = 0; ix < xVertCount; ix++)
		{
			mb->SetUV(u, v);

			mb->SetTangent(CalculatePosition(userData, x + delta, y) - CalculatePosition(userData, x - delta, y));
			mb->SetBitangent(CalculatePosition(userData, x, y + delta) - CalculatePosition(userData, x, y - delta));

			Vector3 position = CalculatePosition(userData, x, y);
			mb->AddVertex(position);

			x += xStep;
			u += uStep;
		}

		y += yStep;
		v += vStep;
	}

	for (int iy = 0; iy < ySections; iy++)
	{
		for (int ix = 0; ix < xSections; ix++)
		{
			int bottomLeft = startIndex + (iy * xVertCount) + ix;
			int bottomRight = bottomLeft + 1;
			int topLeft = bottomLeft + xVertCount;
			int topRight = topLeft + 1;

			mb->AddQuadIndices(topRight, topLeft, bottomRight, bottomLeft);
		}
	}

	return mb;
}


//-----------------------------------------------------------------------------------------------
MeshBuilder* BuildPlane(const Vector3& initialPosition, const Vector3& right, const Vector3& up,
	float startX, float endX, int xSections,
	float startY, float endY, int ySections)
{
	PlaneUserData pud;
	pud.initialPosition = initialPosition;
	pud.right = right;
	pud.up = up;

	return BuildSurfacePatch(startX, endX, xSections, startY, endY, ySections, PlaneFunction, &pud);
}


//-----------------------------------------------------------------------------------------------
Vector3 PlaneFunction(void* userData, float x, float y)
{
	PlaneUserData* plane = (PlaneUserData*)userData;
	Vector3 position = plane->initialPosition + x * plane->right + y * plane->up;

	return position;
}


//-----------------------------------------------------------------------------------------------
MeshBuilder* BuildSphere(const Vector3& center, float radius, int xSections, int ySections)
{
	SphereUserData sud;
	sud.center = center;
	sud.radius = radius;

	return BuildSurfacePatch(0.f, 1.f, xSections, 0.f, 1.f, ySections, SphereFunction, &sud);
}


//-----------------------------------------------------------------------------------------------
Vector3 SphereFunction(void* userData, float x, float y)
{
	SphereUserData* sphere = (SphereUserData*)userData;
	float phiRads = (y * 2.f - 1.f) * (pi / 2.f);
	Vector4 unitCirclePosition(cos(phiRads) * sphere->radius, sin(phiRads) * sphere->radius, 0.f, 0.f);
	float thetaRads = x * (2.f * pi);
	float thetaDegrees = thetaRads * RAD2DEG;
	Matrix44 uvRotationMatrix;
	uvRotationMatrix.MakeRotationY(thetaDegrees);

	Vector3 finalPos = (unitCirclePosition * uvRotationMatrix).XYZ();
	finalPos += sphere->center;

	return finalPos;
}


//-----------------------------------------------------------------------------------------------
void MeshBuilder::AddStringEffectFragment(const StringEffectFragment& fragment, const BitmapFont* font, float scale, float totalStringWidth, 
	float totalWidthUpToNow, float width, float height, int lineNum, float lineWidth, HorizontalAlignment hAlign, EVerticalAlignment vAlign, int& glyphIndex)
{
	std::string asciiText = fragment.m_value;
	if (asciiText.empty())
	{
		return;
	}
	if (font == nullptr)
	{
		font = BitmapFont::CreateOrGetFont("consolas");
	}
	int stringLength = asciiText.size();
	float startY = vAlign == EVerticalAlignment::ALIGNMENT_TOP ? height - (float)(lineNum + 1) * font->m_lineHeight * scale : (float)(lineNum + 1) * font->m_lineHeight * scale;
	float startX = (width - lineWidth) * hAlign * scale + totalWidthUpToNow;
	Vector3 cursorPosition = Vector3(startX, startY, 0.f);
	const Glyph* previousGlyph = nullptr;
	float totalWidthSoFar = totalWidthUpToNow;
	float localWidthSoFar = 0;
	float fragmentWidth = font->CalculateTextWidth(asciiText, scale);
	for (int i = 0; i < stringLength; i++)
	{
		unsigned char currentCharacter = asciiText[i];
		const Glyph* glyph = font->GetGlyph(currentCharacter);
		float glyphWidth = static_cast<float>(glyph->width) * scale;
		float glyphHeight = static_cast<float>(glyph->height) * scale;

		if (previousGlyph)
		{
			const float kerning = font->GetKerningOffset(previousGlyph->tChar, glyph->tChar);
			cursorPosition.x += kerning * scale;
		}
		Vector3 offset(glyph->xOffset * scale, -glyph->yOffset * scale, 0.f);
		Vector3 topRight = cursorPosition + offset;
		topRight.x += glyphWidth;
		Vector3 bl = cursorPosition + offset - Vector3(0.f, glyphHeight, 0.f);
		AABB2 quadBounds = AABB2(Vector2(bl.x, bl.y), Vector2(topRight.x, topRight.y));
		AABB2 glyphBounds = font->GetTexCoordsForGlyph(glyph->tChar);
		// 		if (drawShadow)
		// 		{
		// 			float shadowWidthOffset = glyphWidth / 10.0f;
		// 			float shadowHeightOffset = glyphHeight / -10.0f;
		// 			Vector3 shadowOffset = (right * shadowWidthOffset) + (up * shadowHeightOffset);
		// 			//Vector2 shadowOffset = Vector2(shadowWidthOffset, shadowHeightOffset);
		// 			//AABB2 shadowBounds = AABB2(bottomLeft + shadowOffset, topRight + shadowOffset);
		// 			this->AddGlyph(bottomLeft, up, right, shadowHeightOffset, shadowWidthOffset, glyphBounds.mins, glyphBounds.maxs, RGBA::BLACK);
		// 		}
		//this->AddTexturedAABB(quadBounds, glyphBounds.mins, glyphBounds.maxs, RGBA::WHITE);
		float stringXMin = totalWidthSoFar / totalStringWidth;
		float fragXMin = localWidthSoFar / fragmentWidth;
		totalWidthSoFar += glyph->advance * scale;
		localWidthSoFar += glyph->advance * scale;
		float stringXMax = totalWidthSoFar / totalStringWidth;
		float fragXMax = localWidthSoFar / fragmentWidth;
		cursorPosition.x += glyph->advance * scale;
		this->AddGlyph(bl, Vector3::Up, Vector3::Right, (glyph->height * scale), (glyph->width * scale), glyphBounds.mins, glyphBounds.maxs, WHITE, stringXMin, stringXMax, fragXMin, fragXMax, glyphIndex);
		if (!fragment.m_effect.pop || i == stringLength - 1)
		{
			++glyphIndex;
		}
		previousGlyph = glyph;
	}
}


//-----------------------------------------------------------------------------------------------
void MeshBuilder::AddGlyph(const Vector3& bottomLeft, const Vector3& up, const Vector3& right, float upExtents, float rightExtents, 
	const Vector2& uvMins, const Vector2& uvMaxs, const Rgba& color, float stringCoordXMin, float stringCoordXMax, float fragCoordXMin, float fragCoordXMax, int glyphIndex)
{
	int startingVertex = m_vertices.size();
	Vector3 topLeft = bottomLeft + (up * upExtents);
	Vector3 bottomRight = bottomLeft + (right * rightExtents);
	Vector3 topRight = topLeft + (right * rightExtents);
	SetGlyphIndex(glyphIndex);
	//SetColor(color);
	SetUV(uvMins.x, uvMaxs.y);
	SetNormalizedGlyphCoords(0.f, 0.f);
	SetNormalizedStringCoords(stringCoordXMin, 0.f);
	SetNormalizedFragCoords(fragCoordXMin);
	AddVertex(bottomLeft);
	SetUV(uvMaxs.x, uvMaxs.y);
	SetNormalizedGlyphCoords(1.f, 0.f);
	SetNormalizedStringCoords(stringCoordXMax, 0.f);
	SetNormalizedFragCoords(fragCoordXMax);
	AddVertex(bottomRight);
	SetUV(uvMaxs.x, uvMins.y);
	SetNormalizedGlyphCoords(1.f, 1.f);
	SetNormalizedStringCoords(stringCoordXMax, 0.f);
	SetNormalizedFragCoords(fragCoordXMax);
	AddVertex(topRight);
	SetUV(uvMins.x, uvMins.y);
	SetNormalizedGlyphCoords(0.f, 1.f);
	SetNormalizedStringCoords(stringCoordXMin, 0.f);
	SetNormalizedFragCoords(fragCoordXMin);
	AddVertex(topLeft);
	AddQuadIndices(startingVertex + 3, startingVertex + 2, startingVertex + 0, startingVertex + 1);
}