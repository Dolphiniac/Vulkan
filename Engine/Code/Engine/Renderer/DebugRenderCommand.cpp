#include "Engine/Renderer/DebugRenderCommand.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/EngineSystemManager.hpp"

#ifndef __USING_UWP


//-----------------------------------------------------------------------------------------------
void DebugRenderCommand::Draw()
{
	float xrayWidth = .3f;
	float depthTestWidth = 3.f;
	if (m_dtt == DUALDEPTHTEST || m_dtt == NODEPTHTEST)
	{
		The.Renderer->UseZBuffering(false);
		unsigned char saveColor = m_color.a;
		unsigned char saveFaceColor = m_faceColor.a;
		m_color.a = (unsigned char)(m_color.a * .5f);
		m_faceColor.a = (unsigned char)(m_faceColor.a * .5f);
		switch (m_drt)
		{
		case DRT_POINT:
			DrawPoint(xrayWidth);
			break;
		case LINE:
			DrawLine(xrayWidth);
			break;
		case ARROW:
			DrawArrow(xrayWidth);
			break;
		case AABB3:
			DrawAABB3(xrayWidth);
			break;
		case SPHERE:
			DrawSphere(xrayWidth);
			break;
		}
		m_color.a = saveColor;
		m_faceColor.a = saveFaceColor;
	}

	if (m_dtt == DUALDEPTHTEST || m_dtt == DEPTHTEST)
	{
		The.Renderer->UseZBuffering(true);
		switch (m_drt)
		{
		case DRT_POINT:
			DrawPoint(depthTestWidth);
			break;
		case LINE:
			DrawLine(depthTestWidth);
			break;
		case ARROW:
			DrawArrow(depthTestWidth);
			break;
		case AABB3:
			DrawAABB3(depthTestWidth);
			break;
		case SPHERE:
			DrawSphere(depthTestWidth);
			break;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void DebugRenderCommand::DrawPoint(float lineThickness) const
{
	DrawSphere(lineThickness);
}


//-----------------------------------------------------------------------------------------------
void DebugRenderCommand::DrawLine(float lineThickness) const
{
	The.Renderer->DrawLine(m_position, m_endPosition, m_color, m_color, lineThickness);
}


//-----------------------------------------------------------------------------------------------
void DebugRenderCommand::DrawArrow(float lineThickness) const
{
	The.Renderer->DrawLine(m_position, m_endPosition, m_color, m_color, lineThickness);
	DrawSphere(lineThickness);
}


//-----------------------------------------------------------------------------------------------
void DebugRenderCommand::DrawAABB3(float lineThickness) const
{
	Vector3 mins(m_position);
	Vector3 maxs(m_endPosition);
	Vector3 lfb(mins);
	Vector3 lft(mins.x, mins.y, maxs.z);
	Vector3 lbb(mins.x, maxs.y, mins.z);
	Vector3 lbt(mins.x, maxs.y, maxs.z);
	Vector3 rfb(maxs.x, mins.y, mins.z);
	Vector3 rft(maxs.x, mins.y, maxs.z);
	Vector3 rbb(maxs.x, maxs.y, mins.z);
	Vector3 rbt(maxs);

	The.Renderer->DrawLine(lfb, lft, m_color, m_color, lineThickness);
	The.Renderer->DrawLine(lfb, rfb, m_color, m_color, lineThickness);
	The.Renderer->DrawLine(lfb, lbb, m_color, m_color, lineThickness);
	The.Renderer->DrawLine(rbb, lbb, m_color, m_color, lineThickness);
	The.Renderer->DrawLine(rbb, rbt, m_color, m_color, lineThickness);
	The.Renderer->DrawLine(rbb, rfb, m_color, m_color, lineThickness);
	The.Renderer->DrawLine(rft, rfb, m_color, m_color, lineThickness);
	The.Renderer->DrawLine(lft, rft, m_color, m_color, lineThickness);
	The.Renderer->DrawLine(lbb, lbt, m_color, m_color, lineThickness);
	The.Renderer->DrawLine(lft, lbt, m_color, m_color, lineThickness);
	The.Renderer->DrawLine(rbt, lbt, m_color, m_color, lineThickness);
	The.Renderer->DrawLine(rbt, rft, m_color, m_color, lineThickness);

	The.Renderer->DrawQuad(lfb, rfb, rft, lft, m_faceColor);
	The.Renderer->DrawQuad(lbb, lfb, lft, lbt, m_faceColor);
	The.Renderer->DrawQuad(rbb, lbb, lbt, rbt, m_faceColor);
	The.Renderer->DrawQuad(rfb, rbb, rbt, rft, m_faceColor);
	The.Renderer->DrawQuad(lft, rft, rbt, lbt, m_faceColor);
	The.Renderer->DrawQuad(lbb, rbb, rfb, lfb, m_faceColor);
}


//-----------------------------------------------------------------------------------------------
void DebugRenderCommand::DrawSphere(float lineThickness) const
{
	The.Renderer->TranslateView(m_position);
	The.Renderer->DrawPolygon(m_radius, 30.f, 0.f, m_color, lineThickness);
	//The.Renderer->TranslateView(-m_position);
	The.Renderer->RotateView(90.f, 1.f, 0.f, 0.f);
	//The.Renderer->TranslateView(m_position);
	The.Renderer->DrawPolygon(m_radius, 30.f, 0.f, m_color, lineThickness); 
	//The.Renderer->TranslateView(-m_position);
	The.Renderer->RotateView(-90.f, 1.f, 0.f, 0.f);
	The.Renderer->RotateView(90.f, 0.f, 1.f, 0.f);
	//The.Renderer->TranslateView(m_position);
	The.Renderer->DrawPolygon(m_radius, 30.f, 0.f, m_color, lineThickness);
	The.Renderer->RotateView(-90.f, 0.f, 1.f, 0.f);
	The.Renderer->TranslateView(-m_position);
}
#endif