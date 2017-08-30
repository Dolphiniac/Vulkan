#include "Engine/Renderer/Sprite.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/SpriteRenderer.hpp"
#include "Engine/Renderer/MeshRenderer.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/XMLUtils.hpp"


//-----------------------------------------------------------------------------------------------
std::map<std::string, SpriteResource*> SpriteResource::s_spriteDatabase;


//-----------------------------------------------------------------------------------------------
Sprite* SpriteResource::GetInstance(const std::string& name, ERenderLayer layer)
{
	auto iter = s_spriteDatabase.find(name);
	ASSERT_OR_DIE(iter != s_spriteDatabase.end(), Stringf("Cannot find resource of name %s", name.c_str()));
	Sprite* result = new Sprite();
	result->m_resource = iter->second;
	result->m_targetLayer = layer;

	return result;
}


//-----------------------------------------------------------------------------------------------
SpriteResource* SpriteResource::Register(const std::string& name, const std::string& imageFilePath)
{
	Texture* tex = Texture::CreateOrGetTexture(imageFilePath);
	SpriteResource* newResource = new SpriteResource();
	newResource->m_filepath = imageFilePath;
	newResource->m_texture = tex;
	s_spriteDatabase.insert(std::make_pair(name, newResource));

	return newResource;
}


//-----------------------------------------------------------------------------------------------
void SpriteResource::UnloadDatabase()
{
	for (std::pair<const std::string, SpriteResource*>& p : s_spriteDatabase)
	{
		delete p.second;
	}

	s_spriteDatabase.clear();
}


//-----------------------------------------------------------------------------------------------
Sprite::Sprite()
{
	m_mesh = new Mesh();
	m_mesh->SetVertexType(R_VERTEX2_PCT);
	Vertex2_PCT vertices[4];
	Vertex2_PCT vertex;
	vertex.m_color = WHITE;
	vertex.m_texCoords = Vector2(0.f, 0.f);
	vertex.m_position = Vector2(-1.f, 1.f);
	vertices[0] = vertex;
	vertex.m_texCoords = Vector2(0.f, 1.f);
	vertex.m_position = Vector2(-1.f, -1.f);
	vertices[1] = vertex;
	vertex.m_texCoords = Vector2(1.f, 1.f);
	vertex.m_position = Vector2(1.f, -1.f);
	vertices[2] = vertex;
	vertex.m_texCoords = Vector2(1.f, 0.f);
	vertex.m_position = Vector2(1.f, 1.f);
	vertices[3] = vertex;
	//m_mesh->UpdateVBO(&vertices[0], 4, sizeof(Vertex2_PCT), R_DYNAMIC_DRAW);
	m_mesh->InitializeVBO(nullptr, 4, sizeof(Vertex2_PCT), R_DYNAMIC_DRAW);
	unsigned short indices[6] =
	{
		0, 1, 2,
		2, 3, 0
	};

	m_mesh->InitializeIBO(&indices[0], 6, sizeof(indices[0]));

	m_materialOverride = g_spriteRenderer->GetMaterial();

	m_meshRenderer = new MeshRenderer(m_mesh, m_materialOverride);
}


static void CheckVectorAgainstMinsAndMaxs(const Vector4 &vec, float &minX, float &maxX, float &minY, float &maxY)
{
	if (vec.x < minX)
	{
		minX = vec.x;
	}
	if (vec.x > maxX)
	{
		maxX = vec.x;
	}
	if (vec.y < minY)
	{
		minY = vec.y;
	}
	if (vec.y > maxY)
	{
		maxY = vec.y;
	}
}


