#include "Quantum/Hephaestus/MeshRenderer.h"
#include "Quantum/Hephaestus/Declarations.h"
#include "Quantum/Hephaestus/Manager.h"
#include "Quantum/Hephaestus/Queue.h"
#include "Quantum/Hephaestus/RenderPass.h"
#include "Quantum/Hephaestus/Material.h"
#include "Quantum/Hephaestus/Mesh.h"

#include <vulkan.h>


//-----------------------------------------------------------------------------------------------
//CTORS AND DTOR
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
HMeshRenderer::HMeshRenderer(HMesh* mesh, HMaterial* material)
	: m_mesh(mesh)
	, m_material(material)
{
	uint32 renderPassCount = m_material->m_states.GetSize();
	FOR_COUNT(renderPassIndex, renderPassCount)
	{
		auto& p = m_material->m_states.GetPairAtIndex(renderPassIndex);
		std::vector<HMaterialState*>& states = p.value;
		uint32 stateCount = states.size();
		FOR_COUNT(stateIndex, stateCount)
		{
			if (states[stateIndex])
			{
				m_tempCommandBuffers.push_back(HManager::GetGraphicsQueue()->RetrieveCommandBuffer(false, false));
			}
			else
			{
				m_tempCommandBuffers.push_back(nullptr);
			}
		}

		m_buffers.Insert(p.key, m_tempCommandBuffers);
		m_tempCommandBuffers.clear();
	}
}


//-----------------------------------------------------------------------------------------------
//END CTORS AND DTOR
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
void HMeshRenderer::Draw() const
{
	HRenderPass* renderPass = HManager::GetCurrentRenderPass();
	HephCommandBufferInheritanceInfo inheritanceInfo = renderPass->GetInheritanceInfo();
	uint32 subpassIndex = inheritanceInfo->subpass;

	auto states = m_buffers.Find(renderPass->GetHandle());
	HCommandBuffer* commandBuffer = states->operator[](subpassIndex);
	ASSERT_OR_DIE(commandBuffer, "Mesh renderer not compatible with this subpass\n");
	commandBuffer->Reset();
	commandBuffer->Begin(inheritanceInfo);

	ASSERT_OR_DIE(m_material, "Must have material to draw with HMeshRenderer\n");
	m_material->BindState(commandBuffer, renderPass->GetHandle(), inheritanceInfo->subpass);

	ASSERT_OR_DIE(m_mesh, "Must have mesh to draw with HMeshRenderer\n");
	m_mesh->Draw(commandBuffer);

	commandBuffer->End();

	renderPass->RegisterDrawCommands(commandBuffer);
}