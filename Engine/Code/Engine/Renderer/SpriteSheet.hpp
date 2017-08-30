#pragma once
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/Vector2.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
class AABB2;
class Texture;


//-----------------------------------------------------------------------------------------------
class SpriteSheet
{
public:
	SpriteSheet(const std::string& imageFilePath, int tilesWide, int tilesHigh);
	AABB2 GetTexCoordsForSpriteCoords(int spriteX, int spriteY) const;
	AABB2 GetTexCoordsForSpriteIndex(int spriteIndex) const;
	int GetNumSprites() const;
	Texture* GetTexture() const { return m_spriteSheetTexture; }

private:
	Texture*		m_spriteSheetTexture;
	IntVector2		m_spriteLayout;
	Vector2			m_texCoordsPerSpriteCoords;
};