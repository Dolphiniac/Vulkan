#include "Engine/Actor/Transform.hpp"


//-----------------------------------------------------------------------------------------------
Matrix44 Transform::GetTransformationMatrix() const
{
	Matrix44 result;
	result.MakeTransformationMatrix(uniformScale, rotation, position);
	return result;
}