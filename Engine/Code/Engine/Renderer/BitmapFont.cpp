#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineSystemManager.hpp"
#include "Engine/Core/FileUtils.hpp"

#define STATIC

#include <sstream>
#include <string>


//-----------------------------------------------------------------------------------------------
STATIC std::map<std::string, BitmapFont*> BitmapFont::s_fontRegistry;


//-----------------------------------------------------------------------------------------------
BitmapFont::BitmapFont(const std::string& imageFilePath)
	: m_spriteSheet(imageFilePath, 16, 16)
{
}


//-----------------------------------------------------------------------------------------------
STATIC BitmapFont* BitmapFont::CreateOrGetFont(const std::string& bitmapFontName)
{
	BitmapFont* result = BitmapFont::GetFontByName(bitmapFontName);

	if (!result)
	{
		std::string imageFilePath = Stringf("Data/Fonts/%s.png", bitmapFontName.c_str());
		std::string metaFilePath = Stringf("Data/Fonts/%s.fnt", bitmapFontName.c_str());
		GUARANTEE_OR_DIE(DoesFileExist(imageFilePath), Stringf("Texture %s undefined\n", imageFilePath.c_str()));
		result = new BitmapFont(imageFilePath);
		if (DoesFileExist(metaFilePath))
		{
			FILE* metaFile;
			fopen_s(&metaFile, metaFilePath.c_str(), "r");
			std::ifstream prospectiveMetaFile(metaFile);
			result->PopulateGlyphMapAndSetupKerningPairs(prospectiveMetaFile);
			result->m_isMonospace = false;
			fclose(metaFile);
		}
		else
		{
			result->m_isMonospace = true;
		}
		s_fontRegistry[bitmapFontName] = result;
	}

	return result;
}


//-----------------------------------------------------------------------------------------------
STATIC BitmapFont* BitmapFont::GetFontByName(const std::string& bitmapFontName)
{
	auto fontIter = s_fontRegistry.find(bitmapFontName);
	if (fontIter == s_fontRegistry.end())
	{
		return nullptr;
	}
	return fontIter->second;
}


//-----------------------------------------------------------------------------------------------
AABB2 BitmapFont::GetTexCoordsForGlyph(int glyphUnicode) const
{
	if (m_isMonospace)
		return m_spriteSheet.GetTexCoordsForSpriteIndex(glyphUnicode);

	Glyph* currGlyph = GetGlyph((unsigned char)glyphUnicode);
	Vector2 mins(currGlyph->x * m_texScale, currGlyph->y * m_texScale);
	Vector2 maxs(mins.x + (currGlyph->width * m_texScale), mins.y + (currGlyph->height * m_texScale));
	return AABB2(mins, maxs);
}


//-----------------------------------------------------------------------------------------------
Glyph* BitmapFont::GetGlyph(unsigned char tChar) const
{
	auto glyphIter = m_glyphs.find(tChar);
	if (glyphIter == m_glyphs.end())
		return nullptr;

	return glyphIter->second;
}


