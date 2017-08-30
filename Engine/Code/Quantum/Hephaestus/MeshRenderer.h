#pragma once

#include "Quantum/Core/SimpleMap.h"
#include "Quantum/Hephaestus/Renderable.h"

#include <vector>


//-----------------------------------------------------------------------------------------------
class HMeshRenderer : public HRenderable
{
public:
	HMeshRenderer(class HMesh* mesh, class HMaterial* material);

public:
	virtual void Draw() const override;

private:
	class HMesh* m_mesh = nullptr;
	class HMaterial* m_material = nullptr;
	QuSimpleMap<std::vector<class HCommandBuffer*>> m_buffers;
	std::vector<class HCommandBuffer*> m_tempCommandBuffers;
};