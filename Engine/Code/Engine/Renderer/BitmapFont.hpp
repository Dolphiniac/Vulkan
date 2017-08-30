#pragma once

#include "Engine/Renderer/SpriteSheet.hpp"

#include <fstream>
#include <string>
#include <vector>
#include <tuple>
#include <map>


//-----------------------------------------------------------------------------------------------
struct Glyph
{
	unsigned char tChar;
	float x;
	float y;
	float width;
	float height;
	float xOffset;
	float yOffset;
	float advance;
};


//-----------------------------------------------------------------------------------------------
class BitmapFont
{
public:
	static BitmapFont* CreateOrGetFont(const std::string& bitmapFontName);
	static BitmapFont* GetFontByName(const std::string& bitmapFontName);
	AABB2 GetTexCoordsForGlyph(int glyphUnicode) const;
	Texture* GetTexture() const { return m_spriteSheet.GetTexture(); }
	Glyph* GetGlyph(unsigned char tChar) const;
	float GetKerningOffset(unsigned char first, unsigned char second) const;
	float CalculateTextWidth(const std::string& asciiText, float scale) const;
	static void DestroyFonts();
	~BitmapFont();
//	static float CalculateTextWidth(const BitmapFont* font, const std::string& asciiText, float cellHeight);

private:
	BitmapFont(const std::string& imageFilePath);

private:
	void PopulateGlyphMapAndSetupKerningPairs(std::ifstream& meta);
	static std::map<std::string, BitmapFont*> s_fontRegistry;
	SpriteSheet m_spriteSheet;
	std::map<unsigned char, Glyph*> m_glyphs;
	std::vector<std::tuple<unsigned char, unsigned char, float>> m_kerningPairs;
	bool m_isMonospace;
	float m_texScale;

public:
	float m_base;
	float m_lineHeight;
};