//-----------------------------------------------------------------------------------------------
void BitmapFont::PopulateGlyphMapAndSetupKerningPairs(std::ifstream& meta)
{
	std::string currLine;
	std::stringstream currLineStream;
	while (std::getline(meta, currLine))
	{
		currLineStream << currLine;
		std::string currTok;
		while (std::getline(currLineStream, currTok, ' '))
		{
			if (currTok == "common")
			{
				std::getline(currLineStream, currTok, '=');
				std::getline(currLineStream, currTok, ' ');
				m_lineHeight = std::stof(currTok);

				std::getline(currLineStream, currTok, '=');
				std::getline(currLineStream, currTok, ' ');
				m_base = std::stof(currTok);

				std::getline(currLineStream, currTok, '=');
				std::getline(currLineStream, currTok, ' ');
				m_texScale = 1.f / std::stof(currTok);
			}
			else if (currTok == "char")
			{
				Glyph* nextGlyph = new Glyph();
				std::getline(currLineStream, currTok, '=');
				std::getline(currLineStream, currTok, ' ');
				nextGlyph->tChar = (unsigned char)std::stoi(currTok);

				std::getline(currLineStream, currTok, '=');
				std::getline(currLineStream, currTok, ' ');
				nextGlyph->x = std::stof(currTok);

				std::getline(currLineStream, currTok, '=');
				std::getline(currLineStream, currTok, ' ');
				nextGlyph->y = std::stof(currTok);

				std::getline(currLineStream, currTok, '=');
				std::getline(currLineStream, currTok, ' ');
				nextGlyph->width = std::stof(currTok);

				std::getline(currLineStream, currTok, '=');
				std::getline(currLineStream, currTok, ' ');
				nextGlyph->height = std::stof(currTok);

				std::getline(currLineStream, currTok, '=');
				std::getline(currLineStream, currTok, ' ');
				nextGlyph->xOffset = std::stof(currTok);

				std::getline(currLineStream, currTok, '=');
				std::getline(currLineStream, currTok, ' ');
				nextGlyph->yOffset = std::stof(currTok);

				std::getline(currLineStream, currTok, '=');
				std::getline(currLineStream, currTok, ' ');
				nextGlyph->advance = std::stof(currTok);

				m_glyphs[nextGlyph->tChar] = nextGlyph;
			}
			else if (currTok == "kerning")
			{
				unsigned char tFirstChar;
				unsigned char tSecondChar;
				float kerningAmount;

				std::getline(currLineStream, currTok, '=');
				std::getline(currLineStream, currTok, ' ');
				tFirstChar = (unsigned char)std::stoi(currTok);

				std::getline(currLineStream, currTok, '=');
				std::getline(currLineStream, currTok, ' ');
				tSecondChar = (unsigned char)std::stoi(currTok);

				std::getline(currLineStream, currTok, '=');
				std::getline(currLineStream, currTok, ' ');
				kerningAmount = std::stof(currTok);

				m_kerningPairs.push_back(std::make_tuple(tFirstChar, tSecondChar, kerningAmount));
			}
		}
		currLineStream.clear();
	}
}


//-----------------------------------------------------------------------------------------------
float BitmapFont::GetKerningOffset(unsigned char first, unsigned char second) const
{
	for (std::tuple<unsigned char, unsigned char, float> t : m_kerningPairs)
	{
		if (std::get<0>(t) == first && std::get<1>(t) == second)
		{
			return std::get<2>(t);
		}
	}
	
	return 0.f;
}


// -----------------------------------------------------------------------------------------------
// float BitmapFont::CalculateTextWidth(const BitmapFont* font, const std::string& asciiText, float cellHeight)
// {
// 	Rgba transparent(0.f, 0.f, 0.f, 0.f);
// 
// 	float result;
// 
// 	The.Renderer->DrawText2D(Vector2::Zero, asciiText, cellHeight, transparent, font, &result);
// 
// 	return result;
// }


//-----------------------------------------------------------------------------------------------
float BitmapFont::CalculateTextWidth(const std::string& asciiText, float scale) const
{
	float result = 0.f;
	Glyph* prevGlyph = nullptr;
	for (char c : asciiText)
	{
		Glyph* currGlyph = GetGlyph(c);
		if (!currGlyph)
		{
			prevGlyph = currGlyph;
			continue;
		}
		if (prevGlyph)
		{
			result += GetKerningOffset(prevGlyph->tChar, currGlyph->tChar) * scale;
		}

		result += currGlyph->advance * scale;

		prevGlyph = currGlyph;
	}

	return result;
}


//-----------------------------------------------------------------------------------------------
void BitmapFont::DestroyFonts()
{
	for (std::pair<const std::string, BitmapFont*>& font : s_fontRegistry)
	{
		delete font.second;
	}
	s_fontRegistry.clear();
}


//-----------------------------------------------------------------------------------------------
BitmapFont::~BitmapFont()
{
	for (std::pair<const unsigned char, Glyph*>& glyph : m_glyphs)
	{
		delete glyph.second;
	}
}