#pragma once

#include "Engine/Model/MeshBuilder.hpp"


//-----------------------------------------------------------------------------------------------
enum EMeshType
{
	BOX,
	UV_SPHERE,
	NUM_MESH_TYPES
};

//extern std::vector<MeshBuilder*> loadedMeshes;
//extern int numModels;
//extern class Actor* models;


//-----------------------------------------------------------------------------------------------
class Mesh
{
	friend class Material;
	friend class MeshRenderer;
	friend class MeshBuilder;

public:
	Mesh();
	void InitializeIBO(void* data, size_t count, size_t elemSize, Renum usage = R_STATIC_DRAW);
	void InitializeVBO(void* data, size_t count, size_t elemSize, Renum usage = R_STATIC_DRAW, bool shouldDeleteBuffer = false);
	void UpdateVBO(void* data, size_t count, size_t elemSize, Renum usage);
	void SetDrawShape(Renum drawShape) { m_drawShape = drawShape; }
	void Render() const;
	void SetVertexType(Renum vertexType) { m_vertexType = vertexType; }
	static void InitializeDefaultMeshes();
	static void DestroyDefaultMeshes();
	~Mesh();

private:
	static void InitSphereMeshTBN();
	static void InitBoxMeshTBN();

public:
	unsigned int m_vboID;
	unsigned int m_iboID;
	static Mesh* s_defaultMeshes[NUM_MESH_TYPES];

private:
	int m_numVertices;
	int m_numIndices;
	bool m_usingIBO;
	Renum m_vertexType;
	Renum m_drawShape;
};