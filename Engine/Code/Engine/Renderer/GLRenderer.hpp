// First definitions provided by Squirrel Eiserloh
// 
#pragma once

#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/Rgba.hpp"
#include "Engine/Renderer/TheRenderer.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
enum EBlendFunction { ADDITIVE, ALPHABLEND, SUBTRACT };


//-----------------------------------------------------------------------------------------------
class GLRenderer;
class Texture;
class BitmapFont;
class Vector4;
class Matrix44;





//-----------------------------------------------------------------------------------------------
struct Vertex_PCT
{
	Vector3 m_position;
	Rgba m_color;
	Vector2 m_texCoords;

	Vertex_PCT();
	Vertex_PCT(const Vector3& position, const Vector2& texCoords = Vector2::Zero, const Rgba& color = WHITE);
};


//-----------------------------------------------------------------------------------------------
struct Vertex2_PCT
{
	Vector2 m_position;
	Rgba m_color;
	Vector2 m_texCoords;
};


//-----------------------------------------------------------------------------------------------
struct Vertex_PCTTB
{
	Vector3 m_position;
	Rgba m_color;
	Vector2 m_texCoords;
	Vector3 m_tangent;
	Vector3 m_bitangent;

	Vertex_PCTTB() : m_position(Vector3::Zero), m_color(WHITE), m_texCoords(Vector2::Zero), m_tangent(Vector3::Zero), m_bitangent(Vector3::Zero) {}
	Vertex_PCTTB(const Vector3& position, const Vector2& texCoords = Vector2::Zero, const Rgba& color = WHITE, const Vector3& tangent = Vector3::Zero, const Vector3& bitangent = Vector3::Zero)
		: m_position(position), m_texCoords(texCoords), m_color(color), m_tangent(tangent), m_bitangent(bitangent) {}
};


//-----------------------------------------------------------------------------------------------
struct Vertex_PCTN
{
	Vector3 m_position;
	Rgba m_color;
	Vector2 m_texCoords;
	Vector3 m_normal;

	Vertex_PCTN() : m_position(Vector3::Zero), m_color(WHITE), m_texCoords(Vector2::Zero), m_normal(Vector3::Zero) {}
	Vertex_PCTN(const Vector3& position, const Vector2& texCoords = Vector2::Zero, const Rgba& color = WHITE, const Vector3& normal = Vector3::Zero)
		: m_position(position), m_texCoords(texCoords), m_color(color), m_normal(normal) {}
};


//-----------------------------------------------------------------------------------------------
struct Vertex_PCTTBB
{
	Vector3 m_position;
	Rgba m_color;
	Vector2 m_texCoords;
	Vector3 m_tangent;
	Vector3 m_bitangent;
	Vector4 m_boneWeights;
	IntVector4 m_boneIndices;
};


//-----------------------------------------------------------------------------------------------
struct Vertex_Text
{
	Vector3 m_position;
	Rgba m_color;
	Vector2 m_texCoords;
	Vector2 m_normalizedGlyphPosition;
	Vector2 m_normalizedStringPosition;
	float m_normalizedFragPosition;
	int m_glyphIndex;
};


//-----------------------------------------------------------------------------------------------
struct RenderState
{
	bool depthWrite = true;
	bool depthTest = true;
	EBlendFunction bf = ALPHABLEND;
	bool backFaceCulling = true;
	Renum windingOrder = R_CCW;
};


