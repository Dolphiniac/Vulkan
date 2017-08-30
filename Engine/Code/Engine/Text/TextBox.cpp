#include "Engine/Text/TextBox.hpp"
#include "ThirdParty\XML\xml.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Model/MeshBuilder.hpp"
#include "Engine/Renderer/MeshRenderer.hpp"
#include "Engine/Renderer/OpenGLExtensions.hpp"
#include "Engine/Renderer/ShaderStorageBlock.hpp"

#include <deque>


//-----------------------------------------------------------------------------------------------
TextBox::TextBox(float width, float height, float scale, class BitmapFont* baseFont, HorizontalAlignment hAlign, EVerticalAlignment vAlign)
	: m_baseFont(baseFont)
	, m_width(width)
	, m_height(height)
	, m_scale(scale)
	, m_totalTimeSinceReset(0.f)
	, m_hAlign(hAlign)
	, m_vAlign(vAlign)
	, m_ssboName("TextIndex")
{
}


//-----------------------------------------------------------------------------------------------
void TextBox::EvaluateLine(std::deque<StringEffectFragment>& currLine, std::deque<StringEffectFragment>& fragmentQueue)
{
	std::string fullLineString = "";
	std::deque<StringEffectFragment> workingLine;
	if (currLine.size() == 1)
	{
		return;
	}
	while (!currLine.empty())
	{
		workingLine.push_back(currLine.front());
		currLine.pop_front();
		std::string currString = workingLine.back().m_value;
		fullLineString += currString;
		float width = m_baseFont->CalculateTextWidth(fullLineString, m_scale);
		if (width > m_width)
		{
			for (size_t i = 0; i < currString.size(); i++)
			{
				fullLineString.pop_back();
			}
			if (currString.find(" ") == std::string::npos)
			{
				currLine.push_front(workingLine.back());
				TrimBeginning(currLine[0].m_value);
				workingLine.pop_back();
				currLine.push_front(StringEffectFragment("\n"));
				size_t workingLineSize = workingLine.size();
				for (size_t i = 0; i < workingLineSize; i++)
				{
					if (i == 0)
					{
						TrimEnd(workingLine.back().m_value);
					}
					currLine.push_front(workingLine.back());
					workingLine.pop_back();
				}
			}
			else
			{
				StringEffectFragment badFrag = workingLine.back();
				workingLine.pop_back();
				std::string unprocessedText = badFrag.m_value;
				std::string processedText = "";
				for (;;)
				{
					if (unprocessedText.empty())
					{
						break;
					}
					char testChar = unprocessedText[0];
					processedText.push_back(testChar);
					unprocessedText = unprocessedText.substr(1);
					if (testChar == ' ')
					{
						std::string testString = fullLineString + processedText;
						if (m_baseFont->CalculateTextWidth(testString, m_scale) > m_width)
						{
							for (char rewindChar = processedText.back(); !processedText.empty() && rewindChar != ' '; rewindChar = processedText.back())
							{
								processedText.pop_back();
								unprocessedText = rewindChar + unprocessedText;
							}
							TrimBeginning(unprocessedText);
							TrimEnd(processedText);
							if (unprocessedText != "")
							{
								StringEffectFragment unprocessedFrag(unprocessedText);
								unprocessedFrag.m_effect = badFrag.m_effect;
								currLine.push_front(unprocessedFrag);
							}
							currLine.push_front(StringEffectFragment("\n"));
							if (processedText != "")
							{
								StringEffectFragment processedFrag(processedText);
								processedFrag.m_effect = badFrag.m_effect;
								currLine.push_front(processedFrag);
							}
							size_t workingLineSize = workingLine.size();
							for (size_t i = 0; i < workingLineSize; i++)
							{
								currLine.push_front(workingLine.back());
								workingLine.pop_back();
							}
							break;
						}
					}
				}
			}
			break;
		}
	}
	if (currLine.empty())
	{
		std::swap(currLine, workingLine);
	}
	else
	{
		for (size_t i = 0; i < currLine.size(); i++)
		{
			fragmentQueue.push_front(currLine.back());
			currLine.pop_back();
		}
	}
}


//-----------------------------------------------------------------------------------------------
static void SetEffectProperties(MeshRenderer* mr, const StringEffectFragment& frag)
{
	mr->SetUniformFloat("gWave", frag.m_effect.wave);
	mr->SetUniformInt("gShake", (int)frag.m_effect.shake);
	mr->SetUniformFloat("gDilate", frag.m_effect.dilate);
	mr->SetUniformInt("gPop", (int)frag.m_effect.pop);
	mr->SetUniformVec4("gColor1", ToVec4(frag.m_effect.color1));
	mr->SetUniformVec4("gColor2", ToVec4(frag.m_effect.color2));
	mr->SetUniformInt("gRainbow", (int)frag.m_effect.rainbow);
}

