#include "Engine/Model/FBX.hpp"
#include "Engine/Math/Matrix44Stack.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Quantum/Math/MathCommon.h"

#ifdef TOOLS_BUILD

#include <fbxsdk.h>
#pragma comment(lib, "libfbxsdk-md")
#include "Game/TheGame.hpp"
#include "Engine/Core/ConsoleCommand.hpp"


//-----------------------------------------------------------------------------------------------
static void PrintAttribute(FbxNodeAttribute* attribute, int depth)
{
	if (!attribute)
	{
		return;
	}

	FbxNodeAttribute::EType type = attribute->GetAttributeType();

	std::string typeName = type == FbxNodeAttribute::EType::eSkeleton ? "skeleton" : "mesh";
	std::string attribName = attribute->GetName();

	ConsolePrintf(WHITE, "%*s- type='%s', name='%s'", depth * 2, " ", typeName.c_str(), attribName.c_str());
}


//-----------------------------------------------------------------------------------------------
static void PrintNode(FbxNode* node, int depth)
{
	ConsolePrintf(WHITE, "%*sNode [%s]", depth, " ", node->GetName());
	for (int i = 0; i < node->GetNodeAttributeCount(); i++)
	{
		PrintAttribute(node->GetNodeAttributeByIndex(i), depth);
	}
	for (int i = 0; i < node->GetChildCount(); i++)
	{
		PrintNode(node->GetChild(i), depth + 1);
	}

}


//-----------------------------------------------------------------------------------------------
void FbxList(const std::string& filename)
{
	UNUSED(filename);
	FbxManager* fbxManager = FbxManager::Create();
	if (!fbxManager)
	{
		return;
	}
	FbxIOSettings* ioSettings = FbxIOSettings::Create(fbxManager, IOSROOT);

	fbxManager->SetIOSettings(ioSettings);

	FbxImporter* importer = FbxImporter::Create(fbxManager, "");

	importer->Initialize(filename.c_str(), -1, ioSettings);

	FbxScene* scene = FbxScene::Create(fbxManager, "");
	importer->Import(scene);

	FbxNode* root = scene->GetRootNode();
	PrintNode(root, 0);

	FBX_SAFE_DESTROY(scene);
	FBX_SAFE_DESTROY(importer);
	FBX_SAFE_DESTROY(ioSettings);
	FBX_SAFE_DESTROY(fbxManager);

}


//-----------------------------------------------------------------------------------------------
// static Vector3 ToVec3(const FbxVector4& fbxPos)
// {
// 	return Vector3((float)fbxPos.mData[0], (float)fbxPos.mData[1], (float)fbxPos.mData[2]);
// }


//-----------------------------------------------------------------------------------------------
static Vector4 ToVec4(const FbxDouble4& vec)
{
	return Vector4((float)vec.mData[0], (float)vec.mData[1], (float)vec.mData[2], (float)vec.mData[3]);
}


//-----------------------------------------------------------------------------------------------
static Matrix44 ToEngineMatrix(const FbxMatrix& basis)
{
	Matrix44 result;
	Vector4 zero = ToVec4(basis.mData[0]);
	Vector4 one = ToVec4(basis.mData[1]);
	Vector4 two = ToVec4(basis.mData[2]);
	Vector4 three = ToVec4(basis.mData[3]);

	result.SetRows(zero, one, two, three);

	return result;
}


//-----------------------------------------------------------------------------------------------
static bool GetPosition(Vector3* pos, const Matrix44& transform, const FbxMesh* mesh, int polygonIndex, int vertexIndex)
{
	FbxVector4 fbxPos;
	int controlIndex = mesh->GetPolygonVertex(polygonIndex, vertexIndex);
	fbxPos = mesh->GetControlPointAt(controlIndex);

	Vector4 pos4 = ToVec4(fbxPos);
	pos4.w = 1.f;
	*pos = (pos4 * transform).XYZ();
	return true;
}


