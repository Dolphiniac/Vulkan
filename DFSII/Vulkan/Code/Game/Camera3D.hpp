#pragma once

#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"


//-----------------------------------------------------------------------------------------------
class Camera3D
{
public:
	Vector3 GetForwardXYZ() const;
	Vector3 GetForwardXZ() const;
	Vector3 GetRightXZ() const;
public:
	static const float MOVEMENT_SPEED;
	Vector3 m_position;
	float m_yawDegreesAboutY;
	float m_pitchDegreesAboutX;
};