#include "Engine/Renderer/SpriteAnimation.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


//-----------------------------------------------------------------------------------------------
SpriteAnimation::SpriteAnimation(const SpriteSheet& spriteSheet, float durationSeconds, ESpriteAnimMode mode, int startSpriteIndex, int endSpriteIndex)
: m_spriteSheet(spriteSheet)
, m_durationSeconds(durationSeconds)
, m_mode(mode)
, m_startSpriteIndex(startSpriteIndex)
, m_endSpriteIndex(endSpriteIndex)
, m_isPlaying(true)
, m_secondsElapsed(0.f)
, m_fractionPerIndex(1.f / (m_endSpriteIndex - m_startSpriteIndex + 1))
, m_fractionPerSecond(1.f / durationSeconds)
{

}


//-----------------------------------------------------------------------------------------------
void SpriteAnimation::Update(float deltaSeconds)
{
	if (m_isPlaying)
	{
		m_secondsElapsed += deltaSeconds;
	}

	if (m_secondsElapsed >= m_durationSeconds)
	{
		switch (m_mode)
		{
		case SPRITE_ANIM_MODE_PLAY_TO_END:
			m_isPlaying = false;
			break;
		case SPRITE_ANIM_MODE_LOOPING:
			m_secondsElapsed = 0.f;
			break;
		default:
			ERROR_AND_DIE("Invalid mode for SpriteAnimation");
		}
	}
}


//-----------------------------------------------------------------------------------------------
AABB2 SpriteAnimation::GetCurrentTexCoords() const
{
	float fraction = GetFractionElapsed();
	float testFraction = 0.f;
	int currentIndex = m_startSpriteIndex;
	bool done = false;

	while (!done)
	{
		testFraction += m_fractionPerIndex;
		if (fraction <= testFraction)
		{
			done = true;
		}
		else
		{
			currentIndex++;
		}
	}

	return m_spriteSheet.GetTexCoordsForSpriteIndex(currentIndex);
}


//-----------------------------------------------------------------------------------------------
Texture* SpriteAnimation::GetTexture() const
{
	return m_spriteSheet.GetTexture();
}


//-----------------------------------------------------------------------------------------------
void SpriteAnimation::Pause()
{
	m_isPlaying = false;
}


//-----------------------------------------------------------------------------------------------
void SpriteAnimation::Resume()
{
	m_isPlaying = true;
}


//-----------------------------------------------------------------------------------------------
void SpriteAnimation::Reset()
{
	m_isPlaying = true;
	m_secondsElapsed = 0.f;
}


//-----------------------------------------------------------------------------------------------
bool SpriteAnimation::IsFinished() const
{
	return m_secondsElapsed >= m_durationSeconds && m_mode == SPRITE_ANIM_MODE_PLAY_TO_END;
}


//-----------------------------------------------------------------------------------------------
float SpriteAnimation::GetSecondsRemaining() const
{
	return m_durationSeconds - m_secondsElapsed;
}


//-----------------------------------------------------------------------------------------------
float SpriteAnimation::GetFractionElapsed() const
{
	return m_secondsElapsed * m_fractionPerSecond;
}


//-----------------------------------------------------------------------------------------------
float SpriteAnimation::GetFractionRemaining() const
{
	return 1.f - GetFractionElapsed();
}


//-----------------------------------------------------------------------------------------------
void SpriteAnimation::SetSecondsElapsed(float secondsElapsed)
{
	m_secondsElapsed = secondsElapsed;
}


//-----------------------------------------------------------------------------------------------
void SpriteAnimation::SetFractionElapsed(float fractionElapsed)
{
	m_secondsElapsed = fractionElapsed * m_durationSeconds;
}


//-----------------------------------------------------------------------------------------------
void SpriteAnimation::SetDurationSeconds(float durationSeconds)
{
	m_durationSeconds = durationSeconds;
	m_fractionPerSecond = 1.f / m_durationSeconds;
	Reset();
}