//-----------------------------------------------------------------------------------------------
template <typename T, typename U>
static bool GetObjectFromElement(const FbxMesh* mesh, int polygonIndex, int vertexIndex, const T* element, U* uv)
{
	if (!element)
	{
		return false;
	}

	switch (element->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
	{
		int controlIndex = mesh->GetPolygonVertex(polygonIndex, vertexIndex);
		switch (element->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			if (controlIndex < element->GetDirectArray().GetCount())
			{
				*uv = element->GetDirectArray().GetAt(controlIndex);
				return true;
			}
			break;
		}
		case FbxGeometryElement::eIndexToDirect:
		{
			if (controlIndex < element->GetIndexArray().GetCount())
			{
				int index = element->GetIndexArray().GetAt(controlIndex);
				*uv = element->GetDirectArray().GetAt(index);
				return true;
			}
			break;
		}
		default:
			break;
		}
		break;
	}
	case FbxGeometryElement::eByPolygonVertex:
	{
		int directVertexIndex = polygonIndex * 3 + vertexIndex;
		switch (element->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			if (directVertexIndex < element->GetDirectArray().GetCount())
			{
				*uv = element->GetDirectArray().GetAt(directVertexIndex);
				return true;
			}
			break;
		}
		case FbxGeometryElement::eIndexToDirect:
		{
			if (directVertexIndex < element->GetIndexArray().GetCount())
			{
				int index = element->GetIndexArray().GetAt(directVertexIndex);
				*uv = element->GetDirectArray().GetAt(index);
				return true;
			}
			break;
		}
		default:
			break;
		}
		break;
	}
	default:
		break;
	}

	return false;
}


static bool GetUV(Vector2* uv, const FbxMesh* mesh, int polygonIndex, int vertexIndex, int uvIndex)
{
	FbxVector2 fbxUV;
	const FbxGeometryElementUV* uvs = mesh->GetElementUV(uvIndex);
	if (GetObjectFromElement<FbxGeometryElementUV, FbxVector2>(mesh, polygonIndex, vertexIndex, uvs, &fbxUV))
	{
		*uv = Vector2((float)fbxUV.mData[0], (float)fbxUV.mData[1]);
		return true;
	}

	return false;
}

bool GetNormal(Vector3* normal, const FbxMesh* mesh, const Matrix44& transform, int polygonIndex, int vertexIndex, int normalIndex)
{
	FbxVector4 fbxNormal;
	const FbxGeometryElementNormal* normals = mesh->GetElementNormal(normalIndex);
	if (GetObjectFromElement<FbxGeometryElementNormal, FbxVector4>(mesh, polygonIndex, vertexIndex, normals, &fbxNormal))
	{
		Vector4 norm = ToVec4(fbxNormal);
		norm.w = 0.f;
		norm *= transform;
		*normal = norm.XYZ();
		return true;
	}

	return false;
}

bool GetTangent(Vector3* tangent, const FbxMesh* mesh, const Matrix44& transform, int polygonIndex, int vertexIndex, int tangentIndex)
{
	FbxVector4 fbxTangent;
	const FbxGeometryElementTangent* tangents = mesh->GetElementTangent(tangentIndex);
	if (GetObjectFromElement<FbxGeometryElementTangent, FbxVector4>(mesh, polygonIndex, vertexIndex, tangents, &fbxTangent))
	{
		Vector4 tan = ToVec4(fbxTangent);
		tan.w = 0.f;
		tan *= transform;
		*tangent = tan.XYZ();
		return true;
	}

	return false;
}

bool GetBitangent(Vector3* bitangent, const FbxMesh* mesh, const Matrix44& transform, int polygonIndex, int vertexIndex, int bitangentIndex)
{
	FbxVector4 fbxBitangent;
	const FbxGeometryElementBinormal* bitangents = mesh->GetElementBinormal(bitangentIndex);
	if (GetObjectFromElement<FbxGeometryElementBinormal, FbxVector4>(mesh, polygonIndex, vertexIndex, bitangents, &fbxBitangent))
	{
		Vector4 bitan = ToVec4(fbxBitangent);
		bitan.w = 0.f;
		bitan *= transform;
		*bitangent = bitan.XYZ();
		return true;
	}

	return false;
}

