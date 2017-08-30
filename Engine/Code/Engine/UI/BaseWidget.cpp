#include "Engine/UI/BaseWidget.hpp"
#include "Quantum/Math/Vector2.h"
#include "Quantum/Renderer/Color.h"
#include "Engine/Model/MeshBuilder.hpp"
#include "Engine/Renderer/MeshRenderer.hpp"
#include "Engine/Renderer/SpriteRenderer.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
BaseWidget::BaseWidget()
	: m_currentState(WIDGETSTATE_ACTIVE)
	, m_material(nullptr)
	, m_mesh(nullptr)
	, m_meshRenderer(nullptr)
{
	m_properties.Set("Offset", QuVector2::Zero, DATATYPE_VEC2);
	m_properties.Set("Size", QuVector2(1.f, 1.f), DATATYPE_VEC2);
	m_properties.Set("BackgroundColor", QuColor::WHITE, DATATYPE_COLOR);
	m_properties.Set("EdgeColor", QuColor::BLACK, DATATYPE_COLOR);
	m_properties.Set("Opacity", 1.f, DATATYPE_FLOAT);
	m_properties.Set("EdgeThickness", .05f, DATATYPE_FLOAT);
}


//-----------------------------------------------------------------------------------------------
void BaseWidget::SetProperty(const QuString& name, const QuString& value)
{
	EDataType dataType = DATATYPE_UNSPECIFIED;
	int stubDatum;
	m_properties.Get(name, stubDatum, &dataType);

	switch (dataType)
	{
	case DATATYPE_COLOR:
		m_properties.Set(name, value.AsColor());
		break;
	case DATATYPE_FLOAT:
		m_properties.Set(name, value.AsFloat());
		break;
	case DATATYPE_INT:
		m_properties.Set(name, value.AsInt());
		break;
	case DATATYPE_VEC2:
		m_properties.Set(name, value.AsVec2());
		break;
	}
}


//-----------------------------------------------------------------------------------------------
void BaseWidget::InitializeRendering()
{
	MeshBuilder mb("");
	mb.Begin(R_TRIANGLES, true);
	QuColor color;
	m_properties.Get("BackgroundColor", color);
	m_center.z = 0.f;
	QuVector2 offset;
	m_properties.Get("Offset", offset);
	if (m_parent)
	{
		m_center.x = offset.x + m_parent->m_center.x;
		m_center.y = offset.y + m_parent->m_center.y;
	}
	else
	{
		m_center.x = offset.x;
		m_center.y = offset.y;
	}

	QuVector2 size;
	m_properties.Get("Size", size);
	float edgeThickness;
	m_properties.Get("EdgeThickness", edgeThickness);
	QuColor edgeColor;
	m_properties.Get("EdgeColor", edgeColor);

	float opacity;
	m_properties.Get("Opacity", opacity);
	edgeColor.a = (uint8)((float)edgeColor.a * opacity);
	color.a = (uint8)((float)color.a * opacity);
	//mb.SetColor(edgeColor);
	mb.SetUV(0.f, 0.f);
	mb.AddVertex(m_center + Vector3(-size.x * .5f - edgeThickness, size.y * .5f + edgeThickness, 0.f));

	mb.SetUV(0.f, 1.f);
	mb.AddVertex(m_center + Vector3(-size.x * .5f - edgeThickness, -size.y * .5f - edgeThickness, 0.f));

	mb.SetUV(1.f, 1.f);
	mb.AddVertex(m_center + Vector3(size.x * .5f + edgeThickness, -size.y * .5f - edgeThickness, 0.f));

	mb.SetUV(1.f, 0.f);
	mb.AddVertex(m_center + Vector3(size.x * .5f + edgeThickness, size.y * .5f + edgeThickness, 0.f));
	mb.AddQuadIndices(0, 3, 1, 2);

	//mb.SetColor(color);
	mb.SetUV(0.f, 0.f);
	mb.AddVertex(m_center + Vector3(-size.x * .5f, size.y * .5f, 0.f));

	mb.SetUV(0.f, 1.f);
	mb.AddVertex(m_center + Vector3(-size.x * .5f, -size.y * .5f, 0.f));

	mb.SetUV(1.f, 1.f);
	mb.AddVertex(m_center + Vector3(size.x * .5f, -size.y * .5f, 0.f));

	mb.SetUV(1.f, 0.f);
	mb.AddVertex(m_center + Vector3(size.x * .5f, size.y * .5f, 0.f));



	mb.AddQuadIndices(4, 7, 5, 6);

	m_mesh = mb.ConstructMesh(R_VERTEX_PCT);
	m_material = g_spriteRenderer->GetMaterial();
	m_meshRenderer = new MeshRenderer(m_mesh, m_material);
	m_meshRenderer->SetUniformTexture("gDiffuseTex", Texture::CreateOrGetTexture("NoTex"));
}


//-----------------------------------------------------------------------------------------------
BaseWidget* BaseWidget::GetHighlightedWidget(const QuVector2& vec)
{
	if (PointInsideBox(Vector2(vec.x, vec.y), CalcBounds()))
	{
		return this;
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
AABB2 BaseWidget::CalcBounds() const
{
	AABB2 result;

	QuVector2 size;
	m_properties.Get("Size", size);

	result.mins = Vector2(m_center.x - (size.x * .5f), m_center.y - (size.y * .5f));
	result.maxs = Vector2(m_center.x + (size.x * .5f), m_center.y + (size.y * .5f));

	return result;
}