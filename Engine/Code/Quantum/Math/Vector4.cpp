#include "Quantum/Math/Vector4.h"
#include "Quantum/Math/Vector3.h"
#include "Quantum/Math/Vector2.h"


//-----------------------------------------------------------------------------------------------
STATIC const QuVector4 QuVector4::Zero(0.f, 0.f, 0.f, 0.f);


//-----------------------------------------------------------------------------------------------
QuCVector2 QuCVector4::XY() const
{
	return QuCVector2(r, g);
}


//-----------------------------------------------------------------------------------------------
QuCVector3 QuCVector4::XYZ() const
{
	return QuCVector3(r, g, b);
}


//-----------------------------------------------------------------------------------------------
QuVector2 QuVector4::XY() const
{
	return QuVector2(x, y);
}


//-----------------------------------------------------------------------------------------------
QuVector3 QuVector4::XYZ() const
{
	return QuVector3(x, y, z);
}