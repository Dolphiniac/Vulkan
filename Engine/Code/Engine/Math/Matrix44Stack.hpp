#pragma once

#include "Engine/Math/Matrix44.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class Matrix44Stack
{
public:
	Matrix44Stack() { m_stack.push_back(Matrix44()); }
	bool IsEmpty() const { return m_stack.size() == 1; }
	Matrix44 Top() const { return m_stack.back(); }
	inline void Push(const Matrix44& transform);
	inline Matrix44 Pop();

private:
	std::vector<Matrix44> m_stack;
};


//-----------------------------------------------------------------------------------------------
void Matrix44Stack::Push(const Matrix44& transform)
{
	Matrix44 top = Top();
	Matrix44 nextTop = transform * top;
	m_stack.push_back(nextTop);
}


//-----------------------------------------------------------------------------------------------
Matrix44 Matrix44Stack::Pop()
{
	Matrix44 top = Top();
	if (!IsEmpty())
	{
		m_stack.pop_back();
	}

	return top;
}