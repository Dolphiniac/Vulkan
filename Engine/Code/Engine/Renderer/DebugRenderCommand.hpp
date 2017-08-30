#pragma once

#ifndef __USING_UWP
#include "Engine/Renderer/GLRenderer.hpp"

enum EDebugRenderType
{
	DRT_POINT,
	LINE,
	ARROW,
	AABB3,
	SPHERE,
	NUM_DEBUG_RENDER_TYPES
};

enum EDepthTestType
{
	DEPTHTEST,
	NODEPTHTEST,
	DUALDEPTHTEST
};


class DebugRenderCommand
{
public:
	inline void Tick(float deltaSeconds);
	void Draw();
	bool ShouldDie() const { return m_shouldDie; }
	static inline DebugRenderCommand* DebugPoint(const Vector3& position, bool canTimeout, float timeToLive = 0.f, EDepthTestType dtt = DEPTHTEST, const Rgba& color = WHITE);
	static inline DebugRenderCommand* DebugLine(const Vector3& startPos, const Vector3& endPos, bool canTimeout, float timeToLive = 0.f, EDepthTestType dtt = DEPTHTEST, const Rgba& color = WHITE);
	static inline DebugRenderCommand* DebugArrow(const Vector3& startPos, const Vector3& endPos, bool canTimeout, float timeToLive = 0.f, EDepthTestType dtt = DEPTHTEST, const Rgba& color = WHITE);
	static inline DebugRenderCommand* DebugAABB3(const Vector3& mins, const Vector3& maxs, bool canTimeout, float timeToLive = 0.f, EDepthTestType dtt = DEPTHTEST, const Rgba& color = WHITE, const Rgba& faceColor = WHITE);
	static inline DebugRenderCommand* DebugSphere(const Vector3& position, float radius, bool canTimeout, float timeToLive = 0.f, EDepthTestType dtt = DEPTHTEST, const Rgba& color = WHITE);
private:
	inline DebugRenderCommand(bool canTimeout, float timeToLive, EDepthTestType dtt, const Rgba& color);
	inline void InitializePoint(const Vector3& position);
	inline void InitializeLine(const Vector3& startPos, const Vector3& endPos);
	inline void InitializeArrow(const Vector3& startPos, const Vector3& endPos);
	inline void InitializeAABB3(const Vector3& mins, const Vector3& maxs, const Rgba& edgeColor = WHITE, const Rgba& faceColor = WHITE);
	inline void InitializeSphere(const Vector3& position, float radius);
	void DrawPoint(float lineThickness) const;
	void DrawLine(float lineThickness) const;
	void DrawArrow(float lineThickness) const;
	void DrawAABB3(float lineThickness) const;
	void DrawSphere(float lineThickness) const;
private:
	float m_timeRemaining;
	EDepthTestType m_dtt;
	EDebugRenderType m_drt;
	Rgba m_color;
	bool m_canTimeout;
	bool m_shouldDie;

	Vector3 m_position;
	Vector3 m_endPosition;
	Rgba m_faceColor;
	float m_radius;
};

typedef DebugRenderCommand DRC;


//-----------------------------------------------------------------------------------------------
DebugRenderCommand::DebugRenderCommand(bool canTimeout, float timeToLive /* = 0.f */, EDepthTestType dtt /* = DEPTHTEST */, const Rgba& color /* = WHITE */)
	: m_timeRemaining(timeToLive)
	, m_dtt(dtt)
	, m_color(color)
	, m_canTimeout(canTimeout)
	, m_shouldDie(false)
{
}


//-----------------------------------------------------------------------------------------------
void DebugRenderCommand::Tick(float deltaSeconds)
{
	m_timeRemaining -= deltaSeconds;
	if (m_timeRemaining <= 0.f && m_canTimeout)
	{
		m_shouldDie = true;
	}
}


//-----------------------------------------------------------------------------------------------
void DebugRenderCommand::InitializePoint(const Vector3& position)
{
	m_position = position;
	m_radius = .1f;
	m_drt = DRT_POINT;
}


//-----------------------------------------------------------------------------------------------
void DebugRenderCommand::InitializeLine(const Vector3& startPos, const Vector3& endPos)
{
	m_position = startPos;
	m_endPosition = endPos;
	m_drt = LINE;
}


//-----------------------------------------------------------------------------------------------
void DebugRenderCommand::InitializeArrow(const Vector3& startPos, const Vector3& endPos)
{
	InitializeLine(endPos, startPos);
	m_radius = .1f;
	m_drt = ARROW;
}


//-----------------------------------------------------------------------------------------------
void DebugRenderCommand::InitializeAABB3(const Vector3& mins, const Vector3& maxs, const Rgba& edgeColor, const Rgba& faceColor)
{
	m_position = mins;
	m_endPosition = maxs;
	m_color = edgeColor;
	m_faceColor = faceColor;
	m_drt = AABB3;
}


//-----------------------------------------------------------------------------------------------
void DebugRenderCommand::InitializeSphere(const Vector3& position, float radius)
{
	m_position = position;
	m_radius = radius;
	m_drt = SPHERE;
}


//-----------------------------------------------------------------------------------------------
DebugRenderCommand* DebugRenderCommand::DebugPoint(const Vector3& position, bool canTimeout, float timeToLive /* = 0.f */, EDepthTestType dtt /* = DEPTHTEST */, const Rgba& color /* = WHITE */)
{
	DebugRenderCommand* result = new DebugRenderCommand(canTimeout, timeToLive, dtt, color);
	result->InitializePoint(position);
	return result;
}


//-----------------------------------------------------------------------------------------------
DebugRenderCommand* DebugRenderCommand::DebugLine(const Vector3& startPos, const Vector3& endPos, bool canTimeout, float timeToLive /* = 0.f */, EDepthTestType dtt /* = DEPTHTEST */, const Rgba& color /* = WHITE */)
{
	DebugRenderCommand* result = new DebugRenderCommand(canTimeout, timeToLive, dtt, color);
	result->InitializeLine(startPos, endPos);
	return result;
}


//-----------------------------------------------------------------------------------------------
DebugRenderCommand* DebugRenderCommand::DebugArrow(const Vector3& startPos, const Vector3& endPos, bool canTimeout, float timeToLive /* = 0.f */, EDepthTestType dtt /* = DEPTHTEST */, const Rgba& color /* = WHITE */)
{
	DebugRenderCommand* result = new DebugRenderCommand(canTimeout, timeToLive, dtt, color);
	result->InitializeArrow(startPos, endPos);
	return result;
}


//-----------------------------------------------------------------------------------------------
DebugRenderCommand* DebugRenderCommand::DebugAABB3(const Vector3& mins, const Vector3& maxs, bool canTimeout, 
	float timeToLive /* = 0.f */, EDepthTestType dtt /* = DEPTHTEST */, const Rgba& color /* = WHITE */, const Rgba& faceColor /* = WHITE */)
{
	DebugRenderCommand* result = new DebugRenderCommand(canTimeout, timeToLive, dtt, color);
	result->InitializeAABB3(mins, maxs, color, faceColor);
	return result;
}


//-----------------------------------------------------------------------------------------------
DebugRenderCommand* DebugRenderCommand::DebugSphere(const Vector3& position, float radius, bool canTimeout, float timeToLive /* = 0.f */, EDepthTestType dtt /* = DEPTHTEST */, const Rgba& color /* = WHITE */)
{
	DebugRenderCommand* result = new DebugRenderCommand(canTimeout, timeToLive, dtt, color);
	result->InitializeSphere(position, radius);
	return result;
}

#endif