bool GetColor(float4* color, const FbxMesh* mesh, int polygonIndex, int vertexIndex)
{
	int matIndex = mesh->GetElementMaterial()->GetIndexArray().GetFirst();
	FbxSurfaceMaterial* surfaceMat = mesh->GetNode()->GetMaterial(matIndex);
	if (surfaceMat->GetClassId() == FbxSurfacePhong::ClassId)
	{
		FbxSurfacePhong* surfacePhong = (FbxSurfacePhong*)surfaceMat;
		FbxDouble3 col = surfacePhong->Diffuse.Get();
		*color = float4((float)col.mData[0], (float)col.mData[1], (float)col.mData[2], 1.f);
		return true;
	}
	else if (surfaceMat->GetClassId() == FbxSurfaceLambert::ClassId)
	{
		FbxSurfaceLambert* surfaceLambert = (FbxSurfaceLambert*)surfaceMat;
		FbxDouble3 col = surfaceLambert->Diffuse.Get();
		*color = float4((float)col.mData[0], (float)col.mData[1], (float)col.mData[2], 1.f);
		return true;
	}
	else
	{
		FbxColor fbxColor;
		const FbxGeometryElementVertexColor* colors = mesh->GetElementVertexColor();
		if (GetObjectFromElement<FbxGeometryElementVertexColor, FbxColor>(mesh, polygonIndex, vertexIndex, colors, &fbxColor))
		{
			*color = float4((float)fbxColor.mRed, (float)fbxColor.mGreen, (float)fbxColor.mBlue, (float)fbxColor.mAlpha);
			return true;
		}
		return false;
	}
}


struct SkinWeight
{
	IntVector4 indices = { 0, 0, 0, 0 };
	Vector4 weights = { 1.f, 0.f, 0.f, 0.f };
};

//-----------------------------------------------------------------------------------------------
static void ImportVertex(MeshBuilder* mb, const Matrix44& transform, const FbxMesh* mesh, int polygonIndex, int vertexIndex, const std::vector<SkinWeight>& skinWeights)
{
	Vector3 normal;
	if (GetNormal(&normal, mesh, transform, polygonIndex, vertexIndex, 0))
	{
		mb->SetNormal(normal);
	}

	Vector3 tangent;
	if (GetTangent(&tangent, mesh, transform, polygonIndex, vertexIndex, 0))
	{
		mb->SetTangent(tangent);
	}

	Vector3 bitangent;
	if (GetBitangent(&bitangent, mesh, transform, polygonIndex, vertexIndex, 0))
	{
		mb->SetBitangent(bitangent);
	}

	Vector2 uv;
	if (GetUV(&uv, mesh, polygonIndex, vertexIndex, 0))
	{
		mb->SetUV(uv.x, uv.y);
	}

	float4 color;
	if (GetColor(&color, mesh, polygonIndex, vertexIndex))
	{
		mb->SetColor(color);
	}

	int controlIndex = mesh->GetPolygonVertex(polygonIndex, vertexIndex);
	if (controlIndex < (int)skinWeights.size())
	{
		mb->SetBoneWeights(skinWeights[controlIndex].indices, skinWeights[controlIndex].weights);
	}
	else
	{
		mb->ClearBoneWeights();
	}

	Vector3 pos;
	if (GetPosition(&pos, transform, mesh, polygonIndex, vertexIndex))
	{
		mb->AddVertex(pos);
	}
}



//-----------------------------------------------------------------------------------------------
static bool HasSkinWeights(const FbxMesh* mesh)
{
	int deformerCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
	return (deformerCount > 0);
}


//-----------------------------------------------------------------------------------------------
static void AddHighestWeight(SkinWeight* skinWeight, int jointIndex, float weight)
{
	int lowestIndex = 0;
	float lowestWeight = skinWeight->weights.x;
	if (skinWeight->weights.y < lowestWeight)
	{
		lowestWeight = skinWeight->weights.y;
		lowestIndex = 1;
	}
	if (skinWeight->weights.z < lowestWeight)
	{
		lowestWeight = skinWeight->weights.z;
		lowestIndex = 2;
	}
	if (skinWeight->weights.w < lowestWeight)
	{
		lowestWeight = skinWeight->weights.w;
		lowestIndex = 3;
	}

	if (weight > lowestWeight)
	{
		switch (lowestIndex)
		{
		case 0:
			skinWeight->weights.x = weight;
			skinWeight->indices.x = jointIndex;
			break;
		case 1:
			skinWeight->weights.y = weight;
			skinWeight->indices.y = jointIndex;
			break;
		case 2:
			skinWeight->weights.z = weight;
			skinWeight->indices.z = jointIndex;
			break;
		case 3:
			skinWeight->weights.w = weight;
			skinWeight->indices.w = jointIndex;
			break;
		}
	}
}
static int GetJointIndexForNode(Skeleton* skeleton, FbxNode* node)
{
	std::string name = node->GetName();
	int result;
	for (result = 0; result < (int)skeleton->m_names.size(); result++)
	{
		if (name == skeleton->m_names[result])
		{
			return result;
		}
	}

	return -1;
}


