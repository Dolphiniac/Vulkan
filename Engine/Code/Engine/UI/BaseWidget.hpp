#pragma once

#include "Engine/Renderer/RendererInterface.hpp"
#include "Quantum/Core/EventSystem.h"
#include "Engine/UI/UISystem.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Quantum/Math/Vector2.h"
#include "Engine/Math/AABB2.hpp"


//-----------------------------------------------------------------------------------------------
enum EWidgetState
{
	WIDGETSTATE_ACTIVE,
	WIDGETSTATE_HIGHLIGHTED,
	WIDGETSTATE_PRESSED,
	WIDGETSTATE_DISABLED,
	WIDGETSTATE_HIDDEN,
	NUM_WIDGETSTATES,
	WIDGETSTATE_INVALID = NUM_WIDGETSTATES
};


//-----------------------------------------------------------------------------------------------
class BaseWidget : public RendererInterface
{
	friend class UISystem;
public:
	BaseWidget();
	virtual bool ShouldDie() const override { return false; }
	virtual bool IsGroup() const { return false; }
	void SetProperty(const QuString& name, const QuString& value);
	virtual void InitializeRendering();
	virtual BaseWidget* GetHighlightedWidget(const QuVector2& vec);
	AABB2 CalcBounds() const;

protected:
	class MeshRenderer* m_meshRenderer;
	class Mesh* m_mesh;
	class Material* m_material;
	QuNamedProperties m_properties;
	EWidgetState m_currentState;
	BaseWidget* m_parent = nullptr;
	Vector3 m_center = Vector3::Zero;
};