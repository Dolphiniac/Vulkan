#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"

#include <math.h>

#define STATIC


//-----------------------------------------------------------------------------------------------
STATIC IntVector2 IntVector2::Zero = IntVector2(0, 0);


//-----------------------------------------------------------------------------------------------
IntVector2::IntVector2(const IntVector3& toCopy)
: x(toCopy.x)
, y(toCopy.y)
{

}


//-----------------------------------------------------------------------------------------------
IntVector2::IntVector2(const Vector2& toCopy)
: x((int)floor(toCopy.x))
, y((int)floor(toCopy.y))
{
}


//-----------------------------------------------------------------------------------------------
IntVector2::IntVector2(const Vector3& toCopy)
: x((int)floor(toCopy.x))
, y((int)floor(toCopy.y))
{
}


//-----------------------------------------------------------------------------------------------
IntVector2::IntVector2(const std::string& toParse)
{
	std::vector<std::string> pieces = SplitOnDelimiter(toParse, ',');
	if (pieces.size() == 1)
	{
		x = y = std::stoi(pieces[0]);
	}
	else if (pieces.size() == 2)
	{
		x = std::stoi(pieces[0]);
		y = std::stoi(pieces[1]);
	}
	else
	{
		ERROR_AND_DIE(Stringf("Bad string %s, cannot parse to Vector2", toParse.c_str()));
	}
}