//-----------------------------------------------------------------------------------------------
static void GetSkinWeights(SceneImport* import, std::vector<SkinWeight>& skinWeights, const FbxMesh* mesh)
{
	for (size_t i = 0; i < skinWeights.size(); i++)
	{
		skinWeights[i].indices = { 0, 0, 0, 0 };
		skinWeights[i].weights = { 0.f, 0.f, 0.f, 0.f };
	}
	int deformerCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
	for (int deformerIndex = 0; deformerIndex < deformerCount; deformerIndex++)
	{
		FbxSkin* skin = (FbxSkin*)mesh->GetDeformer(deformerIndex, FbxDeformer::eSkin);
		if (!skin)
		{
			continue;
		}

		for (int clusterIndex = 0; clusterIndex < skin->GetClusterCount(); clusterIndex++)
		{
			FbxCluster* cluster = skin->GetCluster(clusterIndex);
			FbxNode* linkNode = cluster->GetLink();
			if (!linkNode)
			{
				continue;
			}

			int jointIndex = GetJointIndexForNode(import->m_skeletons[0], linkNode);
			if (jointIndex == -1)
			{
				continue;
			}

			int* indices = cluster->GetControlPointIndices();
			double* weights = cluster->GetControlPointWeights();

			for (int i = 0; i < cluster->GetControlPointIndicesCount(); i++)
			{
				int controlIndex = indices[i];
				double weight = weights[i];

				SkinWeight* skinWeight = &skinWeights[controlIndex];
				AddHighestWeight(skinWeight, jointIndex, (float)weight);
			}
		}
	}

	for (size_t i = 0; i < skinWeights.size(); i++)
	{
		float totalWeight = skinWeights[i].weights.x + skinWeights[i].weights.y + skinWeights[i].weights.z + skinWeights[i].weights.w;
		if (totalWeight > 0.f)
		{
			skinWeights[i].weights.x /= totalWeight;
			skinWeights[i].weights.y /= totalWeight;
			skinWeights[i].weights.z /= totalWeight;
			skinWeights[i].weights.w /= totalWeight;
		}
		else
		{
			skinWeights[i].weights = { 1.f, 0.f, 0.f, 0.f };
		}
	}
}


//-----------------------------------------------------------------------------------------------
static Matrix44 GetGeometricTransform(FbxNode* node)
{
	Matrix44 result;
	if (node && node->GetNodeAttribute())
	{
		FbxVector4 geoTrans = node->GetGeometricTranslation(FbxNode::eSourcePivot);
		FbxVector4 geoRot = node->GetGeometricRotation(FbxNode::eSourcePivot);
		FbxVector4 geoScale = node->GetGeometricScaling(FbxNode::eSourcePivot);

		FbxMatrix geoMat;
		geoMat.SetTRS(geoTrans, geoRot, geoScale);

		result = ToEngineMatrix(geoMat);
	}

	return result;
}


