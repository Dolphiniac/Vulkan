#include "Engine/UI/GroupWidget.hpp"
#include "Engine/Renderer/MeshRenderer.hpp"


//-----------------------------------------------------------------------------------------------
STATIC WidgetGeneratorRegistration GroupWidget::s_registration("GroupWidget", []() -> BaseWidget* { return new GroupWidget(); });


//-----------------------------------------------------------------------------------------------
void GroupWidget::Update(float deltaSeconds)
{
	for (BaseWidget* child : m_children)
	{
		child->Update(deltaSeconds);
	}
}


//-----------------------------------------------------------------------------------------------
void GroupWidget::Render() const
{
	m_meshRenderer->Render();

	for (BaseWidget* child : m_children)
	{
		child->Render();
	}
}