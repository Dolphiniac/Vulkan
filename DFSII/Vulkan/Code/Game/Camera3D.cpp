#include "Game/Camera3D.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
const float Camera3D::MOVEMENT_SPEED = 1.f;


//-----------------------------------------------------------------------------------------------
Vector3 Camera3D::GetForwardXYZ() const
{
	float z = CosDegrees(m_yawDegreesAboutY) * CosDegrees(m_pitchDegreesAboutX);
	float x = -SinDegrees(m_yawDegreesAboutY) * CosDegrees(m_pitchDegreesAboutX);
	float y = -SinDegrees(m_pitchDegreesAboutX);

	return Vector3(x, y, z);
}


//-----------------------------------------------------------------------------------------------
Vector3 Camera3D::GetForwardXZ() const
{
	Vector3 xyzVec = GetForwardXYZ();
	xyzVec.y = 0.f;

	xyzVec.Normalize();

	return xyzVec;
}


//-----------------------------------------------------------------------------------------------
Vector3 Camera3D::GetRightXZ() const
{
	Vector3 forwardXZ = GetForwardXZ();
	float temp = -forwardXZ.x;
	forwardXZ.x = forwardXZ.z;
	forwardXZ.z = temp;

	return forwardXZ;
}