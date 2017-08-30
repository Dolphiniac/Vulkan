#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Math/AABB2.hpp"
#include "../Core/ErrorWarningAssert.hpp"


//-----------------------------------------------------------------------------------------------
SpriteSheet::SpriteSheet(const std::string& imageFilePath, int tilesWide, int tilesHigh)
: m_spriteLayout(tilesWide, tilesHigh)
, m_spriteSheetTexture(Texture::CreateOrGetTexture(imageFilePath))
, m_texCoordsPerSpriteCoords(Vector2(1.f / (float)tilesWide, 1.f / (float)tilesHigh))
{
}


//-----------------------------------------------------------------------------------------------
AABB2 SpriteSheet::GetTexCoordsForSpriteCoords(int spriteX, int spriteY) const
{
	Vector2 mins((float)spriteX, (float)(spriteY));
	Vector2 maxs((float)(spriteX) + 1.f, (float)(spriteY) + 1.f);

	mins.x *= m_texCoordsPerSpriteCoords.x;
	maxs.x *= m_texCoordsPerSpriteCoords.x;

	mins.y *= m_texCoordsPerSpriteCoords.y;
	maxs.y *= m_texCoordsPerSpriteCoords.y;
	
	return AABB2(mins, maxs);
}


//-----------------------------------------------------------------------------------------------
AABB2 SpriteSheet::GetTexCoordsForSpriteIndex(int spriteIndex) const
{
	if (spriteIndex == 1)
	{
		DebuggerPrintf("Hello\n");
	}
	int x = spriteIndex % m_spriteLayout.x;
	int y = spriteIndex / m_spriteLayout.x;

	return GetTexCoordsForSpriteCoords(x, y);
}


//-----------------------------------------------------------------------------------------------
int SpriteSheet::GetNumSprites() const
{
	return m_spriteLayout.x * m_spriteLayout.y;
}
