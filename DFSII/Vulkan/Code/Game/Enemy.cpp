#include "Game/Enemy.hpp"


//-----------------------------------------------------------------------------------------------
const float Enemy::BURST_TIME = 2.5f;


//-----------------------------------------------------------------------------------------------
Enemy::Enemy(const Vector2& position, bool isJohnCena)
{
	if (isJohnCena)
	{
		m_resource = SpriteResource::GetResource("johncena");
	}
	else
	{
		m_resource = SpriteResource::GetResource("lucas");
	}

	m_transform.position = Vector3(position.x, position.y, 0.f);
	m_targetLayer = ENEMY_LAYER;
	m_radius = .5f;
	m_health = 10;
	m_timeSinceLastBurst = 0.f;
	m_needsBurst = false;
}


//-----------------------------------------------------------------------------------------------
void Enemy::Update(float deltaSeconds)
{
	m_timeSinceLastBurst += deltaSeconds;

	if (m_timeSinceLastBurst >= BURST_TIME)
	{
		m_timeSinceLastBurst = 0.f;

		m_needsBurst = true;
	}
}