//-----------------------------------------------------------------------------------------------
static AABB2 TransformBoundingBoxAndGetTransformedPoints(const AABB2& origBox, const Matrix44& transformationMatrix, Vector2& tl, Vector2& bl, Vector2& br, Vector2& tr)
{
	Vector4 tl4(origBox.mins.x, origBox.maxs.y, 0.f, 1.f);
	Vector4 bl4(origBox.mins.x, origBox.mins.y, 0.f, 1.f);
	Vector4 br4(origBox.maxs.x, origBox.mins.y, 0.f, 1.f);
	Vector4 tr4(origBox.maxs.x, origBox.maxs.y, 0.f, 1.f);

	tl4 = tl4 * transformationMatrix;
	bl4 = bl4 * transformationMatrix;
	br4 = br4 * transformationMatrix;
	tr4 = tr4 * transformationMatrix;

	float minX = tl4.x;
	float minY = tl4.y;
	float maxX = tl4.x;
	float maxY = tl4.y;

	CheckVectorAgainstMinsAndMaxs(bl4, minX, maxX, minY, maxY);
	CheckVectorAgainstMinsAndMaxs(br4, minX, maxX, minY, maxY);
	CheckVectorAgainstMinsAndMaxs(tr4, minX, maxX, minY, maxY);

	tl = Vector2(tl4.x, tl4.y);
	bl = Vector2(bl4.x, bl4.y);
	br = Vector2(br4.x, br4.y);
	tr = Vector2(tr4.x, tr4.y);

	return AABB2(minX, minY, maxX, maxY);
}


//-----------------------------------------------------------------------------------------------
void Sprite::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}


//-----------------------------------------------------------------------------------------------
static Matrix44 GetTotalTransform(const Sprite* sprite)
{
	if (!sprite)
	{
		return Matrix44::Identity;
	}

	return GetTotalTransform(sprite->parent) * sprite->m_transform.GetTransformationMatrix();
}


//-----------------------------------------------------------------------------------------------
void Sprite::Render() const
{
	Vector2 tl, bl, tr, br;
	Matrix44 totalTransform = GetTotalTransform(this);
	AABB2 currBoundingBox = TransformBoundingBoxAndGetTransformedPoints(m_boundingBoxAtIdentity, totalTransform, tl, bl, br, tr);
	if (!AreBoundingBoxesPossiblyOverlapping(currBoundingBox, g_spriteRenderer->GetBoundingBox()))
	{
		return;
	}
	if (g_spriteRenderer->GetMaterial() != m_materialOverride)
	{
		m_meshRenderer->SetMaterial(m_materialOverride);
	}

	m_meshRenderer->SetUniformTexture("gDiffuseTex", m_resource->GetTexture());

	Vertex2_PCT vertices[4];
	Vertex2_PCT vertex;
	vertex.m_color = WHITE;
	vertex.m_texCoords = Vector2(0.f, 0.f);
	vertex.m_position = tl;
	vertices[0] = vertex;
	vertex.m_texCoords = Vector2(0.f, 1.f);
	vertex.m_position = bl;
	vertices[1] = vertex;
	vertex.m_texCoords = Vector2(1.f, 1.f);
	vertex.m_position = br;
	vertices[2] = vertex;
	vertex.m_texCoords = Vector2(1.f, 0.f);
	vertex.m_position = tr;
	vertices[3] = vertex;
	m_mesh->InitializeVBO(&vertices[0], 4, sizeof(Vertex2_PCT), R_STATIC_DRAW, true);
	m_meshRenderer->SetMesh(m_mesh);

	m_meshRenderer->Render();
}


//-----------------------------------------------------------------------------------------------
void Sprite::SetTargetLayer(ERenderLayer layer)
{
	m_targetLayer = layer;
	if (m_isEnabled)
	{
		g_spriteRenderer->RemoveFromLayers(this);
		g_spriteRenderer->AddToLayer(this, m_targetLayer);
	}
}


//-----------------------------------------------------------------------------------------------
void SpriteResource::LoadResourcesFromXML(const struct XMLNode& node)
{
	int childCount = node.nChildNode();
	for (int i = 0; i < childCount; i++)
	{
		XMLNode currNode = node.getChildNode(i);
		std::string name = XMLUtils::GetAttribute(currNode, "name").GetRaw();
		std::string filepath = XMLUtils::GetAttribute(currNode, "path").GetRaw();
		Register(name, filepath);
	}
}


//-----------------------------------------------------------------------------------------------
void SpriteResource::SaveResourcesToXML(const std::string& filepath)
{
	XMLNode root = XMLNode::createXMLTopNode("Sprites");
	for (std::pair<const std::string, SpriteResource*>& p : s_spriteDatabase)
	{
		XMLNode currNode = root.addChild("Sprite");
		currNode.addAttribute("name", p.first.c_str());
		currNode.addAttribute("path", p.second->m_filepath.c_str());
	}

	root.writeToFile(filepath.c_str());
}