#pragma once

#include "Engine/Math/IntVector2.hpp"

#include <map>


//-----------------------------------------------------------------------------------------------
enum ETextureFormat
{
	RGBA8,
	D24S8
};


//-----------------------------------------------------------------------------------------------
class Texture
{
private:
	static std::map<std::string, Texture*> s_textureRegistry;
	Texture(const std::string& imageFilePath);
	Texture(int texID, IntVector2 texelSize);
	Texture(const unsigned char* texelData, const IntVector2& texelSize, int numComponents);
	Texture(int width, int height, ETextureFormat format);

public:
	static Texture* GetTextureByName(const std::string& imageFilePath);
	static Texture* CreateOrGetTexture(const std::string& imageFilePath);
	static Texture* CreateTexture2D(int width, int height, ETextureFormat format);
	static void DestroyTextures();

public:
	int				m_openglTextureID;
	IntVector2		m_texelSize;
};