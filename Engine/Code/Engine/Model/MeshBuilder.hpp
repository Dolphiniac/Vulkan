#pragma once

#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/Rgba.hpp"
#include "Engine/Renderer/TheRenderer.hpp"
#include "Quantum/Hephaestus/Declarations.h"
#include "Quantum/Math/MathCommon.h"

#include <vector>
#include <string>


//-----------------------------------------------------------------------------------------------
struct Vertex_Master
{
	Vector3 m_position;
	Vector2 m_texCoords;
	Rgba m_color;
	float4 m_floatColor;
	Vector3 m_tangent;
	Vector3 m_bitangent;
	Vector3 m_normal;
	float m_bitanSign;

	Vector4 m_boneWeights;
	IntVector4 m_boneIndices;
	Vector2 m_normalizedGlyphCoords;
	Vector2 m_normalizedStringCoords;
	float m_normalizedFragCoords;
	int m_glyphIndex;
};


//-----------------------------------------------------------------------------------------------
enum EWriteMask
{
	POSITION = 0,
	TEX_COORDS,
	COLOR,
	TANGENT,
	BITANGENT,
	NORMAL,
	INDEX,
	MATERIAL,
	BONE,
	NUM_FIELDS
};


//-----------------------------------------------------------------------------------------------
typedef float HorizontalAlignment;
enum EVerticalAlignment;