//-----------------------------------------------------------------------------------------------
static void ImportMesh(SceneImport* import, const FbxMesh* mesh, const Matrix44Stack& stack, const std::string& name)
{
	//ASSERT_OR_DIE(import->m_skeletons.size() > 0, "No skeletons!  Can't skin");
	
	MeshBuilder* mb = new MeshBuilder(name);

	ASSERT_OR_DIE(mesh->IsTriangleMesh(), "MESH IS INVALID");

	mb->Begin(R_TRIANGLES, false /*use ibo bool*/);

	Matrix44 transform = stack.Top();

	transform = GetGeometricTransform(mesh->GetNode()) * transform;

	std::vector<SkinWeight> skinWeights;
	int controlPointCount = mesh->GetControlPointsCount();
	skinWeights.resize(controlPointCount);
	if (HasSkinWeights(mesh))
	{
		GetSkinWeights(import, skinWeights, mesh);
	}
	else
	{
		SkinWeight weight;
		weight.weights = { 1.f, 0.f, 0.f, 0.f };
		weight.indices = { 0, 0, 0, 0 };
		FbxNode* node = mesh->GetNode();
		int jointIndex = -1;
		while (node)
		{
			for (int i = 0; i < node->GetNodeAttributeCount(); i++)
			{
				FbxNodeAttribute* attr = node->GetNodeAttributeByIndex(i);
				if (attr->GetAttributeType() == FbxNodeAttribute::eSkeleton)
				{
					jointIndex = GetJointIndexForNode(import->m_skeletons[0], node);
					break;
				}
			}
			if (jointIndex != -1)
			{
				break;
			}
			node = node->GetParent();
		}
		if (jointIndex != -1)
		{
			weight.indices.x = jointIndex;
		}
		for (int i = 0; i < controlPointCount; i++)
		{
			skinWeights[i] = weight;
		}
	}

	int polyCount = mesh->GetPolygonCount();
	for (int polygonIndex = 0; polygonIndex < polyCount; polygonIndex++)
	{
		int vertexCount = mesh->GetPolygonSize(polygonIndex);
		ASSERT_OR_DIE(vertexCount == 3, "MESH DOESN'T HAVE TRIANGLES, WATTTTTT");
		for (int vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
		{
			ImportVertex(mb, transform, mesh, polygonIndex, vertexIndex, skinWeights);
		}
	}

	mb->GenerateTangentSpace(false);
	const FbxGeometryElementMaterial* material = mesh->GetElementMaterial();
	fbxsdk::FbxLayerElementArrayTemplate<int>& indices = material->GetIndexArray();
	int materialIndex = indices.GetFirst();
	FbxSurfaceMaterial* actualMaterial = mesh->GetNode()->GetMaterial(materialIndex);
	const char* matName = actualMaterial->GetName();
	mb->SetMat(matName);
	import->m_meshes.push_back(mb);
}


//-----------------------------------------------------------------------------------------------
static void ImportAttribute(SceneImport* import, const FbxNodeAttribute* attribute, const Matrix44Stack& stack, const std::string& name)
{
	if (!attribute)
	{
		return;
	}
	
	switch (attribute->GetAttributeType())
	{
		case FbxNodeAttribute::eMesh:
			ImportMesh(import, (FbxMesh*)attribute, stack, name);
			break;

		default:
			break;
	}
}


//-----------------------------------------------------------------------------------------------
Matrix44 GetNodeTransform(FbxNode* node)
{
	FbxMatrix localMatrix = node->EvaluateLocalTransform();

	return ToEngineMatrix(localMatrix);
}


//-----------------------------------------------------------------------------------------------
static void ImportSceneMeshes(SceneImport* import, FbxNode* node, Matrix44Stack& stack, const std::string& name)
{
	if (!node)
	{
		return;
	}

	Matrix44 nodeTransform = GetNodeTransform(node);
	stack.Push(nodeTransform);

	for (int i = 0; i < node->GetNodeAttributeCount(); i++)
	{
		ImportAttribute(import, node->GetNodeAttributeByIndex(i), stack, name);
	}

	for (int i = 0; i < node->GetChildCount(); i++)
	{
		ImportSceneMeshes(import, node->GetChild(i), stack, name);
	}

	stack.Pop();
}


//-----------------------------------------------------------------------------------------------
static void TriangulateScene(FbxScene* scene)
{
	FbxGeometryConverter converter(scene->GetFbxManager());
	converter.Triangulate(scene, true);
}

#include "Engine/Model/AnimationCurve.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
static std::vector<TransformationCurve> GetCurvesFromProperty(FbxVector4 evaluatedTransform, FbxPropertyT<FbxDouble3> prop, FbxAnimLayer* layer, float startTime, float animationTime)
{
	std::vector<TransformationCurve> result;
	std::string nodeComponents[3];
	FbxAnimCurveNode* animNode = prop.GetCurveNode();
	if (!animNode || (animNode && animNode->GetChannelsCount() == 0))
	{
		FbxVector4 transformVec = evaluatedTransform;
		TransformationCurve transCurve1, transCurve2, transCurve3;
		KeyFrame frame;
		frame.timeIntoAnimation = 0.f;
		frame.type = CONSTANT;
		frame.transformationChannelValue = (float)transformVec.mData[0];
		transCurve1.InsertKeyFrame(frame);
		frame.transformationChannelValue = (float)transformVec.mData[1];
		transCurve2.InsertKeyFrame(frame);
		frame.transformationChannelValue = (float)transformVec.mData[2];
		transCurve3.InsertKeyFrame(frame);
		result.push_back(transCurve1);
		result.push_back(transCurve2);
		result.push_back(transCurve3);
		return result;
	}
	nodeComponents[0] = FBXSDK_CURVENODE_COMPONENT_X;
	nodeComponents[1] = FBXSDK_CURVENODE_COMPONENT_Y;
	nodeComponents[2] = FBXSDK_CURVENODE_COMPONENT_Z;
	for (int i = 0; i < 3; i++)
	{
		TransformationCurve transCurve;
		FbxAnimCurve* animCurve = prop.GetCurve(layer, nodeComponents[i].c_str());
		if (animCurve)
		{
			FbxTimeSpan spanOfCurve;
			animCurve->GetTimeInterval(spanOfCurve);
			float curveStartTime = (float)spanOfCurve.GetStart().GetSecondDouble();
			float curveEndTime = (float)spanOfCurve.GetStop().GetSecondDouble();
			int keyFrames = animCurve->KeyGetCount();
			for (int keyFrameIndex = 0; keyFrameIndex < keyFrames; keyFrameIndex++)
			{
				KeyFrame frame;
				FbxAnimCurveKey keyFrame = animCurve->KeyGet(keyFrameIndex);
				frame.transformationChannelValue = keyFrame.GetValue();
				float keyFrameTime = (float)keyFrame.GetTime().GetSecondDouble();
				keyFrameTime = RangeMap(keyFrameTime, curveStartTime, curveEndTime, startTime, startTime + animationTime);
				frame.timeIntoAnimation = keyFrameTime;
				switch (keyFrame.GetInterpolation())
				{
				case FbxAnimCurveDef::eInterpolationConstant:
					frame.type = CONSTANT;
					break;
				case FbxAnimCurveDef::eInterpolationLinear:
					frame.type = LINEAR;
					break;
				case FbxAnimCurveDef::eInterpolationCubic:
					frame.type = CUBIC;
					switch (keyFrame.GetTangentMode())
					{
					case FbxAnimCurveDef::eTangentAuto:
						frame.leftSlope = keyFrame.GetDataFloat(FbxAnimCurveDef::eRightVelocity);
						frame.rightSlope = keyFrame.GetDataFloat(FbxAnimCurveDef::eNextLeftVelocity);
						break;
					default:
						ERROR_RECOVERABLE("Unsupported cubic function type encountered");
					}
					break;
				default:
					ERROR_AND_DIE("Interpolation value for this keyframe is invalid");
				}
				transCurve.InsertKeyFrame(frame);
			}
		}
		else
		{
			FbxVector4 transformVec = evaluatedTransform;
			KeyFrame frame;
			frame.timeIntoAnimation = 0.f;
			frame.type = CONSTANT;
			frame.transformationChannelValue = (float)transformVec.mData[i];
			transCurve.InsertKeyFrame(frame);
		}
		result.push_back(transCurve);
	}

	return result;
}


//-----------------------------------------------------------------------------------------------
static void AddAnimationCurveFor(SceneImport* import, FbxNode* node, FbxAnimLayer* layer, const Matrix44& localTransformForJoint)
{
	Motion* motion = import->m_motions[0];
	AnimationCurve* curve = new AnimationCurve();
	motion->AddAnimationCurve(curve);
	curve->m_transformationInverse = localTransformForJoint.Inverse();
	FbxAnimCurveFilterUnroll filter;
	FbxAnimCurveNode* animNode = node->LclRotation.GetCurveNode();
	if (animNode && filter.NeedApply(*animNode))
	{
		filter.SetQualityTolerance(.25);
		filter.SetTestForPath(true);
		FbxTime time(0);
		filter.SetStartTime(time);
		ASSERT_OR_DIE(filter.Apply(*animNode), "WAT");
	}
	std::vector<TransformationCurve> rotCurves = GetCurvesFromProperty(node->EvaluateLocalRotation(), node->LclRotation, layer, motion->startTime, motion->m_totalLengthOfAnimation);
	for (size_t i = 0; i < rotCurves.size(); i++)
	{
		curve->m_rotation[i] = rotCurves[i];
	}
	std::vector<TransformationCurve> scaleCurves = GetCurvesFromProperty(node->EvaluateLocalScaling(), node->LclScaling, layer, motion->startTime, motion->m_totalLengthOfAnimation);
	for (size_t i = 0; i < scaleCurves.size(); i++)
	{
		curve->m_scaling[i] = scaleCurves[i];
	}
	std::vector<TransformationCurve> transCurves = GetCurvesFromProperty(node->EvaluateLocalTranslation(), node->LclTranslation, layer, motion->startTime, motion->m_totalLengthOfAnimation);
	for (size_t i = 0; i < transCurves.size(); i++)
	{
		curve->m_translation[i] = transCurves[i];
	}
}


//-----------------------------------------------------------------------------------------------
static Skeleton* ImportSkeleton(SceneImport* import, Matrix44Stack& stack, Skeleton* skeleton, int parentBoneIndex, FbxSkeleton* fbxSkeleton, FbxAnimLayer* layer, const Matrix44& importTransform)
{
	Skeleton* result = nullptr;
	if (fbxSkeleton->IsSkeletonRoot())
	{
		result = new Skeleton();
		result->importTransform = importTransform;
		import->m_skeletons.push_back(result);
	}
	else
	{
		result = skeleton;
		ASSERT_OR_DIE(result, "Passed skeleton is nullptr!");
	}

	Matrix44 geoTransform = GetGeometricTransform(fbxSkeleton->GetNode());
	Matrix44 localTransform = GetNodeTransform(fbxSkeleton->GetNode());

	FbxNode* node = fbxSkeleton->GetNode();
	FbxNode::EPivotSet pivot = FbxNode::eSourcePivot;
	FbxEuler::EOrder order;
	node->GetRotationOrder(pivot, order);
	ASSERT_OR_DIE(order == FbxEuler::eOrderXYZ, "Please hit me");

	FbxAMatrix fbxPre;
	fbxPre.SetR(node->GetPreRotation(pivot), order);

	FbxAMatrix fbxPost;
	fbxPost.SetR(node->GetPostRotation(pivot), order);

	JointMeta meta;
	meta.scalePivot = ToVec4(node->GetScalingPivot(pivot)).XYZ();
	meta.scaleOffset = ToVec4(node->GetScalingOffset(pivot)).XYZ();
	meta.rotationPivot = ToVec4(node->GetRotationPivot(pivot)).XYZ();
	meta.rotationOffset = ToVec4(node->GetRotationOffset(pivot)).XYZ();
	meta.preRot = ToEngineMatrix(fbxPre);
	meta.postRot = ToEngineMatrix(fbxPost.Inverse());
	result->AddJoint(fbxSkeleton->GetNode()->GetName(), parentBoneIndex, geoTransform, localTransform, meta);

	if (layer)
	{
		AddAnimationCurveFor(import, fbxSkeleton->GetNode(), layer, localTransform);
	}
	stack.Pop();

	return result;
}


//-----------------------------------------------------------------------------------------------
static void ImportSceneSkeletons(SceneImport* import, FbxNode* node, Matrix44Stack& stack, Skeleton* skeleton, int parentBoneIndex, FbxAnimLayer* layer, const Matrix44& importTransform)
{
	if (!node)
	{
		return;
	}

	Matrix44 mat = GetNodeTransform(node);
	stack.Push(mat);

	int attribCount = node->GetNodeAttributeCount();
	for (int attribIndex = 0; attribIndex < attribCount; attribIndex++)
	{
		FbxNodeAttribute* attrib = node->GetNodeAttributeByIndex(attribIndex);
		if (attrib && attrib->GetAttributeType() == FbxNodeAttribute::eSkeleton)
		{
			FbxSkeleton* fbxSkeleton = (FbxSkeleton*)attrib;
			Skeleton* newSkeleton = ImportSkeleton(import, stack, skeleton, parentBoneIndex, fbxSkeleton, layer, importTransform);

			if (newSkeleton)
			{
				skeleton = newSkeleton;
				parentBoneIndex = skeleton->GetLastAddedJointIndex();
			}
		}
	}

	int childCount = node->GetChildCount();
	for (int childIndex = 0; childIndex < childCount; childIndex++)
	{
		ImportSceneSkeletons(import, node->GetChild(childIndex), stack, skeleton, parentBoneIndex, layer, importTransform);
	}

	stack.Pop();
}


//-----------------------------------------------------------------------------------------------
static FbxAnimLayer* ImportSceneAnimation(SceneImport* import, FbxScene* scene)
{
	int animStacks = scene->GetSrcObjectCount<FbxAnimStack>();
	if (animStacks <= 0)
	{
		return nullptr;
	}
	ASSERT_OR_DIE(animStacks == 1, "Too many animations!");
	FbxAnimStack* stack = scene->GetSrcObject<FbxAnimStack>();
	FbxTimeSpan span = stack->GetLocalTimeSpan();
	Motion* motion = new Motion((float)span.GetDuration().GetSecondDouble());
	motion->startTime = (float)span.GetStart().GetSecondDouble();
	import->m_motions.push_back(motion);
	int animLayers = stack->GetMemberCount<FbxAnimLayer>();
	ASSERT_OR_DIE(animLayers == 1, "Must have exactly one layer!");
	FbxAnimLayer* layer = stack->GetMember<FbxAnimLayer>();

	return layer;
}


//-----------------------------------------------------------------------------------------------
static void ImportScene(SceneImport* import, FbxScene* scene, Matrix44Stack& stack, const std::string& name)
{
	UNUSED(name);
	TriangulateScene(scene);

	FbxNode* root = scene->GetRootNode();

	FbxAnimLayer* layer = ImportSceneAnimation(import, scene);
	//ImportSceneSkeletons(import, root, stack, nullptr, -1, layer, stack.Top());
	ImportSceneMeshes(import, root, stack, name);

	for (Skeleton* skel : import->m_skeletons)
	{
		skel->ApplyGeometricTransforms();
	}
}


//-----------------------------------------------------------------------------------------------
static Matrix44 GetSceneBasis(FbxScene* scene)
{
	FbxAxisSystem axisSystem = scene->GetGlobalSettings().GetAxisSystem();

	FbxAMatrix basis;
	axisSystem.GetMatrix(basis);

	return ToEngineMatrix(basis);
}


//-----------------------------------------------------------------------------------------------
SceneImport* FbxLoadSceneFromFile(const std::string& modelName, const std::string& filename, const Matrix44& engineBasis, bool isEngineBasisRightHanded, const Matrix44& transform /*= Matrix44::Identity*/)
{
	FbxManager* fbxManager = FbxManager::Create();
	if (!fbxManager)
	{
		return nullptr;
	}
	FbxIOSettings* ioSettings = FbxIOSettings::Create(fbxManager, IOSROOT);

	fbxManager->SetIOSettings(ioSettings);

	FbxImporter* importer = FbxImporter::Create(fbxManager, "");

	importer->Initialize(filename.c_str(), -1, ioSettings);

	FbxScene* scene = FbxScene::Create(fbxManager, "");
	importer->Import(scene);

	SceneImport* import = new SceneImport();
	Matrix44Stack stack;

	stack.Push(transform);
	stack.Push(engineBasis);

	Matrix44 sceneBasis = GetSceneBasis(scene);
	sceneBasis.Transpose();

	if (!isEngineBasisRightHanded)
	{
		Vector3 forward = sceneBasis.GetForward();
		sceneBasis.SetForward(-forward);
	}

	stack.Push(sceneBasis);

	ImportScene(import, scene, stack, modelName);

	return import;
}

#else


//-----------------------------------------------------------------------------------------------
void FbxList(const std::string& filename) { UNUSED(filename); }
SceneImport* FbxLoadSceneFromFile(const std::string& modelName, const std::string& filename, const Matrix44& engineBasis, bool isEngineBasisRightHanded, const Matrix44& transform) { UNUSED(filename); UNUSED(engineBasis);
																																				UNUSED(isEngineBasisRightHanded); UNUSED(transform); return nullptr;
																																				UNUSED(modelName);
}

#endif