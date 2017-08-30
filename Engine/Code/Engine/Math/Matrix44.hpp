#pragma once

#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"


__declspec(align(16)) class Matrix44
{
public:
	float data[16];

public:
	inline Matrix44();
	inline void ToIdentity();
	void MakeProjOrthogonal(float nx, float fx, float ny, float fy, float nz, float fz);
	void MakeProjOrthogonal(float width, float height, float nz, float fz);
	void MakeOrthographic(float width, float height);
	void MakePerspective(float fovDegreesY, float aspect, float nz, float fz);
	void MakeLookAt(const Vector3& pos, const Vector3& up, const Vector3& target);
	void InvertOrthonormal();
	void SetTranslation(const Vector3& trans);
	void SetRotation(float yaw, float pitch, float roll);	//For right handed default system
	void SetUniformScale(float uniformScale);
	void SetNonUniformScale(const Vector3& scale);
	void NegateRow(uint32 rowIndex);
	void MakeRotationX(float degrees);
	void MakeRotationY(float degrees);
	void MakeRotationZ(float degrees);
	void MakeTransformationMatrix(float uniformScale, const Vector3& eulerRot, const Vector3& translation);
	void Transpose();
	void SetRows(const Vector4& one, const Vector4& two, const Vector4& three, const Vector4& four);
	Matrix44 Inverse() const;
	void Invert();
	Vector3 GetForward() const;
	void SetForward(const Vector3& forward);
	bool operator==(const Matrix44& otherMat) const;
	bool operator!=(const Matrix44& otherMat) const;
	void operator=(const Matrix44& otherMat);

	Matrix44 operator*(const Matrix44& rightMat) const;
	Matrix44 operator*(float scalar) const;
	Matrix44 operator+(const Matrix44& rightMat) const;
	float& operator[](int index);
	void operator*=(float scalar);
	friend Matrix44 operator*(float scalar, const Matrix44& mat);

	static Matrix44 MakeZForwardBasis();

	static Matrix44 Identity;
	static Matrix44 ZForward;
};


//-----------------------------------------------------------------------------------------------
Matrix44::Matrix44()
	: data
{
	1.f, 0.f, 0.f, 0.f,
	0.f, 1.f, 0.f, 0.f,
	0.f, 0.f, 1.f, 0.f,
	0.f, 0.f, 0.f, 1.f
}
{

}


//-----------------------------------------------------------------------------------------------
void Matrix44::ToIdentity()
{
	*this = Matrix44();
}