//-----------------------------------------------------------------------------------------------
class MeshBuilder
{
	static const int MESH_BUILDER_VERSION = 1;
public:
	MeshBuilder(const std::string& name) : m_name(name), m_version(MESH_BUILDER_VERSION) {}
	void Begin(Renum drawMode, bool usingIbo) { m_drawMode = drawMode; m_usingIbo = usingIbo; }
	void AddVertex(const Vector3& position);
	class Mesh* ConstructMesh(Renum vertexType);
	void SetColor(const float4& color) { m_mask |= (1 << COLOR); m_color = Rgba(color.r, color.g, color.b, color.a); m_floatColor = color; }
	void SetUV(float u, float v) { m_mask |= (1 << TEX_COORDS); m_uv = Vector2(u, v); }
	void SetTangent(const Vector3& tangent) { m_mask |= (1 << TANGENT); m_tangent = tangent; }
	void SetBitangent(const Vector3& bitangent) { m_mask |= (1 << BITANGENT); m_bitangent = bitangent; }
	void SetNormal(const Vector3& normal) { m_mask |= (1 << NORMAL); m_normal = normal; }
	void SetBoneWeights(const IntVector4& boneIndices, const Vector4& boneWeights) { m_boneWeights = boneWeights; m_boneIndices = boneIndices; m_mask |= (1 << BONE); }
	void SetNormalizedGlyphCoords(float x, float y) { m_normalizedGlyphCoords = Vector2(x, y); }
	void SetNormalizedStringCoords(float x, float y) { m_normalizedStringCoords = Vector2(x, y); }
	void SetNormalizedFragCoords(float value) { m_normalizedFragCoords = value; }
	void SetGlyphIndex(int value) { m_glyphIndex = value; }
	void ClearBoneWeights() { m_boneIndices = { 0, 0, 0, 0 }; m_boneWeights = { 1.f, 0.f, 0.f, 0.f }; }
	void SetMat(const std::string& matName) { m_mask |= (1 << MATERIAL); m_materialName = matName; }
	void AddQuadIndices(int topLeft, int topRight, int bottomLeft, int bottomRight);
	void AddStringEffectFragment(const class StringEffectFragment& asciiText, const class BitmapFont* font, float scale, float totalStringWidth, float totalWidthUpToNow,
		float width, float height, int lineNum, float lineWidth, HorizontalAlignment hAlign, EVerticalAlignment vAlign, int& glyphIndex);
	void AddGlyph(const Vector3& bottomLeft, const Vector3& up, const Vector3& right, float upExtents, float rightExtents, const Vector2& uvMins, const Vector2& uvMaxs, const Rgba& color,
		float stringCoordXMin, float stringCoordXMax, float fragCoordXMin, float fragCoordXMax, int glyphIndex);
	static MeshBuilder* Combine(MeshBuilder** meshBuilders, int numBuilders);
	void GenerateTangentSpace(bool overrideTangent);
	static void Write(MeshBuilder* mb, class BinaryWriter& writer);
	static std::vector<MeshBuilder*> ReadFromFile(const std::string& filePath);
	void CopyInterleavedMeshData(EVertexType vertexType, void** ppOutVerts, uint32* pOutNumVerts, uint32* pOutDataSize);
	void CopyIndexData(void** ppOutIndices, uint32* pOutNumIndices);
	void CalculateNormalsFromFaces();
	void GenerateIndexData();

private:
	void CopyDataPCT(class Mesh* mesh);
	void CopyDataPCTTB(class Mesh* mesh);
	void CopyDataPCTN(class Mesh* mesh);
	void CopyDataPCTTBB(class Mesh* mesh);
	void CopyDataText(class Mesh* mesh);
	void operator+=(MeshBuilder* other);
	static int MikkTGetNumFaces(const struct SMikkTSpaceContext* context);
	static int MikkTGetNumVerticesOnFace(const struct SMikkTSpaceContext* context, int iFace);
	static void MikkTGetPosition(const struct SMikkTSpaceContext* context, float* fvPosOut, int iFace, int iVert);
	static void MikkTGetUV(const struct SMikkTSpaceContext* context, float* fvTexcOut, int iFace, int iVert);
	static void MikkTGetNormal(const struct SMikkTSpaceContext* context, float* fvNormOut, int iFace, int iVert);
	static void MikkTSetTangent(const struct SMikkTSpaceContext* context, const float* fvTangent, float fSign, int iFace, int iVert);

private:
	std::string m_name;
	std::vector<Vertex_Master> m_vertices;
	std::vector<unsigned short> m_indices;
	bool m_usingIbo;
	int m_mask = 0;
	int m_version;
	Renum m_drawMode;
	Rgba m_color = WHITE;
	float4 m_floatColor = float4(1.f);
	Vector2 m_uv = Vector2::Zero;
	Vector3 m_tangent = Vector3::Zero;
	Vector3 m_bitangent = Vector3::Zero;
	Vector3 m_normal = Vector3::Zero;
	IntVector4 m_boneIndices{ 0, 0, 0, 0 };
	Vector4 m_boneWeights{ 1.f, 0.f, 0.f, 0.f };
	Vector2 m_normalizedGlyphCoords;
	Vector2 m_normalizedStringCoords;
	float m_normalizedFragCoords;
	int m_glyphIndex;
	
public:
	std::string m_materialName;
	static std::vector<MeshBuilder*> CombineByMaterial(MeshBuilder** meshes, int numElements);
};

typedef Vector3(*PatchFunction)(void* userData, float x, float y);

//-----------------------------------------------------------------------------------------------
MeshBuilder* BuildSurfacePatch(float startX, float endX, int xSections,
	float startY, float endY, int ySections,
	PatchFunction CalculatePosition, void* userData);


//-----------------------------------------------------------------------------------------------
struct PlaneUserData
{
	Vector3 initialPosition;
	Vector3 right;
	Vector3 up;
};


//-----------------------------------------------------------------------------------------------
struct SphereUserData
{
	Vector3 center;
	float radius;
};


//-----------------------------------------------------------------------------------------------
Vector3 PlaneFunction(void* userData, float x, float y);
MeshBuilder* BuildPlane(const Vector3& initialPosition, const Vector3& right, const Vector3& up,
	float startX, float endX, int xSections,
	float startY, float endY, int ySections);


//-----------------------------------------------------------------------------------------------
Vector3 SphereFunction(void* userData, float x, float y);
MeshBuilder* BuildSphere(const Vector3& center, float radius, int xSections, int ySections);