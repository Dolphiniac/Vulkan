#pragma once

#include "Engine/Math/Matrix44.hpp"

#include <vector>
#include <string>


//-----------------------------------------------------------------------------------------------
extern class Skeleton* loadedSkeleton;


//-----------------------------------------------------------------------------------------------
struct JointMeta
{
	Vector3 scalePivot;
	Vector3 rotationPivot;
	Vector3 scaleOffset;
	Vector3 rotationOffset;
	Matrix44 preRot;
	Matrix44 postRot;

	Matrix44 CalculateFinalLocal(const Vector3& translation, const Matrix44& rotation, const Vector3& scaling) const
	{
		Matrix44 matTranslation;
		matTranslation.SetTranslation(translation);
		Matrix44 matScale;
		matScale.SetNonUniformScale(scaling);
		Matrix44 matScaleOffset;
		matScaleOffset.SetTranslation(scaleOffset);
		Matrix44 matScalePivot;
		matScalePivot.SetTranslation(scalePivot);
		Matrix44 matScalePivotInverse = matScalePivot.Inverse();
		Matrix44 matRotOffset;
		matRotOffset.SetTranslation(rotationOffset);
		Matrix44 matRotPivot;
		matRotPivot.SetTranslation(rotationPivot);
		Matrix44 matRotPivotInverse = matRotPivot.Inverse();

		return matScalePivotInverse * matScale * matScalePivot * matScaleOffset * matRotPivotInverse * postRot * rotation * preRot * matRotPivot * matRotOffset * matTranslation;
	}
};


//-----------------------------------------------------------------------------------------------
class Skeleton
{
	static const int SKELETON_FILE_VERSION = 1;
public:
	int GetLastAddedJointIndex() const { return m_names.size() - 1; };
	void AddJoint(const std::string& name, int parentBoneIndex, const Matrix44& geometricTransform, const Matrix44& localTransform, const JointMeta& meta);
	int FindJointIndex(const std::string& name);
	class Joint* GetJoint(int jointIndex);
	void WriteToFile(const std::string& filepath);
	static Skeleton* ReadFromFile(const std::string& filepath);
	int GetNumJoints() const { return m_names.size(); }
	void SetWorldTransformForJoint(int jointIndex, const Matrix44& worldTransform);
	void ApplyGeometricTransforms();

public:
	std::vector<std::string> m_names;
	std::vector<int> m_parentIndices;
	std::vector<Matrix44> m_localTransformMatrices;
	std::vector<Matrix44> m_startWorldTransformationInverses;
	std::vector<Matrix44> m_geometricTransformationMatrices;
	std::vector<Matrix44> m_currentWorldTransformationMatrices;
	Matrix44 importTransform;
	std::vector<JointMeta> m_jointMetadata;
};


//-----------------------------------------------------------------------------------------------
class Joint
{
public:
	std::string name;
	int parentIndex;
	Matrix44 geometricTransform;
	Matrix44 localTransform;
	Matrix44 currentWorldTransform;
	JointMeta meta;
};