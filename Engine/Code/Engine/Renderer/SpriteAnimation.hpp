#pragma once
#include "Engine/Renderer/SpriteSheet.hpp"


//-----------------------------------------------------------------------------------------------
enum ESpriteAnimMode
{
	SPRITE_ANIM_MODE_PLAY_TO_END,
	SPRITE_ANIM_MODE_LOOPING,
// 	SPRITE_ANIM_MODE_PINGPONG,
	NUM_SPRITE_ANIM_MODES
};


//-----------------------------------------------------------------------------------------------
class SpriteSheet;
class AABB2;
class Texture;


//-----------------------------------------------------------------------------------------------
class SpriteAnimation
{
public:
	SpriteAnimation(const SpriteSheet& spriteSheet, float durationSeconds, ESpriteAnimMode mode, int startSpriteIndex, int endSpriteIndex);

	void Update(float deltaSeconds);
	AABB2 GetCurrentTexCoords() const;
	Texture* GetTexture() const;
	void Pause();
	void Resume();
	void Reset();
	bool IsFinished() const;
	bool IsPlaying() const { return m_isPlaying; }
	float GetDurationSeconds() const { return m_durationSeconds; }
	float GetSecondsElapsed() const { return m_secondsElapsed; }
	float GetSecondsRemaining() const;
	float GetFractionElapsed() const;
	float GetFractionRemaining() const;
	void SetSecondsElapsed(float secondsElapsed);
	void SetFractionElapsed(float fractionElapsed);
	void SetDurationSeconds(float durationSeconds);

private:
	SpriteSheet				m_spriteSheet;
	float					m_durationSeconds;
	ESpriteAnimMode			m_mode;
	int						m_startSpriteIndex;
	int						m_endSpriteIndex;
	bool					m_isPlaying;
	float					m_secondsElapsed;
	float					m_fractionPerIndex;
	float					m_fractionPerSecond;
};