#include "Engine/Actor/Actor.hpp"
#include "Engine/Core/ConsoleCommand.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "ThirdParty/XML/xml.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Model/Skeleton.hpp"
#include "Engine/Model/Motion.hpp"
#include "Engine/Core/Profiler.hpp"


//-----------------------------------------------------------------------------------------------
void Actor::Render()
{
	if (m_meshRenderers.empty())
		return;

	if (!ShouldRender())
		return;

	if (skeleton)
	{
		PROFILE_LOG_SECTION(skeletonUpdate);
		std::vector<Matrix44> morphTransforms;
		morphTransforms.reserve(skeleton->GetNumJoints());
		for (int i = 0; i < skeleton->GetNumJoints(); i++)
		{
			Matrix44 localTransform;
			int parentIndex = i;
			while (parentIndex != -1)
			{
				localTransform = localTransform * skeleton->m_localTransformMatrices[parentIndex];
				parentIndex = skeleton->m_parentIndices[parentIndex];
			}
			localTransform = localTransform.Inverse() * skeleton->m_currentWorldTransformationMatrices[i];
			morphTransforms.push_back(localTransform);
		}
		SetUniformMatrix44Array("gSkinningMatrices", skeleton->GetNumJoints(), &morphTransforms[0]);
	}
	
	for (MeshRenderer* m_meshRenderer : m_meshRenderers)
	{
		PROFILE_LOG_SECTION(meshRender);
		m_meshRenderer->UpdateModelMatrix(m_transform);
		m_meshRenderer->Render();
	}
}


//-----------------------------------------------------------------------------------------------
bool Actor::ShouldRender() const
{
	return true;
}


//-----------------------------------------------------------------------------------------------
void Actor::Tick(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}
extern Skeleton* loadedSkeleton;
#ifndef __USING_UWP
//-----------------------------------------------------------------------------------------------
Actor* Actor::LoadActorFromXML(const struct XMLNode& node)
{
	Actor* actor = new Actor();
	std::string actorName = node.getAttribute("name");
	XMLNode transform = node.getChildNode("Transform");
	const char* trans = transform.getAttribute("position");
	if (trans)
	{
		actor->m_transform.position = Vector3(trans);
	}
	const char* scale = transform.getAttribute("scale");
	if (scale)
	{
		try
		{
			actor->m_transform.uniformScale = std::stof(scale);
		}
		catch (const std::exception&)
		{
			ERROR_AND_DIE(Stringf("String %s cannot be parsed to a float", scale));
		}
	}
	const char* rot = transform.getAttribute("rotation");
	if (rot)
	{
		actor->m_transform.rotation = Vector3(rot);
	}

	std::vector<MeshBuilder*> meshes = MeshBuilder::ReadFromFile("Data/Actors/" + actorName + "/" + actorName + ".model");
	actor->skeleton = Skeleton::ReadFromFile("Data/Actors/" + actorName + "/" + actorName + ".skel");
	loadedSkeleton = actor->skeleton;
	int numMeshes = meshes.size();
	XMLNode shader = node.getChildNode("Model");
	std::string shaderName = shader.getAttribute("shader");
	XMLNode shaderMeta = XMLNode::parseFile("Data/Shaders/ShaderMeta.xml").getChildNode();
	Renum vertEnum = R_VERTEX_PCT;
	std::vector<std::string> uniformBlocks;
	for (int i = 0; i < shaderMeta.nChildNode(); i++)
	{
		XMLNode currShader = shaderMeta.getChildNode(i);
		if (currShader.getAttribute("name") == shaderName)
		{
			std::string vertexType = currShader.getChildNode("VertexType").getAttribute("name");
			if (vertexType == "Vertex_PCT")
			{
				vertEnum = R_VERTEX_PCT;
			}
			else if (vertexType == "Vertex_PCTTB")
			{
				vertEnum = R_VERTEX_PCTTB;
			}
			else if (vertexType == "Vertex_PCTN")
			{
				vertEnum = R_VERTEX_PCTN;
			}
			else if (vertexType == "Vertex_PCTTBB")
			{
				vertEnum = R_VERTEX_PCTTBB;
			}
			else
			{
				ERROR_AND_DIE("Cannot create this mesh with no shader meta data");
			}

			XMLNode uniforms = currShader.getChildNode("Uniforms");
			int searchInt = 0;
			for (int j = 0; j < uniforms.nChildNode("UniformBlock"); j++)
			{
				XMLNode uniformBlock = uniforms.getChildNode("UniformBlock", &searchInt);
				uniformBlocks.push_back(uniformBlock.getAttribute("name"));
			}
		}
	}
	for (int i = 0; i < numMeshes; i++)
	{
		Material* mat = Material::CreateOrGetMaterial(shaderName);// Material(shaderName);
		MeshBuilder* builder = meshes[i];
		Mesh* mesh = builder->ConstructMesh(vertEnum);
		MeshRenderer* renderer = new MeshRenderer(mesh, mat, true);
		actor->m_meshRenderers.push_back(renderer);
		for (const std::string& blockName : uniformBlocks)
		{
			mat->BindUniformBlock(blockName);
		}
		std::string matName = meshes[i]->m_materialName;
		if (matName != "")
		{
			for (int childIndex = 0; childIndex < shader.nChildNode(); childIndex++)
			{
				XMLNode mapping = shader.getChildNode(childIndex);
				const char* currMatName = mapping.getAttribute("material");
				std::string currMatString;
				if (currMatName)
				{
					currMatString = currMatName;
					currMatString.push_back('\0');
				}
				if (currMatString == matName)
				{
					const char* diffuse = mapping.getAttribute("diffuse");
					if (diffuse)
					{
						renderer->SetUniformTexture("gDiffuseTex", Texture::CreateOrGetTexture("Data/Actors/" + actorName + "/Textures/" + diffuse));
					}
					break;
				}
			}
		}
	}
// 	for (int i = 0; i < numMeshes; i++)
// 	{
// 	}

	std::vector<std::string> motionPaths = FindFilesWith("Data/Actors/" + actorName + "/", "*.anim");
	for (const std::string& motionPath : motionPaths)
	{
		std::string prefix = motionPath.substr(("Data/Actors/" + actorName + "/").size() + 1);
		prefix = prefix.substr(0, prefix.size() - 5);
		Motion* motion = Motion::ReadFromFile(motionPath);
		motion->name = prefix;
		actor->m_motions.insert(std::make_pair(motion->name, motion));
	}

	for (MeshBuilder* builder : meshes)
	{
		delete builder;
	}

	return actor;
}

Actor* loadedActor = nullptr;
CONSOLE_COMMAND(ActorLoad, args)
{
	std::string actorName = args.GetNextArg();
	if (DoesFileExist("Data/Actors/" + actorName + "/" + actorName +  ".xml"))
	{
		XMLNode meta = XMLNode::parseFile(("Data/Actors/" + actorName + "/" + actorName + ".xml").c_str()).getChildNode();
		loadedActor = Actor::LoadActorFromXML(meta);
	}
	else
	{
		ConsolePrintf(RED, "%s.xml does not exist", actorName.c_str());
	}
}
#endif


//-----------------------------------------------------------------------------------------------
Actor::~Actor()
{
	for (MeshRenderer* mr : m_meshRenderers)
	{
		delete mr;
	}
	if (skeleton)
	{
		delete skeleton;
	}
	for (std::pair<const std::string, Motion*> p : m_motions)
	{
		delete p.second;
	}
}