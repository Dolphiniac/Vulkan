#pragma once

#include "Engine/Renderer/Sprite.hpp"


//-----------------------------------------------------------------------------------------------
class Enemy : public Sprite
{
	static const float BURST_TIME;

public:
	Enemy(const Vector2& position, bool isJohnCena);
	virtual void Update(float deltaSeconds) override;

public:
	float m_radius;
	bool m_shouldDie;
	int m_health;
	float m_timeSinceLastBurst;
	bool m_needsBurst;
};