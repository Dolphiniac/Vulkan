#pragma once

#include "Quantum/Hephaestus/Declarations.h"


//-----------------------------------------------------------------------------------------------
class HMesh
{
public:
	void SetVertexData(void* data, uint32 dataSize, uint32 numVertices);
	void SetIndexData(void* data, uint32 numIndices, EIndexType type);
	void Draw(class HCommandBuffer* commandBuffer);
	static HMesh GetSkyboxMesh();

private:
	HephBuffer m_vertexBuffer = H_NULL_HANDLE;
	uint32 m_numVertices = H_INVALID;
	HephBuffer m_indexBuffer = H_NULL_HANDLE;
	uint32 m_numIndices = H_INVALID;
	EIndexType m_indexType;
};