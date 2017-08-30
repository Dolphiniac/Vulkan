#pragma once

#include <vector>


//-----------------------------------------------------------------------------------------------
typedef unsigned int GLuint;


//-----------------------------------------------------------------------------------------------
class Framebuffer
{
public:
	GLuint m_handle;
	std::vector<class Texture*> m_colorTargets;
	Texture* m_depthStencilTarget;

	unsigned int m_pixelWidth;
	unsigned int m_pixelHeight;
};