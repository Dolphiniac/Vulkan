#pragma once

#include "ThirdParty/XML/xml.hpp"
#include "Quantum/Core/String.h"

#define XML_EXTRACT_ATTRIBUTE(node, attribute) QuString attribute = XMLUtils::GetAttribute(node, #attribute);

#define XML_LAST_CASE_ERROR(errorMsg) else ERROR_AND_DIE(errorMsg)

#define XML_IGNORE_OPTION(attribute, ignoreOption) \
if (attribute == ignoreOption); else

#define XML_SET_VAR(variable, value) \
variable = value

#define XML_SET_VAR_WITH_IGNORE(attribute, variable, value, ignoreOption) \
XML_IGNORE_OPTION(attribute, ignoreOption) \
XML_SET_VAR(variable, value)

#define XML_SET_VAR_ONE_OPTION(attribute, variable, option, value) \
if (attribute == option) XML_SET_VAR(variable, value)

#define XML_SET_VAR_ONE_OPTION_WITH_IGNORE(attribute, variable, option, value, ignoreOption) \
XML_IGNORE_OPTION(attribute, ignoreOption) \
XML_SET_VAR_ONE_OPTION(attribute, variable, option, value)

#define XML_SET_VAR_ONE_OPTION_CONTINUE(attribute, variable, option, value) \
else XML_SET_VAR_ONE_OPTION(attribute, variable, option, value)

#define XML_SET_VAR_ONE_OPTION_WITH_ERROR(attribute, variable, option, value, errorMsg) \
XML_SET_VAR_ONE_OPTION(attribute, variable, option, value); \
XML_LAST_CASE_ERROR(errorMsg)

#define XML_SET_VAR_ONE_OPTION_ERROR_AND_IGNORE(attribute, variable, option, value, errorMsg, ignoreOption) \
XML_SET_VAR_ONE_OPTION_WITH_IGNORE(attribute, variable, option, value, ignoreOption); \
XML_LAST_CASE_ERROR(errorMsg)

#define XML_SET_VAR_TWO_OPTIONS(attribute, variable, option1, value1, option2, value2) \
XML_SET_VAR_ONE_OPTION(attribute, variable, option1, value1); \
XML_SET_VAR_ONE_OPTION_CONTINUE(attribute, variable, option2, value2)

#define XML_SET_VAR_TWO_OPTIONS_WITH_IGNORE(attribute, variable, option1, value1, option2, value2, ignoreOption) \
XML_IGNORE_OPTION(attribute, ignoreOption) \
XML_SET_VAR_TWO_OPTIONS(attribute, variable, option1, value1, option2, value2)

#define XML_SET_VAR_TWO_OPTIONS_WITH_ERROR(attribute, variable, option1, value1, option2, value2, errorMsg) \
XML_SET_VAR_TWO_OPTIONS(attribute, variable, option1, value1, option2, value2); \
XML_LAST_CASE_ERROR(errorMsg)

#define XML_SET_VAR_TWO_OPTIONS_ERROR_AND_IGNORE(attribute, variable, option1, value1, option2, value2, errorMsg, ignoreOption) \
XML_SET_VAR_TWO_OPTIONS_WITH_IGNORE(attribute, variable, option1, value1, option2, value2, ignoreOption); \
XML_LAST_CASE_ERROR(errorMsg)

#define XML_SET_VAR_THREE_OPTIONS(attribute, variable, option1, value1, option2, value2, option3, value3) \
XML_SET_VAR_TWO_OPTIONS(attribute, variable, option1, value1, option2, value2); \
XML_SET_VAR_ONE_OPTION_CONTINUE(attribute, variable, option3, value3)

#define XML_SET_VAR_THREE_OPTIONS_WITH_IGNORE(attribute, variable, option1, value1, option2, value2, option3, value3, ignoreOption) \
XML_IGNORE_OPTION(attribute, ignoreOption) \
XML_SET_VAR_THREE_OPTIONS(attribute, variable, option1, value1, option2, value2, option3, value3)

#define XML_SET_VAR_THREE_OPTIONS_WITH_ERROR(attribute, variable, option1, value1, option2, value2, option3, value3, errorMsg) \
XML_SET_VAR_THREE_OPTIONS(attribute, variable, option1, value1, option2, value2, option3, value3); \
XML_LAST_CASE_ERROR(errorMsg)

