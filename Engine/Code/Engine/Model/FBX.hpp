#pragma once

#include "Engine/Model/MeshBuilder.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Model/Skeleton.hpp"
#include "Engine/Model/Motion.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class SceneImport
{
public:
	inline ~SceneImport();

public:
	std::vector<MeshBuilder*> m_meshes;
	std::vector<Skeleton*> m_skeletons;
	std::vector<Motion*> m_motions;
};


//-----------------------------------------------------------------------------------------------
void FbxList(const std::string& filename);
SceneImport* FbxLoadSceneFromFile(const std::string& modelName, const std::string& filename, const Matrix44& engineBasis, bool isEngineBasisRightHanded, const Matrix44& transform = Matrix44::Identity);


//-----------------------------------------------------------------------------------------------
SceneImport::~SceneImport()
{
	for (MeshBuilder* mb : m_meshes)
	{
		delete mb;
	}
}