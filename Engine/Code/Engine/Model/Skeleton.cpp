#include "Engine/Model/Skeleton.hpp"
#include "Engine/Core/BinaryReader.hpp"
#include "Engine/Core/BinaryWriter.hpp"


//-----------------------------------------------------------------------------------------------
Skeleton* loadedSkeleton = nullptr;


//-----------------------------------------------------------------------------------------------
void Skeleton::AddJoint(const std::string& name, int parentBoneIndex, const Matrix44& geometricTransform, const Matrix44& localTransform, const JointMeta& meta)
{
	m_names.push_back(name);
	m_parentIndices.push_back(parentBoneIndex);
	m_geometricTransformationMatrices.push_back(geometricTransform);
	m_localTransformMatrices.push_back(localTransform);
	m_currentWorldTransformationMatrices.push_back(Matrix44::Identity);
	SetWorldTransformForJoint(m_currentWorldTransformationMatrices.size() - 1, Matrix44::Identity);
	m_startWorldTransformationInverses.push_back(m_currentWorldTransformationMatrices[m_currentWorldTransformationMatrices.size() - 1].Inverse());
	m_jointMetadata.push_back(meta);
}


//-----------------------------------------------------------------------------------------------
int Skeleton::FindJointIndex(const std::string& name)
{
	int index = 0;
	for (auto iter = m_names.begin(); iter != m_names.end(); iter++, index++)
	{
		if (*iter == name)
		{
			break;
		}
	}
	
	return index;
}


//-----------------------------------------------------------------------------------------------
Joint* Skeleton::GetJoint(int jointIndex)
{
	Joint* result = new Joint();
	result->name = m_names[jointIndex];
	result->geometricTransform = m_geometricTransformationMatrices[jointIndex];
	result->localTransform = m_localTransformMatrices[jointIndex];
	result->meta = m_jointMetadata[jointIndex];
	result->parentIndex = m_parentIndices[jointIndex];
	result->currentWorldTransform = m_currentWorldTransformationMatrices[jointIndex];

	return result;
}


//-----------------------------------------------------------------------------------------------
void Skeleton::WriteToFile(const std::string& filepath)
{
	BinaryWriter writer;
	writer.Write(SKELETON_FILE_VERSION);
	writer.Write((int)m_names.size());
	writer.Write(importTransform);
	for (size_t i = 0; i < m_names.size(); i++)
	{
		writer.Write(m_names[i]);
		writer.Write(m_parentIndices[i]);
		writer.Write(m_geometricTransformationMatrices[i]);
		writer.Write(m_localTransformMatrices[i]);
		JointMeta& meta = m_jointMetadata[i];
		writer.Write(meta.rotationOffset);
		writer.Write(meta.rotationPivot);
		writer.Write(meta.scaleOffset);
		writer.Write(meta.scalePivot);
		writer.Write(meta.preRot);
		writer.Write(meta.postRot);
	}
	writer.WriteBufferToFile(filepath);
}


//-----------------------------------------------------------------------------------------------
Skeleton* Skeleton::ReadFromFile(const std::string& filepath)
{
	Skeleton* result = new Skeleton();
	BinaryReader reader(filepath);
	int version;
	reader.Read(version);
	int numJoints;
	reader.Read(numJoints);
	reader.Read(result->importTransform);
	for (int i = 0; i < numJoints; i++)
	{
		std::string name;
		reader.Read(name);
		result->m_names.push_back(name);
		int parentIndex;
		reader.Read(parentIndex);
		result->m_parentIndices.push_back(parentIndex);
		Matrix44 geometricTransform;
		reader.Read(geometricTransform);
		result->m_geometricTransformationMatrices.push_back(geometricTransform);
		Matrix44 localTransform;
		reader.Read(localTransform);
		result->m_localTransformMatrices.push_back(localTransform);
		JointMeta meta;
		reader.Read(meta.rotationOffset);
		reader.Read(meta.rotationPivot);
		reader.Read(meta.scaleOffset);
		reader.Read(meta.scalePivot);
		reader.Read(meta.preRot);
		reader.Read(meta.postRot);
		result->m_jointMetadata.push_back(meta);
		result->m_currentWorldTransformationMatrices.push_back(Matrix44::Identity);
		result->SetWorldTransformForJoint(i, Matrix44::Identity);
	}

	result->ApplyGeometricTransforms();

	return result;
}


//-----------------------------------------------------------------------------------------------
void Skeleton::SetWorldTransformForJoint(int jointIndex, const Matrix44& localTransformChange)
{
	Matrix44 worldTransform = m_localTransformMatrices[jointIndex] * localTransformChange;
	if (m_parentIndices[jointIndex] != -1)
	{
		worldTransform = worldTransform * m_currentWorldTransformationMatrices[m_parentIndices[jointIndex]];
	}
	else
	{
		worldTransform = worldTransform * importTransform;
	}

	m_currentWorldTransformationMatrices[jointIndex] = worldTransform;
}


//-----------------------------------------------------------------------------------------------
void Skeleton::ApplyGeometricTransforms()
{
	//Only call after all joints' world transforms have been updated.
	//This will apply geometric transforms, which should not be propagated to child bones
	for (int i = 0; i < GetNumJoints(); i++)
	{
		m_currentWorldTransformationMatrices[i] = m_geometricTransformationMatrices[i] * m_currentWorldTransformationMatrices[i];
	}
}

#include "Engine/Core/ConsoleCommand.hpp"
CONSOLE_COMMAND(SkelSave, args)
{
	if (!loadedSkeleton)
	{
		ConsolePrint("No skeleton loaded!", RED);
	};
	std::string filename = args.GetNextArg();
	std::string filepath = "Data/Actors/" + filename + "/" + filename + ".skel";
	loadedSkeleton->WriteToFile(filepath);
}

extern void MakeDebugPointsForSkeleton(Skeleton* skeleton);
CONSOLE_COMMAND(SkelLoad, args)
{
	std::string filename = args.GetNextArg();
	std::string filepath = "Data/Actors/" + filename + "/" + filename + ".skel";
	loadedSkeleton = Skeleton::ReadFromFile(filepath);
}