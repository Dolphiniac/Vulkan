#pragma once

#include "Engine/Renderer/Sprite.hpp"


//-----------------------------------------------------------------------------------------------
class Bullet : public Sprite
{
	static const float BULLET_RADIUS;
	static const float BULLET_TIME_TO_LIVE;

public:
	Bullet(ERenderLayer layer, const Vector2& initialVelocity);
	virtual void Update(float deltaSeconds) override;

public:
	float m_radius;
	float m_timeAlive;
	bool m_shouldDie;
	bool m_isPlayer;
	Vector2 m_velocity;
};