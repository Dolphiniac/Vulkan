// #include "Quantum/Hephaestus/Tokenizer.h"
// #include "Quantum/Core/String.h"
// #include "Engine/Core/ErrorWarningAssert.hpp"
// 
// 
// //-----------------------------------------------------------------------------------------------
// static bool IsNumber(char toDetermine)
// {
// 	return (toDetermine >= '0' && toDetermine <= '9');
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static bool IsLetter(char toDetermine)
// {
// 	return (toDetermine >= 'a' && toDetermine <= 'z') || (toDetermine >= 'A' && toDetermine <= 'Z') || (toDetermine == '_');
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static bool IsOperator(char toDetermine)
// {
// 	switch (toDetermine)
// 	{
// 	case '+':
// 	case '-':
// 	case '*':
// 	case '=':
// 	case '|':
// 	case '&':
// 	case '!':
// 	case '<':
// 	case '>':
// 	case '~':
// 	case '[':
// 	case ']':
// 	case '?':
// 	case ':':
// 		return true;
// 	default:
// 		return false;
// 	}
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static bool IsPunctuation(char toDetermine)
// {
// 	switch (toDetermine)
// 	{
// 	case ',':
// 	case ';':
// 	case '{':
// 	case '}':
// 	case '(':
// 	case ')':
// 		return true;
// 	default:
// 		return false;
// 	}
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static bool IsWhitespace(char toDetermine)
// {
// 	switch (toDetermine)
// 	{
// 	case ' ':
// 	case '\n':
// 	case '\t':
// 	case '\r':
// 		return true;
// 	default:
// 		return false;
// 	}
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static HDsl::ETokenType DetermineTokenType(char toDetermine)
// {
// 	using namespace HDsl;
// 
// 	if (IsNumber(toDetermine))
// 	{
// 		return H_TOKEN_TYPE_LITERAL_INT;
// 	}
// 
// 	if (IsLetter(toDetermine))
// 	{
// 		return H_TOKEN_TYPE_WORD;
// 	}
// 
// 	if (IsOperator(toDetermine))
// 	{
// 		return H_TOKEN_TYPE_OPERATOR;
// 	}
// 
// 	if (IsPunctuation(toDetermine))
// 	{
// 		return H_TOKEN_TYPE_PUNCTUATION;
// 	}
// 
// 	if (toDetermine == '/')
// 	{
// 		//This could be the start of a comment, or a division operator, so we'll set it to an unknown slash type
// 		return H_TOKEN_TYPE_SLASH;
// 	}
// 
// 	if (toDetermine == '.')
// 	{
// 		//This could either be the start of a floating point literal < 1, or a member access operator
// 		return H_TOKEN_TYPE_PERIOD;
// 	}
// 
// 	return H_TOKEN_TYPE_NOT_SET;
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static void PushAndBeginNewToken(std::queue<HDsl::HToken> &outQueue, QuString &outToken, char toCheck, HDsl::ETokenType& inOutCurrentType)
// {
// 	using namespace HDsl;
// 
// 	HToken token;
// 	token.type = inOutCurrentType;
// 	token.str = outToken;
// 
// 	outQueue.push(token);
// 
// 	outToken.Clear();
// 
// 	inOutCurrentType = DetermineTokenType(toCheck);
// 
// 	if (inOutCurrentType != H_TOKEN_TYPE_NOT_SET)
// 	{
// 		outToken.Push(toCheck);
// 	}
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static void ParseLiteralInt(std::queue<HDsl::HToken>& outQueue, QuString& outToken, char toCheck, HDsl::ETokenType& inOutCurrentType)
// {
// 	using namespace HDsl;
// 
// 	if (IsOperator(toCheck) || IsPunctuation(toCheck) || IsWhitespace(toCheck) || toCheck == '/')
// 	{
// 		PushAndBeginNewToken(outQueue, outToken, toCheck, inOutCurrentType);
// 		return;
// 	}
// 
// 	if (toCheck == '.')
// 	{
// 		inOutCurrentType = H_TOKEN_TYPE_LITERAL_DOUBLE;
// 		outToken.Push(toCheck);
// 	}
// 	else if (IsNumber(toCheck))
// 	{
// 		outToken.Push(toCheck);
// 	}
// 	else
// 	{
// 		ERROR_AND_DIE("Bad token.  Literal int needs an integer or . to continue\n");
// 	}
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static void ParseLiteralDouble(std::queue<HDsl::HToken>& outQueue, QuString& outToken, char toCheck, HDsl::ETokenType& inOutCurrentType)
// {
// 	using namespace HDsl;
// 
// 	if (IsOperator(toCheck) || IsPunctuation(toCheck) || IsWhitespace(toCheck))
// 	{
// 		ERROR_AND_DIE("Bad token.  No double-precision floating point numbers allowed.  Use an \"f\" designator\n");
// 	}
// 
// 	if (toCheck == '.')
// 	{
// 		ERROR_AND_DIE("Bad token.  Two decimal points in the same literal\n");
// 	}
// 
// 	if (IsNumber(toCheck))
// 	{
// 		outToken.Push(toCheck);
// 	}
// 	else if (toCheck == 'f')
// 	{
// 		//Choosing not to push the f character into the token.  It's not needed, except to enforce C++ float syntax
// 		inOutCurrentType = H_TOKEN_TYPE_LITERAL_FLOAT;
// 	}
// 	else
// 	{
// 		ERROR_AND_DIE("Bad token.  Literal double needs an integer or f to continue\n");
// 	}
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static void ParseLiteralFloat(std::queue<HDsl::HToken>& outQueue, QuString& outToken, char toCheck, HDsl::ETokenType& inOutCurrentType)
// {
// 	if (IsOperator(toCheck) || IsPunctuation(toCheck) || IsWhitespace(toCheck) || toCheck == '/')
// 	{
// 		PushAndBeginNewToken(outQueue, outToken, toCheck, inOutCurrentType);
// 		return;
// 	}
// 
// 	ERROR_AND_DIE("Bad token.  Nothing can follow the f in a literal float but another token\n");
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static void ParseWord(std::queue<HDsl::HToken>& outQueue, QuString& outToken, char toCheck, HDsl::ETokenType& inOutCurrentType)
// {
// 	if (IsOperator(toCheck) || IsPunctuation(toCheck) || IsWhitespace(toCheck) || toCheck == '.' || toCheck == '/')
// 	{
// 		PushAndBeginNewToken(outQueue, outToken, toCheck, inOutCurrentType);
// 		return;
// 	}
// 
// 	if (IsLetter(toCheck) || IsNumber(toCheck))
// 	{
// 		outToken.Push(toCheck);
// 	}
// 	else
// 	{
// 		ERROR_AND_DIE("Bad token.  Word token needs alphanumerical character to continue\n");
// 	}
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static void ParseOperator(std::queue<HDsl::HToken>& outQueue, QuString& outToken, char toCheck, HDsl::ETokenType& inOutCurrentType)
// {
// 	if (IsWhitespace(toCheck) || IsLetter(toCheck) || IsNumber(toCheck) || toCheck == '.' || toCheck == '/')
// 	{
// 		PushAndBeginNewToken(outQueue, outToken, toCheck, inOutCurrentType);
// 		return;
// 	}
// 
// 	if (IsOperator(toCheck))
// 	{
// 		outToken.Push(toCheck);
// 	}
// 	else
// 	{
// 		ERROR_AND_DIE("Bad token.  Operator needs operator to continue\n");
// 	}
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static void ParsePunctuation(std::queue<HDsl::HToken>& outQueue, QuString& outToken, char toCheck, HDsl::ETokenType& inOutCurrentType)
// {
// 	if (IsLetter(toCheck) || IsNumber(toCheck) || IsPunctuation(toCheck) || IsOperator(toCheck) || IsWhitespace(toCheck) || toCheck == '.' || toCheck == '/')
// 	{
// 		PushAndBeginNewToken(outQueue, outToken, toCheck, inOutCurrentType);
// 		return;
// 	}
// 
// 	ERROR_AND_DIE("Bad token.  Invalid character\n");
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static void ParseSlash(std::queue<HDsl::HToken>& outQueue, QuString& outToken, char toCheck, HDsl::ETokenType& inOutCurrentType)
// {
// 	using namespace HDsl;
// 
// 	if (toCheck == '/')
// 	{
// 		inOutCurrentType = H_TOKEN_TYPE_COMMENT_INLINE;
// 		outToken.Push(toCheck);
// 	}
// 	else if (toCheck == '*')
// 	{
// 		inOutCurrentType = H_TOKEN_TYPE_COMMENT_BLOCK;
// 		outToken.Push(toCheck);
// 	}
// 	else if (IsNumber(toCheck) || IsLetter(toCheck) || IsWhitespace(toCheck) || toCheck == '.')
// 	{
// 		inOutCurrentType = H_TOKEN_TYPE_OPERATOR;
// 		PushAndBeginNewToken(outQueue, outToken, toCheck, inOutCurrentType);
// 		return;
// 	}
// 
// 	ERROR_AND_DIE("Bad token.  / must be either the start of a comment or denote a division operator\n");
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static void ParseCommentInline(std::queue<HDsl::HToken>& outQueue, QuString& outToken, char toCheck, HDsl::ETokenType& inOutCurrentType)
// {
// 	outToken.Push(toCheck);
// 
// 	if (toCheck == '\n')
// 	{
// 		PushAndBeginNewToken(outQueue, outToken, toCheck, inOutCurrentType);
// 		return;
// 	}
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static void ParseCommentBlock(std::queue<HDsl::HToken>& outQueue, QuString& outToken, char toCheck, HDsl::ETokenType& inOutCurrentType)
// {
// 	outToken.Push(toCheck);
// 
// 	if (toCheck == '/')
// 	{
// 		if (outToken.GetLast() == '*')
// 		{
// 			PushAndBeginNewToken(outQueue, outToken, toCheck, inOutCurrentType);
// 			return;
// 		}
// 	}
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static void ParsePeriod(std::queue<HDsl::HToken>& outQueue, QuString& outToken, char toCheck, HDsl::ETokenType& inOutCurrentType)
// {
// 	using namespace HDsl;
// 
// 	if (IsNumber(toCheck))
// 	{
// 		inOutCurrentType = H_TOKEN_TYPE_LITERAL_DOUBLE;
// 		outToken.Push(toCheck);
// 	}
// 	else if (IsLetter(toCheck))
// 	{
// 		inOutCurrentType = H_TOKEN_TYPE_OPERATOR;
// 		PushAndBeginNewToken(outQueue, outToken, toCheck, inOutCurrentType);
// 		return;
// 	}
// 	else
// 	{
// 		ERROR_AND_DIE("Bad token.  Period must either begin a floating point literal or be a member access operator\n");
// 	}
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static void AdvanceTokenizationOnType(std::queue<HDsl::HToken>& outQueue, QuString& outToken, char toCheck, HDsl::ETokenType& inOutCurrentType)
// {
// 	using namespace HDsl;
// 
// 	switch (inOutCurrentType)
// 	{
// 	case H_TOKEN_TYPE_LITERAL_INT:
// 		ParseLiteralInt(outQueue, outToken, toCheck, inOutCurrentType);
// 		break;
// 	case H_TOKEN_TYPE_LITERAL_DOUBLE:
// 		ParseLiteralDouble(outQueue, outToken, toCheck, inOutCurrentType);
// 		break;
// 	case H_TOKEN_TYPE_LITERAL_FLOAT:
// 		ParseLiteralFloat(outQueue, outToken, toCheck, inOutCurrentType);
// 		break;
// 	case H_TOKEN_TYPE_WORD:
// 		ParseWord(outQueue, outToken, toCheck, inOutCurrentType);
// 		break;
// 	case H_TOKEN_TYPE_OPERATOR:
// 		ParseOperator(outQueue, outToken, toCheck, inOutCurrentType);
// 		break;
// 	case H_TOKEN_TYPE_PUNCTUATION:
// 		ParsePunctuation(outQueue, outToken, toCheck, inOutCurrentType);
// 		break;
// 	case H_TOKEN_TYPE_SLASH:
// 		ParseSlash(outQueue, outToken, toCheck, inOutCurrentType);
// 		break;
// 	case H_TOKEN_TYPE_COMMENT_INLINE:
// 		ParseCommentInline(outQueue, outToken, toCheck, inOutCurrentType);
// 		break;
// 	case H_TOKEN_TYPE_COMMENT_BLOCK:
// 		ParseCommentBlock(outQueue, outToken, toCheck, inOutCurrentType);
// 		break;
// 	case H_TOKEN_TYPE_PERIOD:
// 		ParsePeriod(outQueue, outToken, toCheck, inOutCurrentType);
// 		break;
// 	}
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// std::queue<HDsl::HToken> HDsl::Tokenize(const char* inputDsl)
// {
// 	std::queue<HToken> result;
// 
// 	uint32 inputLength = strlen(inputDsl);
// 	QuString currentToken;
// 	ETokenType currentTokenType = H_TOKEN_TYPE_NOT_SET;
// 
// 	for (uint32 charIndex = 0; charIndex < inputLength; charIndex++)
// 	{
// 		char currentChar = inputDsl[charIndex];
// 		if (currentChar == '}')
// 		{
// 			DebuggerPrintf("thingy");
// 		}
// 		//If the type hasn't been set, we want to try to figure it out from the char
// 		if (currentTokenType == H_TOKEN_TYPE_NOT_SET)
// 		{
// 			currentTokenType = DetermineTokenType(currentChar);
// 		}
// 
// 		if (currentToken.IsEmpty())
// 		{
// 			if (currentTokenType != H_TOKEN_TYPE_NOT_SET)
// 			{
// 				currentToken.Push(currentChar);
// 			}
// 		}
// 		else
// 		{
// 			AdvanceTokenizationOnType(result, currentToken, currentChar, currentTokenType);
// 		}
// 	}
// 
// 	if (!currentToken.IsEmpty())
// 	{
// 		HToken tok;
// 		tok.type = currentTokenType;
// 		tok.str = currentToken;
// 
// 		result.push(tok);
// 	}
// 
// 	return result;
// }