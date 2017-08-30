#pragma once

#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
template<typename T>
class Range
{
public:
	Range() : m_min(T()), m_max(T()) {}
	Range(const T& min, const T& max) : m_min(min), m_max(max) {}
	Range(const T& uniformVal) : m_min(uniformVal), m_max(uniformVal) {}
	T Get(float t) const
	{
		return Lerp(m_min, m_max, Clampf(t, 0.f, 1.f));
	}
	T GetRandom() const
	{
		return Get(GetRandomNormalized());
	}
	void SetRange(const T& min, const T& max) { m_min = min; m_max = max; }
	void GetRangeValues(T& outMin, T& outMax) const { outMin = m_min; outMax = m_max; }

private:
	T m_min;
	T m_max;
};