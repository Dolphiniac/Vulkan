#pragma once

#include "Engine/Renderer/Rgba.hpp"

//-----------------------------------------------------------------------------------------------
typedef unsigned int GLuint;
typedef unsigned int GLenum;
typedef int GLint;
typedef unsigned char GLboolean;
typedef int GLsizei;
typedef float GLfloat;
typedef GLenum Renum;

extern Renum R_QUADS;
extern Renum R_POINTS;
extern Renum R_LINES;
extern Renum R_LINE_LOOP;
extern Renum R_STATIC_DRAW;
extern Renum R_DYNAMIC_DRAW;
extern Renum R_TRIANGLES;
extern Renum R_VERTEX_SHADER;
extern Renum R_FRAGMENT_SHADER;
extern Renum R_NEAREST;
extern Renum R_REPEAT;
extern Renum R_CW;
extern Renum R_CCW;
extern Renum R_UNIFORM_BUFFER;
extern Renum R_ARRAY_BUFFER;
extern Renum R_STREAM_DRAW;
extern Renum R_SHADER_STORAGE_BUFFER;
extern Renum R_DYNAMIC_COPY;
const Renum R_VERTEX_PCT = 0;
const Renum R_VERTEX_PCTTB = 1;
const Renum R_VERTEX_PCTN = 2;
const Renum R_VERTEX_PCTTBB = 3;
const Renum R_VERTEX_TEXT = 4;
const Renum R_VERTEX2_PCT = 5;


//-----------------------------------------------------------------------------------------------
class TheRenderer
{
public:
	virtual void UseZBuffering(bool shouldUse) = 0;
	//virtual void DrawLine(const Vector3& startPos, const Vector3& endPos, const Rgba& startColor, const Rgba& endColor, float lineThickness = 2.5f) = 0;
	//virtual void DrawQuad(const Vector3& pos0, const Vector3& pos1, const Vector3& pos2, const Vector3& pos3, const Rgba& color) = 0;
};