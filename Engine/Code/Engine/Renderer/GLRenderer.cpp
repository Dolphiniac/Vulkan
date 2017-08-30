#include "Engine/Renderer/GLRenderer.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/OpenGLExtensions.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Core/EngineSystemManager.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Light.hpp"
#include "Engine/Renderer/Framebuffer.hpp"

#include <direct.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <math.h>
#include <stdio.h>
#include <sstream>
#include <tuple>
#ifndef __USING_UWP
#pragma comment (lib, "opengl32")
#pragma comment (lib, "Glu32")
#endif


//-----------------------------------------------------------------------------------------------
Renum R_QUADS = GL_QUADS;
Renum R_POINTS = GL_POINTS;
Renum R_LINES = GL_LINES;
Renum R_LINE_LOOP = GL_LINE_LOOP;
Renum R_STATIC_DRAW = GL_STATIC_DRAW;
Renum R_DYNAMIC_DRAW = GL_DYNAMIC_DRAW;
Renum R_TRIANGLES = GL_TRIANGLES;
Renum R_VERTEX_SHADER = GL_VERTEX_SHADER;
Renum R_FRAGMENT_SHADER = GL_FRAGMENT_SHADER;
Renum R_NEAREST = GL_NEAREST;
Renum R_REPEAT = GL_REPEAT;
Renum R_CW = GL_CW;
Renum R_CCW = GL_CCW;
Renum R_UNIFORM_BUFFER = GL_UNIFORM_BUFFER;
Renum R_STREAM_DRAW = GL_STREAM_DRAW;
Renum R_ARRAY_BUFFER = GL_ARRAY_BUFFER;
Renum R_SHADER_STORAGE_BUFFER = GL_SHADER_STORAGE_BUFFER;
Renum R_DYNAMIC_COPY = GL_DYNAMIC_COPY;


//-----------------------------------------------------------------------------------------------
GLRenderer::GLRenderer()
{
	if (!wglGetCurrentContext())
		return;
	glEnable(GL_BLEND);
	glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
	glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
	glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
	glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
	glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
	glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
	glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
	glGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
	glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
	glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");
	glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
	glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
	glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
	glGetProgramiv = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
	glDetachShader = (PFNGLDETACHSHADERPROC)wglGetProcAddress("glDetachShader");
	glDeleteProgram = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress("glDeleteProgram");
	glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog");
	glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)wglGetProcAddress("glGenVertexArrays");
	glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)wglGetProcAddress("glDeleteVertexArrays");
	glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)wglGetProcAddress("glBindVertexArray");
	glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)wglGetProcAddress("glGetAttribLocation");
	glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glEnableVertexAttribArray");
	glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");
	glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
	glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
	glUniform1fv = (PFNGLUNIFORM1FVPROC)wglGetProcAddress("glUniform1fv");
	glUniform2fv = (PFNGLUNIFORM2FVPROC)wglGetProcAddress("glUniform2fv");
	glUniform3fv = (PFNGLUNIFORM3FVPROC)wglGetProcAddress("glUniform3fv");
	glUniform4fv = (PFNGLUNIFORM4FVPROC)wglGetProcAddress("glUniform4fv");
	glUniform1iv = (PFNGLUNIFORM1IVPROC)wglGetProcAddress("glUniform1iv");
	glUniform2iv = (PFNGLUNIFORM2IVPROC)wglGetProcAddress("glUniform2iv");
	glUniform3iv = (PFNGLUNIFORM3IVPROC)wglGetProcAddress("glUniform3iv");
	glUniform4iv = (PFNGLUNIFORM4IVPROC)wglGetProcAddress("glUniform4iv");
	glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)wglGetProcAddress("glUniformMatrix4fv");
	glGenSamplers = (PFNGLGENSAMPLERSPROC)wglGetProcAddress("glGenSamplers");
	glBindSampler = (PFNGLBINDSAMPLERPROC)wglGetProcAddress("glBindSampler");
	glSamplerParameteri = (PFNGLSAMPLERPARAMETERIPROC)wglGetProcAddress("glSamplerParameteri");
	glDeleteSamplers = (PFNGLDELETESAMPLERSPROC)wglGetProcAddress("glDeleteSamplers");
	glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
	glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)wglGetProcAddress("glGenFramebuffers");
	glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)wglGetProcAddress("glBindFramebuffer");
	glFramebufferTexture = (PFNGLFRAMEBUFFERTEXTUREPROC)wglGetProcAddress("glFramebufferTexture");
	glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)wglGetProcAddress("glCheckFramebufferStatus");
	glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)wglGetProcAddress("glDeleteFramebuffers");
	glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)wglGetProcAddress("glBlitFramebuffer");
	glDrawBuffers = (PFNGLDRAWBUFFERSPROC)wglGetProcAddress("glDrawBuffers");
	glGetUniformBlockIndex = (PFNGLGETUNIFORMBLOCKINDEXPROC)wglGetProcAddress("glGetUniformBlockIndex");
	glBufferSubData = (PFNGLBUFFERSUBDATAPROC)wglGetProcAddress("glBufferSubData");
	glUniformBlockBinding = (PFNGLUNIFORMBLOCKBINDINGPROC)wglGetProcAddress("glUniformBlockBinding");
	glBindBufferRange = (PFNGLBINDBUFFERRANGEPROC)wglGetProcAddress("glBindBufferRange");
	glBindBufferBase = (PFNGLBINDBUFFERBASEPROC)wglGetProcAddress("glBindBufferBase");
	glGetActiveUniformBlockiv = (PFNGLGETACTIVEUNIFORMBLOCKIVPROC)wglGetProcAddress("glGetActiveUniformBlockiv");
	glGetActiveUniformsiv = (PFNGLGETACTIVEUNIFORMSIVPROC)wglGetProcAddress("glGetActiveUniformsiv");
	glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glDisableVertexAttribArray");
	glVertexAttribIPointer = (PFNGLVERTEXATTRIBIPOINTERPROC)wglGetProcAddress("glVertexAttribIPointer");
	glGetProgramResourceIndex = (PFNGLGETPROGRAMRESOURCEINDEXPROC)wglGetProcAddress("glGetProgramResourceIndex");
	glShaderStorageBlockBinding = (PFNGLSHADERSTORAGEBLOCKBINDINGPROC)wglGetProcAddress("glShaderStorageBlockBinding");
	glGetBufferSubData = (PFNGLGETBUFFERSUBDATAPROC)wglGetProcAddress("glGetBufferSubData");
	glMapBuffer = (PFNGLMAPBUFFERPROC)wglGetProcAddress("glMapBuffer");
	glUnmapBuffer = (PFNGLUNMAPBUFFERPROC)wglGetProcAddress("glUnmapBuffer");
}