//-----------------------------------------------------------------------------------------------
class GLRenderer : public TheRenderer
{
	friend class EngineSystemManager;
public:
	GLRenderer();
	void Clear(const Rgba& clearColor, bool useDepth = false);
	void SetOrtho(const Vector2& bottomLeft, const Vector2& topRight); // calls glLoadIdentity followed by glOrtho, with the 5th and 6th arguments of glOrtho being trivial (from sample code)
	void SetPerspective(float fovDegreesY, float aspect, float nearDist, float farDist);
	void TranslateView(const Vector3& translation);
	void RotateView(float degrees); // calls glRotatef( degrees, 0.f, 0.f, 1.f ); since in 2D we're always rotating "around the +Z axis"
	void RotateView(float degrees, float axisX, float axisY, float axisZ);
	void ScaleView(float uniformScale); // calls glScalef( uniformScale, uniformScale, 1.f );
	void ScaleView3D(float uniformScale);
	void FlipZView();
	void PushView();
	void PopView();
	void DrawLine(const Vector3& startPos, const Vector3& endPos, const Rgba& startColor, const Rgba& endColor, float lineThickness = 2.5f);
	void DrawQuad(const Vector3& pos0, const Vector3& pos1, const Vector3& pos2, const Vector3& pos3, const Rgba& color);
	void DrawTexturedQuad(const Vector3& pos0, const Vector3& pos1, const Vector3& pos2, const Vector3& pos3, const Texture* tex, const AABB2& texAABB = AABB2(Vector2(0.f, 0.f), Vector2(1.f, 1.f)), const Rgba& tint = WHITE);
	void DrawAABB(const AABB2& bounds, const Rgba& color, EBlendFunction bf = ALPHABLEND);
	void DrawTexturedAABB(const AABB2& physBounds, const Texture* tex, const AABB2& texAABB = AABB2(0.f, 0.f, 1.f, 1.f), const Rgba& tint = WHITE, EBlendFunction bf = ALPHABLEND);
	void DrawPolygon(float radius, float numSides, float degreeOffset, const Rgba& color, float lineThickness = 2.5f);
	void SetBlendFunction(EBlendFunction bf);
	void UseBackfaceCulling(bool shouldCull);
	virtual void UseZBuffering(bool shouldBuffer) override;
	void DrawText2D(const Vector2& startTopLeft, const std::string& asciiText, float cellHeight, const Rgba& tint = WHITE, const BitmapFont* font = nullptr, float* width = nullptr);
	void BindTexture(const Texture* tex);
	void DrawVertexArrayQuads(const std::vector<Vertex_PCT>& vertices);
	void DrawVertexArrayPoints(const std::vector<Vertex_PCT>& vertices, float pointSize);
	void DrawVertexArray(const Vertex_PCT* vertices, int numVerts, Renum drawMode);
	int CreateVBOAndGetID();
	void UpdateVBO(int vboID, const Vertex_PCT* vertices, int numVerts);
	void DrawVBO(int vboID, int numVerts, Renum drawMode);
	void DeleteVBO(int vboID);
	void UseZBufferWriting(bool shouldWrite);
	GLuint CreateRenderBuffer(void* data, size_t count, size_t elemSize, GLenum usage = R_STATIC_DRAW, GLenum target = R_ARRAY_BUFFER);
	void DrawArraysThinWrapper(Renum drawShape, size_t count);
	void DrawElementsThinWrapper(Renum drawShape, size_t count);
	void UseProgram(GLuint programID);
	class Framebuffer* CreateFramebuffer(int colorCount, Texture** colorTargets, Texture* depthStencilTarget);
	void DeleteFramebuffer(Framebuffer* fbo);
	void BindFramebuffer(Framebuffer* fbo);
	void CopyFramebufferToBackBuffer(Framebuffer* fbo);
	void LogProgramError(GLuint programID, const std::string& fragFile);
	void LogShaderError(GLuint shaderID, const std::string& filename);
private:

	Framebuffer* m_fbo;
	Framebuffer* m_fboSecond;
};


//-----------------------------------------------------------------------------------------------
inline Vertex_PCT::Vertex_PCT()
: m_position(Vector3::Zero)
, m_color(WHITE)
, m_texCoords(Vector2::Zero)
{
}


//-----------------------------------------------------------------------------------------------
inline Vertex_PCT::Vertex_PCT(const Vector3& position, const Vector2& texCoords /* = Vector2::Zero() */, const Rgba& color /* = WHITE */)
: m_position(position)
, m_color(color)
, m_texCoords(texCoords)
{
}