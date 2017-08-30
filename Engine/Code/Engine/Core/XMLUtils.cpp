#include "Engine/Core/XMLUtils.hpp"
#include "ThirdParty/XML/xml.hpp"

#ifndef __USING_UWP
//-----------------------------------------------------------------------------------------------
XMLNode XMLUtils::GetRootNode(const QuString& filepath)
{
	return XMLNode::parseFile(filepath.GetRaw()).getChildNode();
}


//-----------------------------------------------------------------------------------------------
QuString XMLUtils::GetAttribute(const struct XMLNode& node, const QuString& attributeName)
{
	const char* cResult = node.getAttribute(attributeName.GetRaw());
	if (!cResult)
	{
		return "";
	}
	else
	{
		return cResult;
	}
}


//-----------------------------------------------------------------------------------------------
QuString XMLUtils::GetName(const struct XMLNode& node)
{
	return node.getName();
}

#endif