//-----------------------------------------------------------------------------------------------
GLuint GLRenderer::CreateRenderBuffer(void* data, size_t count, size_t elemSize, GLenum usage /*= GL_STATIC_DRAW*/, GLenum target /*= GL_ARRAY_BUFFER*/)
{
	GLuint buffer;
	glGenBuffers(1, &buffer);

	glBindBuffer(target, buffer);
	glBufferData(target, count * elemSize, data, usage);
	glBindBuffer(target, 0);

	return buffer;
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::LogShaderError(GLuint shaderID, const std::string& filename)
{
	std::string cwd = _getcwd(nullptr, 1024);
	GLint length;
	glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &length);

	std::string buffer;
	buffer.resize(length);

	glGetShaderInfoLog(shaderID, length, &length, &buffer[0]);
	std::stringstream ss;

	std::string msgToken;
	ss << buffer;
	std::getline(ss, msgToken, '(');
	std::getline(ss, msgToken, ')');
	int lineNum = std::stoi(msgToken);
	std::getline(ss, msgToken, ':');
	std::getline(ss, msgToken, ' ');
	std::getline(ss, msgToken);
	std::string errMsg = msgToken;

	const GLubyte* version = glGetString(GL_VERSION);
	const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);

	std::stringstream versionSS;
	versionSS << version;
	std::string versions = "GL Version: " + versionSS.str();
	versionSS.str("");

	versionSS << glslVersion;
	std::string glslVersions = "GLSL Version: " + versionSS.str();

	FatalError((cwd + "/" + filename).c_str(), "shader", lineNum, errMsg + "\n" + versions + "\n" + glslVersions + "\nAll error contents:\n" + buffer);
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::LogProgramError(GLuint programID, const std::string& fragFile)
{
	std::string cwd = _getcwd(nullptr, 1024);

	GLint logLength;
	glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &logLength);

	std::string buffer;
	buffer.resize(logLength + 1);

	glGetProgramInfoLog(programID, logLength, &logLength, &buffer[0]);

	const GLubyte* version = glGetString(GL_VERSION);
	const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);

	std::stringstream ss;
	ss << version;
	std::string versions = "GL Version: " + ss.str();
	ss.str("");

	ss << glslVersion;
	std::string glslVersions = "GLSL Version: " + ss.str();

	FatalError((cwd + "/" + fragFile).c_str(), "linker", 1, buffer + "\n" + versions + "\n" + glslVersions);	//Can't test, for some reason.  Things that should make linker errors don't. :(
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::Clear(const Rgba& clearColor, bool useDepth)
{
	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	glClearDepth(1.f);

	if (useDepth)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	else
	{
		glClear(GL_COLOR_BUFFER_BIT);
	}
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::SetPerspective(float fovDegreesY, float aspect, float nearDist, float farDist)
{
	glLoadIdentity();
	gluPerspective(fovDegreesY, aspect, nearDist, farDist);
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::SetOrtho(const Vector2& bottomLeft, const Vector2& topRight)
{
	glLoadIdentity();
	glOrtho(bottomLeft.x, topRight.x, bottomLeft.y, topRight.y, 0.f, 1.f);
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::TranslateView(const Vector3& translation)
{
	glTranslatef(translation.x, translation.y, translation.z);
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::RotateView(float degrees)
{
	glRotatef(degrees, 0.f, 0.f, 1.f);
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::RotateView(float degrees, float axisX, float axisY, float axisZ)
{
	glRotatef(degrees, axisX, axisY, axisZ);
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::ScaleView(float uniformScale)
{
	glScalef(uniformScale, uniformScale, 0.f);
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::ScaleView3D(float uniformScale)
{
	glScalef(uniformScale, uniformScale, uniformScale);
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::FlipZView()
{
	glScalef(1.f, 1.f, -1.f);
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::PushView()
{
	glPushMatrix();
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::PopView()
{
	glPopMatrix();
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::DrawLine(const Vector3& startPos, const Vector3& endPos, const Rgba& startColor, const Rgba& endColor, float lineThickness /* = 1.f */)
{
	BindTexture(Texture::CreateOrGetTexture("NoTex"));
	glLineWidth(lineThickness);
	Vertex_PCT vertices[2];
	vertices[0].m_position = startPos;
	vertices[0].m_color = startColor;
	vertices[1].m_position = endPos;
	vertices[1].m_color = endColor;
	DrawVertexArray(vertices, 2, R_LINES);
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::DrawQuad(const Vector3& pos0, const Vector3& pos1, const Vector3& pos2, const Vector3& pos3, const Rgba& color)
{
	BindTexture(Texture::CreateOrGetTexture("NoTex"));
	Vertex_PCT vertices[4];
	vertices[0].m_position = pos0;
	vertices[0].m_color = color;
	vertices[1].m_color = color;
	vertices[1].m_position = pos1;
	vertices[2].m_position = pos2;
	vertices[2].m_color = color;
	vertices[3].m_color = color;
	vertices[3].m_position = pos3;
	DrawVertexArray(vertices, 4, R_QUADS);
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::DrawTexturedQuad(const Vector3& pos0, const Vector3& pos1, const Vector3& pos2, const Vector3& pos3, const Texture* tex, const AABB2& texAABB, const Rgba& tint)
{
	BindTexture(tex);
	Vertex_PCT vertices[4];
	vertices[0].m_position = pos0;
	vertices[0].m_color = tint;
	vertices[0].m_texCoords = Vector2(texAABB.mins.x, texAABB.maxs.y);
	vertices[1].m_position = pos1;
	vertices[1].m_color = tint;
	vertices[1].m_texCoords = texAABB.maxs;
	vertices[2].m_position = pos2;
	vertices[2].m_color = tint;
	vertices[2].m_texCoords = Vector2(texAABB.maxs.x, texAABB.mins.y);
	vertices[3].m_position = pos3;
	vertices[3].m_color = tint;
	vertices[3].m_texCoords = texAABB.mins;
	DrawVertexArray(vertices, 4, R_QUADS);
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::DrawAABB(const AABB2& bounds, const Rgba& color, EBlendFunction bf)
{
	SetBlendFunction(bf);
	DrawTexturedQuad(bounds.mins, Vector2(bounds.maxs.x, bounds.mins.y), bounds.maxs, Vector2(bounds.mins.x, bounds.maxs.y), Texture::CreateOrGetTexture("NoTex"), AABB2(), color);
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::DrawTexturedAABB(const AABB2& bounds, const Texture* tex, const AABB2& texAABB, const Rgba& tint, EBlendFunction bf)
{
	SetBlendFunction(bf);
	DrawTexturedQuad(bounds.mins, Vector2(bounds.maxs.x, bounds.mins.y), bounds.maxs, Vector2(bounds.mins.x, bounds.maxs.y), tex, texAABB, tint);
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::DrawPolygon(float radius, float numSides, float degreeOffset, const Rgba& color, float lineThickness)
{
	BindTexture(Texture::CreateOrGetTexture("NoTex"));
	std::vector<Vertex_PCT> vertices;
	const float completeRads = 2.f * pi;
	float iterations = completeRads / numSides;
	float radOffset = degreeOffset * (pi / 180.f);
	Vector2 vertex;
	Vertex_PCT vertex_pct;
	vertex_pct.m_color = color;
	glLineWidth(lineThickness);
	for (float currentRad = 0.f; currentRad < completeRads; currentRad += iterations)
	{
		vertex.SetXY(radius * cos(currentRad + radOffset), radius * sin(currentRad + radOffset));
		vertex_pct.m_position = vertex;
		vertices.push_back(vertex_pct);
	}
	DrawVertexArray(&vertices[0], vertices.size(), R_LINE_LOOP);
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::SetBlendFunction(EBlendFunction bf)
{
	glEnable(GL_BLEND);
	switch (bf)
	{
	case ADDITIVE:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;
	case ALPHABLEND:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case SUBTRACT:
		glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
		break;
	default:
		break;
	}
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::UseBackfaceCulling(bool shouldCull)
{
	if (shouldCull)
	{
		glEnable(GL_CULL_FACE);
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::UseZBuffering(bool shouldBuffer)
{
	if (shouldBuffer)
	{
		glEnable(GL_DEPTH_TEST);
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::DrawText2D(const Vector2& startTopLeft, const std::string& asciiText, float cellHeight, const Rgba& tint /*= WHITE*/, const BitmapFont* font /*= nullptr*/, float* width)
{
	if (!font)
	{
		font = BitmapFont::CreateOrGetFont("arial");
	}
	float scale = cellHeight / font->m_base;

	static const float aspect = .55f;
//	float cellWidth = aspect * cellHeight;

	Vector2 cursor(startTopLeft);
	Texture* fontTex = font->GetTexture();

	char prevGlyph = NULL;

	for (char currentChar : asciiText)
	{
		if (currentChar == '\t')
		{
			for (int i = 0; i < 4; i++)
			{
				cursor.x += font->GetGlyph(' ')->advance * scale;
			}
			prevGlyph = ' ';
			continue;
		}
		Glyph* currGlyph = font->GetGlyph(currentChar);
		if (!currGlyph)
			continue;

		if (prevGlyph)
		{
			cursor.x += font->GetKerningOffset(prevGlyph, currGlyph->tChar) * scale;
		}
		Vector2 tl(cursor.x + (currGlyph->xOffset * scale), cursor.y - (currGlyph->yOffset * scale));
		Vector2 bl(tl.x, tl.y - (currGlyph->height * scale));
		Vector2 tr(tl.x + (currGlyph->width * scale), tl.y);
		Vector2 br(tr.x, bl.y);

		AABB2 toDraw(bl, tr);
		AABB2 texAABB = font->GetTexCoordsForGlyph(currentChar);

		DrawTexturedAABB(toDraw, fontTex, texAABB, tint);

		prevGlyph = currGlyph->tChar;

		cursor.x += currGlyph->advance * scale;
	}
	if (width)
	{
		*width = cursor.x - startTopLeft.x;
	}
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::BindTexture(const Texture* tex)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex->m_openglTextureID);
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::DrawVertexArrayQuads(const std::vector<Vertex_PCT>& vertices)
{
	if (vertices.empty())
		return;

	DrawVertexArray(&vertices[0], vertices.size(), R_QUADS);
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::DrawVertexArrayPoints(const std::vector<Vertex_PCT>& vertices, float pointSize)
{
	if (vertices.empty())
		return;

	glPointSize(pointSize);
	DrawVertexArray(&vertices[0], vertices.size(), R_POINTS);
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::DrawVertexArray(const Vertex_PCT* vertices, int numVerts, Renum drawMode)
{
	if (numVerts == 0)
		return;

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, sizeof(Vertex_PCT), &vertices[0].m_position);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex_PCT), &vertices[0].m_color);
	glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex_PCT), &vertices[0].m_texCoords);

	glDrawArrays(drawMode, 0, numVerts);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}


//-----------------------------------------------------------------------------------------------
int GLRenderer::CreateVBOAndGetID()
{
	GLuint vboID;
	glGenBuffers(1, &vboID);
	return vboID;
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::UpdateVBO(int vboID, const Vertex_PCT* vertices, int numVerts)
{
	glBindBuffer(GL_ARRAY_BUFFER, vboID);
	glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(Vertex_PCT), &vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::DrawVBO(int vboID, int numVerts, Renum drawMode)
{
	if (numVerts == 0)
		return;

	glBindBuffer(GL_ARRAY_BUFFER, vboID);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, sizeof(Vertex_PCT), (const GLvoid*)offsetof(Vertex_PCT, m_position));
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex_PCT), (const GLvoid*)offsetof(Vertex_PCT, m_color));
	glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex_PCT), (const GLvoid*)offsetof(Vertex_PCT, m_texCoords));

	glDrawArrays(drawMode, 0, numVerts);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::DeleteVBO(int vboID)
{
	glDeleteBuffers(1, (GLuint*)(&vboID));
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::UseZBufferWriting(bool shouldWrite)
{
	if (shouldWrite)
		glDepthMask(GL_TRUE);
	else
		glDepthMask(GL_FALSE);
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::DrawArraysThinWrapper(Renum drawShape, size_t count)
{
	glDrawArrays(drawShape, 0, count);
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::DrawElementsThinWrapper(Renum drawShape, size_t count)
{
	glDrawElements(drawShape, count, GL_UNSIGNED_SHORT, (GLvoid*)0);
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::UseProgram(GLuint programID)
{
	glUseProgram(programID);
}


//-----------------------------------------------------------------------------------------------
Framebuffer* GLRenderer::CreateFramebuffer(int colorCount, Texture** colorTargets, Texture* depthStencilTarget)
{
	ASSERT_OR_DIE(colorCount > 0, "Bruh");
	Texture* color0 = colorTargets[0];
	int width = color0->m_texelSize.x;
	int height = color0->m_texelSize.y;

	for (int i = 0; i < colorCount; i++)
	{
		Texture* color = colorTargets[i];
		ASSERT_OR_DIE(color->m_texelSize.x == width, "BRUH");
		ASSERT_OR_DIE(color->m_texelSize.y == height, "BRUUUUUUUUUUUUUUUH");
	}

	if (depthStencilTarget)
	{
		ASSERT_OR_DIE(depthStencilTarget->m_texelSize.x == width, "BRUHBEANS");
		ASSERT_OR_DIE(depthStencilTarget->m_texelSize.y == height, "BRUHDUDE");
	}

	GLuint fboHandle;
	glGenFramebuffers(1, &fboHandle);

	Framebuffer* fbo = new Framebuffer();
	fbo->m_handle = fboHandle;
	fbo->m_pixelHeight = height;
	fbo->m_pixelWidth = width;

	for (int i = 0; i < colorCount; i++)
	{
		fbo->m_colorTargets.push_back(colorTargets[i]);
	}

	fbo->m_depthStencilTarget = depthStencilTarget;

	glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);

	for (int i = 0; i < colorCount; i++)
	{
		Texture* tex = colorTargets[i];
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, tex->m_openglTextureID, 0);
	}

	if (depthStencilTarget)
	{
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, depthStencilTarget->m_openglTextureID, 0);
	}

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		DeleteFramebuffer(fbo);
		return nullptr;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, NULL);

	return fbo;
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::DeleteFramebuffer(Framebuffer* fbo)
{
	glDeleteFramebuffers(1, &(fbo->m_handle));
	delete fbo;
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::BindFramebuffer(Framebuffer* fbo)
{
	if (!fbo)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, NULL);
		glViewport(0, 0, 1600, 900);
	}
	else
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo->m_handle);
		glViewport(0, 0, fbo->m_pixelWidth, fbo->m_pixelHeight);

		GLenum renderTargets[32];
		for (size_t i = 0; i < fbo->m_colorTargets.size(); i++)
		{
			renderTargets[i] = GL_COLOR_ATTACHMENT0 + i;
		}
		glDrawBuffers(fbo->m_colorTargets.size(), renderTargets);
	}
}


//-----------------------------------------------------------------------------------------------
void GLRenderer::CopyFramebufferToBackBuffer(Framebuffer* fbo)
{
	if (!fbo)
	{
		return;
	}

	GLuint handle = fbo->m_handle;
	glBindFramebuffer(GL_READ_FRAMEBUFFER, handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, NULL);

	int readWidth = fbo->m_pixelWidth;
	int readHeight = fbo->m_pixelHeight;

	int drawWidth = 1600;
	int drawHeight = 900;

	glBlitFramebuffer(0, 0, readWidth, readHeight,
		0, 0, drawWidth, drawHeight,
		GL_COLOR_BUFFER_BIT, GL_NEAREST);
}