//-----------------------------------------------------------------------------------------------
void TextBox::ConstructMeshes()
{
	for (MeshRenderer* renderer : m_meshRenderers)
	{
		SAFE_DELETE(renderer);
	}
	m_meshRenderers.clear();
	float totalStringWidth = 0.f;
	int currIndex = 0;
	std::vector<float>lineWidths;
	float currLineWidth = 0.f;
	for (StringEffectFragment frag : m_fragments)
	{
		if (frag.m_value == "\n")
		{
			lineWidths.push_back(currLineWidth);
			currLineWidth = 0.f;
			continue;
		}

		totalStringWidth += m_baseFont->CalculateTextWidth(frag.m_value, m_scale);
		currLineWidth += m_baseFont->CalculateTextWidth(frag.m_value, m_scale);
	}
	lineWidths.push_back(currLineWidth);
	float totalWidthUpToNow = 0.f;
	int lineNum = 0;
	for (StringEffectFragment frag : m_fragments)
	{
		if (frag.m_value == "\n")
		{
			lineNum++;
			totalWidthUpToNow = 0.f;
			continue;
		}
		MeshBuilder mb("Unused");
		mb.AddStringEffectFragment(frag, m_baseFont, m_scale, totalStringWidth, totalWidthUpToNow, m_width, m_height, lineNum, lineWidths[lineNum], m_hAlign, m_vAlign, currIndex);
		Mesh* mesh = mb.ConstructMesh(R_VERTEX_TEXT);// mesh, &Vertex_TextPCT::Copy, sizeof(Vertex_TextPCT), &Vertex_TextPCT::BindMeshToVAO);
		Material* mat = Material::CreateOrGetMaterial("funkyFont");// new Material("funkyFont");
		mat->BindUniformBlock("GlobalMatrices");
		mat->BindShaderStorageBlock(m_ssboName);
		MeshRenderer* meshRenderer = new MeshRenderer(mesh, mat, true);
		meshRenderer->SetUniformTexture("gDiffuseTex", m_baseFont->GetTexture());
		SetEffectProperties(meshRenderer, frag);
		//mat->SetUniformTexture("gDiffuseTex", m_baseFont->GetTexture());
		m_meshRenderers.push_back(meshRenderer);
		totalWidthUpToNow += m_baseFont->CalculateTextWidth(frag.m_value, m_scale);
	}

	ResetAnimation();
}

//-----------------------------------------------------------------------------------------------
void TextBox::ResetAnimation()
{
	m_totalTimeSinceReset = 0.f;
	m_timeOnCurrentIndex = 0.f;
	m_currentRenderedIndex = 0;
}
#ifndef __USING_UWP
//-----------------------------------------------------------------------------------------------
void TextBox::SetFromXMLNode(const struct XMLNode& node)
{
	m_fragments = StringEffectFragment::GetStringFragmentsFromXML(node);

	std::deque<StringEffectFragment> fragmentQueue;

	for (StringEffectFragment& frag : m_fragments)
	{
		fragmentQueue.push_back(frag);
	}

	m_fragments.clear();

	while (!fragmentQueue.empty())
	{
		std::deque<StringEffectFragment> currLine;
		for (StringEffectFragment frag : fragmentQueue)
		{
			currLine.push_back(frag);
			if (frag.m_value == "\n")
			{
				break;
			}
		}
		for (size_t i = 0; i < currLine.size(); i++)
		{
			fragmentQueue.pop_front();
		}
		EvaluateLine(currLine, fragmentQueue);
		while (!currLine.empty())
		{
			m_fragments.push_back(currLine.front());
			currLine.pop_front();
		}
	}

	for (auto iter = m_fragments.begin(); iter != m_fragments.end();)
	{
		if (iter->m_value == "")
		{
			iter = m_fragments.erase(iter);
		}
		else
		{
			iter++;
		}
	}

	ConstructMeshes();
}
#endif
//-----------------------------------------------------------------------------------------------
void TextBox::Update(float deltaSeconds)
{
	m_totalTimeSinceReset += deltaSeconds;
	m_timeOnCurrentIndex += deltaSeconds;
	int renderIndexFromShader = ShaderStorageBlock::GetInt(m_ssboName, "gTextIndex");
	if (renderIndexFromShader != m_currentRenderedIndex)
	{
		m_currentRenderedIndex = renderIndexFromShader;
		m_timeOnCurrentIndex = 0.f;
	}
	SetUniformFloat("gTime", m_totalTimeSinceReset);
	SetUniformFloat("gCurrentIndexTime", m_timeOnCurrentIndex);
	ShaderStorageBlock::SetInt(m_ssboName, "gTextIndex", m_currentRenderedIndex);
}