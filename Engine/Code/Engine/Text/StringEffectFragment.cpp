#include "Engine/Text/StringEffectFragment.hpp"
#include "Engine/Core/XMLUtils.hpp"
#include "ThirdParty/XML/xml.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


//-----------------------------------------------------------------------------------------------
StringEffectFragment::StringEffectFragment(const std::string& value)
	: m_value(value)
	, m_effect()
{

}

//-----------------------------------------------------------------------------------------------
static void ExtractLineReturns(std::vector<StringEffectFragment>& fragments)
{
	std::vector<StringEffectFragment> workingList;
	std::swap(workingList, fragments);
	for (StringEffectFragment frag : workingList)
	{
		std::vector<std::string> splitString = SplitOnDelimiters(frag.m_value, 2, "\r", "\t");
		if (splitString.size() > 1)
		{
			for (size_t i = 0; i < splitString.size(); i++)
			{
				if (splitString[i].find("\n") != std::string::npos)
				{
					if (i != 0)
					{
						TrimEnd(splitString[i - 1]);
					}
					if (i != splitString.size() - 1)
					{
						TrimBeginning(splitString[i + 1]);
					}
					Trim(splitString[i]);

				}
			}
			for (const std::string& s : splitString)
			{
				if (s == "")
				{
					continue;
				}
				StringEffectFragment tempFrag(s);
				tempFrag.m_effect = frag.m_effect;
				fragments.push_back(tempFrag);
			}
		}
		else
		{
			fragments.push_back(frag);
		}
	}
}
#ifndef __USING_UWP
//-----------------------------------------------------------------------------------------------
std::vector<StringEffectFragment> StringEffectFragment::GetStringFragmentsFromXML(const struct XMLNode& node)
{
	std::string inString = XMLUtils::GetAttribute(node, "value").GetRaw();
	std::vector<std::string> fragmentValues = SplitOnDelimiters(inString, 2, "[[", "]]");
	int effectNodeCount = node.nChildNode();
	int effectNodeCountExpected = fragmentValues.size() / 2;

	GUARANTEE_OR_DIE((fragmentValues.size() & 1) == 1,
		Stringf("Input string:\n%s\ncannot be parsed.  Make sure effect text is not nested.  Close all effect text blurbs", inString.c_str()));
	GUARANTEE_OR_DIE(effectNodeCount == effectNodeCountExpected,
		Stringf("Affected fragment and effect node count mismatch.  The input string:\n%s\nexpects %i effect nodes", inString.c_str(), effectNodeCountExpected));

	std::vector<StringEffectFragment> result;
	int childNodeSearchPointer = 0;
	TextEffect defaultTextEffects = GetTextEffect(node);
	for (size_t i = 0; i < fragmentValues.size(); i++)
	{
		if (fragmentValues.at(i) == "")
		{
			if ((i & 1) == 1)
			{
				//This is oddly indexed, which indicates that it is an effect fragment
				//In this case, we need to advance the pointer so the effect is skipped
				node.getChildNode("Effect", &childNodeSearchPointer);
			}
			continue;
		}
		StringEffectFragment currFragment(fragmentValues.at(i));
		if ((i & 1) == 0)
		{
			//This is evenly indexed, which indicates that it lies outside of effect specifiers
			currFragment.m_effect = defaultTextEffects;
		}
		else
		{
			currFragment.m_effect = GetTextEffect(node.getChildNode("Effect", &childNodeSearchPointer));
		}

		result.push_back(currFragment);
	}

	ExtractLineReturns(result);

	return result;
}


//Long and boring effect extractor---------------------------------------------------------------
TextEffect StringEffectFragment::GetTextEffect(const struct XMLNode& node)
{
	TextEffect result;
	std::string wave = XMLUtils::GetAttribute(node, "wave").GetRaw();
	if (wave != "")
	{
		try
		{
			result.wave = std::stof(wave);
		}
		catch (const std::exception&)
		{
			ERROR_AND_DIE(Stringf("wave effect value %s is invalid.  Expected a float", wave.c_str()));
		}
	}

	std::string shake = XMLUtils::GetAttribute(node, "shake").GetRaw();
	if (shake != "")
	{
		if (shake == "true")
		{
			result.shake = true;
		}
		else if (shake == "false");
		else
		{
			ERROR_AND_DIE(Stringf("shake effect value %s is invalid.  Expected \"true\" or \"false\"", shake.c_str()));
		}
	}

	std::string dilate = XMLUtils::GetAttribute(node, "dilate").GetRaw();
	if (dilate != "")
	{
		try
		{
			result.dilate = std::stof(dilate);
		}
		catch (const std::exception&)
		{
			ERROR_AND_DIE(Stringf("dilate effect value %s is invalid.  Expected a float", wave.c_str()));
		}
	}

	std::string pop = XMLUtils::GetAttribute(node, "pop").GetRaw();
	if (pop != "")
	{
		if (pop == "true")
		{
			result.pop = true;
		}
		else if (pop == "false");
		else
		{
			ERROR_AND_DIE(Stringf("pop effect value %s is invalid.  Expected \"true\" or \"false\"", shake.c_str()));
		}
	}

	std::string color = XMLUtils::GetAttribute(node, "color").GetRaw();
	if (color != "")
	{
		Rgba universalColor = GetColorFromHexString(color);
		result.color1 = result.color2 = universalColor;
	}

	std::string color1 = XMLUtils::GetAttribute(node, "color1").GetRaw();
	if (color1 != "")
	{
		Rgba rgba1 = GetColorFromHexString(color1);
		result.color1 = rgba1;
	}

	std::string color2 = XMLUtils::GetAttribute(node, "color2").GetRaw();
	if (color2 != "")
	{
		Rgba rgba2 = GetColorFromHexString(color2);
		result.color2 = rgba2;
	}

	std::string rainbow = XMLUtils::GetAttribute(node, "rainbow").GetRaw();
	if (rainbow != "")
	{
		if (rainbow == "true")
		{
			result.rainbow = true;
		}
		else if (rainbow == "false");
		else
		{
			ERROR_AND_DIE(Stringf("rainbow effect value %s is invalid.  Expected \"true\" or \"false\"", shake.c_str()));
		}
	}

	return result;
}
#endif