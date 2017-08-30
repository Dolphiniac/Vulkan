#pragma once

#include "Engine/Math/Vector3.hpp"
#include "Engine/Text/StringEffectFragment.hpp"
#include "Engine/Actor/Actor.hpp"
#include "Engine/Renderer/BitmapFont.hpp"

#include <deque>

typedef float HorizontalAlignment;

enum EVerticalAlignment
{
	ALIGNMENT_TOP,
	ALIGNMENT_BOTTOM
};

const HorizontalAlignment ALIGNMENT_RIGHT = 1.f;
const HorizontalAlignment ALIGNMENT_CENTER = .5f;
const HorizontalAlignment ALIGNMENT_LEFT = 0.f;


//-----------------------------------------------------------------------------------------------
class TextBox : public Actor
{
public:
	TextBox(float width, float height, float scale, class BitmapFont* baseFont = BitmapFont::CreateOrGetFont("consolas"), HorizontalAlignment hAlign = ALIGNMENT_CENTER, EVerticalAlignment vAlign = ALIGNMENT_TOP);
	void SetFromXMLNode(const struct XMLNode& node);
	void ResetAnimation();
	void Update(float deltaSeconds) override;

private:
	void EvaluateLine(std::deque<StringEffectFragment>& currLine, std::deque<StringEffectFragment>& fragmentQueue);
	void ConstructMeshes();
private:
	std::vector<StringEffectFragment> m_fragments;
	class BitmapFont* m_baseFont;
	float m_width;
	float m_height;
	float m_scale;
	float m_totalTimeSinceReset;
	float m_timeOnCurrentIndex;
	HorizontalAlignment m_hAlign;
	EVerticalAlignment m_vAlign;
	std::string m_ssboName;
	int m_currentRenderedIndex;
};