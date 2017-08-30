#pragma once

#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Matrix44.hpp"


//-----------------------------------------------------------------------------------------------
class Transform
{
public:
	inline Transform();
	inline void TranslateBy(const Vector3& translation);
	Matrix44 GetTransformationMatrix() const;

public:
	Vector3 position;
	Vector3 rotation;
	float uniformScale;
};


//-----------------------------------------------------------------------------------------------
Transform::Transform()
	: position(Vector3::Zero)
	, rotation(Vector3::Zero)
	, uniformScale(1.f)
{
	
}


//-----------------------------------------------------------------------------------------------
void Transform::TranslateBy(const Vector3& translation)
{
	position += translation;
}