#define XML_SET_VAR_THREE_OPTIONS_ERROR_AND_IGNORE(attribute, variable, option1, value1, option2, value2, option3, value3, errorMsg, ignoreOption) \
XML_SET_VAR_THREE_OPTIONS_WITH_IGNORE(attribute, variable, option1, value1, option2, value2, option3, value3, ignoreOption); \
XML_LAST_CASE_ERROR(errorMsg)

#define XML_SET_VAR_FOUR_OPTIONS(attribute, variable, option1, value1, option2, value2, option3, value3, option4, value4) \
XML_SET_VAR_THREE_OPTIONS(attribute, variable, option1, value1, option2, value2, option3, value3); \
XML_SET_VAR_ONE_OPTION_CONTINUE(attribute, variable, option4, value4)

#define XML_SET_VAR_FOUR_OPTIONS_WITH_IGNORE(attribute, variable, option1, value1, option2, value2, option3, value3, option4, value4, ignoreOption) \
XML_IGNORE_OPTION(attribute, ignoreOption) \
XML_SET_VAR_FOUR_OPTIONS(attribute, variable, option1, value1, option2, value2, option3, value3, option4, value4)

#define XML_SET_VAR_FOUR_OPTIONS_WITH_ERROR(attribute, variable, option1, value1, option2, value2, option3, value3, option4, value4, errorMsg) \
XML_SET_VAR_FOUR_OPTIONS(attribute, variable, option1, value1, option2, value2, option3, value3, option4, value4); \
XML_LAST_CASE_ERROR(errorMsg)

#define XML_SET_VAR_FOUR_OPTIONS_ERROR_AND_IGNORE(attribute, variable, option1, value1, option2, value2, option3, value3, option4, value4, errorMsg, ignoreOption) \
XML_SET_VAR_FOUR_OPTIONS_WITH_IGNORE(attribute, variable, option1, value1, option2, value2, option3, value3, option4, value4, ignoreOption); \
XML_LAST_CASE_ERROR(errorMsg)

#define XML_SET_VAR_FIVE_OPTIONS(attribute, variable, option1, value1, option2, value2, option3, value3, option4, value4, option5, value5) \
XML_SET_VAR_FOUR_OPTIONS(attribute, variable, option1, value1, option2, value2, option3, value3, option4, value4); \
XML_SET_VAR_ONE_OPTION_CONTINUE(attribute, variable, option4, value4)

#define XML_SET_VAR_FIVE_OPTIONS_WITH_IGNORE(attribute, variable, option1, value1, option2, value2, option3, value3, option4, value4, option5, value5, ignoreOption) \
XML_IGNORE_OPTION(attribute, ignoreOption) \
XML_SET_VAR_FIVE_OPTIONS(attribute, variable, option1, value1, option2, value2, option3, value3, option4, value4, option5, value5)

#define XML_SET_VAR_FIVE_OPTIONS_WITH_ERROR(attribute, variable, option1, value1, option2, value2, option3, value3, option4, value4, option5, value5, errorMsg) \
XML_SET_VAR_FIVE_OPTIONS(attribute, variable, option1, value1, option2, value2, option3, value3, option4, value4, option5, value5); \
XML_LAST_CASE_ERROR(errorMsg)

#define XML_SET_VAR_FIVE_OPTIONS_ERROR_AND_IGNORE(attribute, variable, option1, value1, option2, value2, option3, value3, option4, value4, option5, value5, errorMsg, ignoreOption) \
XML_SET_VAR_FIVE_OPTIONS_WITH_IGNORE(attribute, variable, option1, value1, option2, value2, option3, value3, option4, value4, option5, value5, ignoreOption); \
XML_LAST_CASE_ERROR(errorMsg)

#include <string>
#ifndef __USING_UWP

//-----------------------------------------------------------------------------------------------
class XMLUtils
{
public:
	static struct XMLNode GetRootNode(const QuString& filepath);
	static QuString GetAttribute(const struct XMLNode& node, const QuString& attributeName);
	static QuString GetName(const struct XMLNode& node);
};
#endif