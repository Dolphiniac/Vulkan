#pragma once

#include <string>
#include <vector>
#include "Engine/Text/TextEffect.hpp"


//-----------------------------------------------------------------------------------------------
class StringEffectFragment
{
public:
	StringEffectFragment(const std::string& value);
#ifndef __USING_UWP
	static std::vector<StringEffectFragment> GetStringFragmentsFromXML(const struct XMLNode& node);

private:
	static TextEffect GetTextEffect(const struct XMLNode& node);
#endif
public:
	std::string m_value;
	TextEffect m_effect;
};