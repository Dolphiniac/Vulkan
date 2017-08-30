#include "Game/Bullet.hpp"


//-----------------------------------------------------------------------------------------------
const float Bullet::BULLET_RADIUS = .5f;
const float Bullet::BULLET_TIME_TO_LIVE = 5.f;


//-----------------------------------------------------------------------------------------------
Bullet::Bullet(ERenderLayer layer, const Vector2& initialVelocity)
{
	m_resource = SpriteResource::GetResource("bullet");
	m_targetLayer = layer;

	m_radius = BULLET_RADIUS;
	m_timeAlive = 0.f;
	m_shouldDie = false;
	m_velocity = initialVelocity;
}


//-----------------------------------------------------------------------------------------------
void Bullet::Update(float deltaSeconds)
{
	m_timeAlive += deltaSeconds;
	if (m_timeAlive >= BULLET_TIME_TO_LIVE)
	{
		m_shouldDie = true;
	}

	m_transform.TranslateBy(Vector3(m_velocity.x * deltaSeconds, m_velocity.y * deltaSeconds, 0.f));
}