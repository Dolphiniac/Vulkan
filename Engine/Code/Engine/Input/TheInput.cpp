#include "Engine/Input/TheInput.hpp"
#include "Engine/Input/TheKeyboard.hpp"
#include "Engine/Input/XboxController.hpp"
#include "ThirdParty/XML/xml.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


//-----------------------------------------------------------------------------------------------
TheInput::TheInput()
: m_isHidingMouse(false)
, m_defaultMousePosition(IntVector2::Zero)
, m_isResettingMouse(false)
, m_deltaMousePos(IntVector2::Zero)
{
	g_controllerOne = new XboxController(1);
	g_controllerTwo = new XboxController(2);
	g_controllerThree = new XboxController(3);
	g_controllerFour = new XboxController(4);

	g_theKeyboard = new TheKeyboard();

#ifndef __USING_UWP
	SetMousePos(m_defaultMousePosition);
#endif
	BuildInputMap();
}


//-----------------------------------------------------------------------------------------------
TheInput::~TheInput()
{
	delete g_controllerOne;
	delete g_controllerTwo;
	delete g_controllerThree;
	delete g_controllerFour;
	delete g_theKeyboard;
	m_globalStringTranslator.clear();
	m_actionTranslator.clear();
	
	g_controllerOne = nullptr;
	g_controllerTwo = nullptr;
	g_controllerThree = nullptr;
	g_controllerFour = nullptr;
	g_theKeyboard = nullptr;
}


//-----------------------------------------------------------------------------------------------
void TheInput::BuildInputMap()
{
	for (char c = 'A'; c <= 'Z'; c++)
	{
		std::string s = "";
		s.push_back(c);
		m_globalStringTranslator[s] = (int)c;
	}
	for (char c = '0'; c <= '9'; c++)
	{
		std::string s = "";
		s.push_back(c);
		m_globalStringTranslator[s] = (int)c;
	}
	m_globalStringTranslator["KB_SPACE"] = KB_SPACE;
	m_globalStringTranslator["KB_ESC"] = KB_ESC;
	m_globalStringTranslator["KB_LEFT"] = KB_LEFT;
	m_globalStringTranslator["KB_RIGHT"] = KB_RIGHT;
	m_globalStringTranslator["KB_UP"] = KB_UP;
	m_globalStringTranslator["KB_DOWN"] = KB_DOWN;
	m_globalStringTranslator["KB_SHIFT"] = KB_SHIFT;
	m_globalStringTranslator["KB_CTRL"] = KB_CTRL;
	m_globalStringTranslator["KB_ALT"] = KB_ALT;
	m_globalStringTranslator["KB_F1"] = KB_F1;
	m_globalStringTranslator["KB_F2"] = KB_F2;
	m_globalStringTranslator["KB_F3"] = KB_F3;
	m_globalStringTranslator["KB_F4"] = KB_F4;
	m_globalStringTranslator["KB_F5"] = KB_F5;
	m_globalStringTranslator["KB_F6"] = KB_F6;
	m_globalStringTranslator["KB_F7"] = KB_F7;
	m_globalStringTranslator["KB_F8"] = KB_F8;
	m_globalStringTranslator["KB_F9"] = KB_F9;
	m_globalStringTranslator["KB_F10"] = KB_F10;
	m_globalStringTranslator["KB_F11"] = KB_F11;
	m_globalStringTranslator["KB_F12"] = KB_F12;
	m_globalStringTranslator["KB_ENTER"] = KB_ENTER;
	m_globalStringTranslator["KB_BACKSPACE"] = KB_BACKSPACE;
	m_globalStringTranslator["KB_/"] = KB_SLASHQUESTION;
	m_globalStringTranslator["KB_~"] = KB_TICKTILDE;
	m_globalStringTranslator["KB_,<"] = KB_COMMA;
	m_globalStringTranslator["KB_NUM0"] = KB_NUM0;
	m_globalStringTranslator["KB_NUM1"] = KB_NUM1;
	m_globalStringTranslator["KB_NUM2"] = KB_NUM2;
	m_globalStringTranslator["KB_NUM3"] = KB_NUM3;
	m_globalStringTranslator["KB_NUM4"] = KB_NUM4;
	m_globalStringTranslator["KB_NUM5"] = KB_NUM5;
	m_globalStringTranslator["KB_NUM6"] = KB_NUM6;
	m_globalStringTranslator["KB_NUM7"] = KB_NUM7;
	m_globalStringTranslator["KB_NUM8"] = KB_NUM8;
	m_globalStringTranslator["KB_NUM9"] = KB_NUM9;
	m_globalStringTranslator["M_LEFT"] = M_LEFT;
	m_globalStringTranslator["M_RIGHT"] = M_RIGHT;
	m_globalStringTranslator["M_WHEEL_UP"] = M_WHEEL_UP;
	m_globalStringTranslator["M_WHEEL_DOWN"] = M_WHEEL_DOWN;
}

#ifndef __USING_UWP
//-----------------------------------------------------------------------------------------------
void TheInput::SetInputActionMapping(const std::string& xmlFile)
{
	XMLNode root = XMLNode::parseFile(("Data/Input/" + xmlFile + ".xml").c_str()).getChildNode();
	int searchHandle = 0;
	for (int i = 0; i < root.nChildNode("InputTranslation"); i++)
	{
		XMLNode mapping = root.getChildNode("InputTranslation", &searchHandle);
		const char* fromKey = mapping.getAttribute("fromKey");
		ASSERT_OR_DIE(fromKey, "fromKey field not set");
		auto globalIter = m_globalStringTranslator.find(fromKey);
		ASSERT_OR_DIE(globalIter != m_globalStringTranslator.end(), "fromKey field invalid");
		const char* toAction = mapping.getAttribute("toAction");
		ASSERT_OR_DIE(toAction, "toAction field not set");
		auto localIter = m_actionTranslator.find(toAction);
		if (localIter == m_actionTranslator.end())
		{
			m_actionTranslator[toAction] = std::vector<int>();
		}
		m_actionTranslator[toAction].push_back(globalIter->second);
	}
}
#endif

//-----------------------------------------------------------------------------------------------
bool TheInput::GetAction(const std::string& alias)
{
	if (!IsAppInFocus())
	{
		return false;
	}
	auto mapIter = m_actionTranslator.find(alias);
	ASSERT_OR_DIE(mapIter != m_actionTranslator.end(), Stringf("Action %s not found", alias.c_str()));
	for (int keyCode : mapIter->second)
	{
		if (GetKey(keyCode))
		{
			return true;
		}
	}
	return false;
}


//-----------------------------------------------------------------------------------------------
bool TheInput::GetActionDown(const std::string& alias)
{
	if (!IsAppInFocus())
	{
		return false;
	}
	auto mapIter = m_actionTranslator.find(alias);
	ASSERT_OR_DIE(mapIter != m_actionTranslator.end(), Stringf("Action %s not found", alias.c_str()));
	for (int keyCode : mapIter->second)
	{
		if (GetKeyDown(keyCode))
		{
			return true;
		}
	}
	return false;
}


//-----------------------------------------------------------------------------------------------
bool TheInput::GetActionUp(const std::string& alias)
{
	if (!IsAppInFocus())
	{
		return false;
	}
	auto mapIter = m_actionTranslator.find(alias);
	ASSERT_OR_DIE(mapIter != m_actionTranslator.end(), Stringf("Action %s not found", alias.c_str()));
	for (int keyCode : mapIter->second)
	{
		if (GetKeyUp(keyCode))
		{
			return true;
		}
	}
	return false;
}

#ifndef __USING_UWP
//-----------------------------------------------------------------------------------------------
void TheInput::SetShouldHideMouse(bool shouldHideMouse)
{
	m_isHidingMouse = shouldHideMouse;
	ShowCursor(!m_isHidingMouse);
}


//-----------------------------------------------------------------------------------------------
IntVector2 TheInput::GetMousePos()
{
	if (!IsAppInFocus())
	{
		return IntVector2::Zero;
	}
	POINT mousePos;
	GetCursorPos(&mousePos);

	return IntVector2(mousePos.x, mousePos.y);
}


//-----------------------------------------------------------------------------------------------
void TheInput::SetMousePos(IntVector2 toSet)
{
	SetCursorPos(toSet.x, toSet.y);
}


//-----------------------------------------------------------------------------------------------
void TheInput::SetMousePos(int x, int y)
{
	SetCursorPos(x, y);
}


//-----------------------------------------------------------------------------------------------
void TheInput::SetDefaultMousePos(IntVector2 toSet)
{
	m_defaultMousePosition = toSet;
}


//-----------------------------------------------------------------------------------------------
void TheInput::SetDefaultMousePos(int x, int y)
{
	m_defaultMousePosition.x = x;
	m_defaultMousePosition.y = y;
}


//-----------------------------------------------------------------------------------------------
void TheInput::SetAutomaticMouseReset(bool shouldReset)
{
	m_isResettingMouse = shouldReset;
	if (shouldReset)
	{
		SetMousePos(m_defaultMousePosition);
	}
}


//-----------------------------------------------------------------------------------------------
IntVector2 TheInput::GetDeltaMousePosi()
{
	if (!IsAppInFocus())
	{
		return IntVector2::Zero;
	}
	return m_deltaMousePos;
}


//-----------------------------------------------------------------------------------------------
Vector2 TheInput::GetDeltaMousePosf()
{
	if (!IsAppInFocus())
	{
		return Vector2::Zero;
	}
	float x = (float)m_deltaMousePos.x;
	float y = (float)m_deltaMousePos.y;

	return Vector2(x, y);
}
#endif

//-----------------------------------------------------------------------------------------------
bool TheInput::IsAppInFocus()
{
	return m_isInFocus;
}

#ifndef __USING_UWP
//-----------------------------------------------------------------------------------------------
void TheInput::SetKeyStatus(unsigned char asKey, bool wasPressed)
{
	g_theKeyboard->SetKeyStatus(asKey, wasPressed);
}
#endif

//-----------------------------------------------------------------------------------------------
void TheInput::SetInFocus(bool inFocus)
{
	m_isInFocus = inFocus;
}


//-----------------------------------------------------------------------------------------------
bool TheInput::GetKey(int keyCode)
{
	if (!IsAppInFocus())
	{
		return false;
	}
	return g_theKeyboard->GetKey(keyCode);
}


//-----------------------------------------------------------------------------------------------
bool TheInput::GetKeyDown(int keyCode)
{
	if (!IsAppInFocus())
	{
		return false;
	}
	return g_theKeyboard->GetKeyDown(keyCode);
}


//-----------------------------------------------------------------------------------------------
bool TheInput::GetKeyUp(int keyCode)
{
	if (!IsAppInFocus())
	{
		return false;
	}
	return g_theKeyboard->GetKeyUp(keyCode);
}


//-----------------------------------------------------------------------------------------------
bool TheInput::GetButton(int buttonCode)
{
	if (!IsAppInFocus())
	{
		return false;
	}
	if (g_controllerOne->IsConnected())
	{
		return g_controllerOne->GetButton(buttonCode);
	}
	else if (g_controllerTwo->IsConnected())
	{
		return g_controllerTwo->GetButton(buttonCode);
	}
	else if (g_controllerThree->IsConnected())
	{
		return g_controllerThree->GetButton(buttonCode);
	}
	else if (g_controllerFour->IsConnected())
	{
		return g_controllerFour->GetButton(buttonCode);
	}
	else
	{
		return false;
	}
}


//-----------------------------------------------------------------------------------------------
bool TheInput::GetButtonDown(int buttonCode)
{
	if (!IsAppInFocus())
	{
		return false;
	}
	if (g_controllerOne->IsConnected())
	{
		return g_controllerOne->GetButtonDown(buttonCode);
	}
	else if (g_controllerTwo->IsConnected())
	{
		return g_controllerTwo->GetButtonDown(buttonCode);
	}
	else if (g_controllerThree->IsConnected())
	{
		return g_controllerThree->GetButtonDown(buttonCode);
	}
	else if (g_controllerFour->IsConnected())
	{
		return g_controllerFour->GetButtonDown(buttonCode);
	}
	else
	{
		return false;
	}
}


//-----------------------------------------------------------------------------------------------
bool TheInput::GetButtonUp(int buttonCode)
{
	if (!IsAppInFocus())
	{
		return false;
	}
	if (g_controllerOne->IsConnected())
	{
		return g_controllerOne->GetButtonUp(buttonCode);
	}
	else if (g_controllerTwo->IsConnected())
	{
		return g_controllerTwo->GetButtonUp(buttonCode);
	}
	else if (g_controllerThree->IsConnected())
	{
		return g_controllerThree->GetButtonUp(buttonCode);
	}
	else if (g_controllerFour->IsConnected())
	{
		return g_controllerFour->GetButtonUp(buttonCode);
	}
	else
	{
		return false;
	}
}


//-----------------------------------------------------------------------------------------------
float TheInput::GetLeftTrigger()
{
	if (!IsAppInFocus())
	{
		return 0.f;
	}
	if (g_controllerOne->IsConnected())
	{
		return g_controllerOne->GetTrigger(LEFT_TRIGGER);
	}
	else if (g_controllerTwo->IsConnected())
	{
		return g_controllerTwo->GetTrigger(LEFT_TRIGGER);
	}
	else if (g_controllerThree->IsConnected())
	{
		return g_controllerThree->GetTrigger(LEFT_TRIGGER);
	}
	else if (g_controllerFour->IsConnected())
	{
		return g_controllerFour->GetTrigger(LEFT_TRIGGER);
	}
	else
	{
		return 0.f;
	}
}


//-----------------------------------------------------------------------------------------------
float TheInput::GetRightTrigger()
{
	if (!IsAppInFocus())
	{
		return 0.f;
	}
	if (g_controllerOne->IsConnected())
	{
		return g_controllerOne->GetTrigger(RIGHT_TRIGGER);
	}
	else if (g_controllerTwo->IsConnected())
	{
		return g_controllerTwo->GetTrigger(RIGHT_TRIGGER);
	}
	else if (g_controllerThree->IsConnected())
	{
		return g_controllerThree->GetTrigger(RIGHT_TRIGGER);
	}
	else if (g_controllerFour->IsConnected())
	{
		return g_controllerFour->GetTrigger(RIGHT_TRIGGER);
	}
	else
	{
		return 0.f;
	}
}


//-----------------------------------------------------------------------------------------------
float TheInput::GetLeftTriggerBinary()
{
	if (!IsAppInFocus())
	{
		return 0.f;
	}
	if (g_controllerOne->IsConnected())
	{
		return g_controllerOne->GetTriggerBinary(LEFT_TRIGGER);
	}
	else if (g_controllerTwo->IsConnected())
	{
		return g_controllerTwo->GetTriggerBinary(LEFT_TRIGGER);
	}
	else if (g_controllerThree->IsConnected())
	{
		return g_controllerThree->GetTriggerBinary(LEFT_TRIGGER);
	}
	else if (g_controllerFour->IsConnected())
	{
		return g_controllerFour->GetTriggerBinary(LEFT_TRIGGER);
	}
	else
	{
		return 0.f;
	}
}


//-----------------------------------------------------------------------------------------------
float TheInput::GetRightTriggerBinary()
{
	if (!IsAppInFocus())
	{
		return 0.f;
	}
	if (g_controllerOne->IsConnected())
	{
		return g_controllerOne->GetTriggerBinary(RIGHT_TRIGGER);
	}
	else if (g_controllerTwo->IsConnected())
	{
		return g_controllerTwo->GetTriggerBinary(RIGHT_TRIGGER);
	}
	else if (g_controllerThree->IsConnected())
	{
		return g_controllerThree->GetTriggerBinary(RIGHT_TRIGGER);
	}
	else if (g_controllerFour->IsConnected())
	{
		return g_controllerFour->GetTriggerBinary(RIGHT_TRIGGER);
	}
	else
	{
		return 0.f;
	}
}


float TheInput::GetLeftPolarRadians()
{
	return g_controllerOne->GetPolarRadians(LEFT_STICK);
}

float TheInput::GetRightPolarRadians()
{
	return g_controllerOne->GetPolarRadians(RIGHT_STICK);
}

//-----------------------------------------------------------------------------------------------
float TheInput::GetHorizontalAxis()
{
	if (!IsAppInFocus())
	{
		return 0.f;
	}
	if (g_controllerOne->IsConnected())
	{
		return g_controllerOne->GetHorizontalAxis(LEFT_STICK);
	}
	else
	{
		return g_theKeyboard->GetHorizontalAxisBinary();
	}
}


//-----------------------------------------------------------------------------------------------
float TheInput::GetVerticalAxis()
{
	if (!IsAppInFocus())
	{
		return 0.f;
	}
	if (g_controllerOne->IsConnected())
	{
		return g_controllerOne->GetVerticalAxis(LEFT_STICK);
	}
	else
	{
		return g_theKeyboard->GetVerticalAxisBinary();
	}
}


//-----------------------------------------------------------------------------------------------
float TheInput::GetHorizontalAxisBinary()
{
	if (!IsAppInFocus())
	{
		return 0.f;
	}
	if (g_controllerOne->IsConnected())
	{
		return g_controllerOne->GetHorizontalAxisBinary(LEFT_STICK);
	}
	else
	{
		return g_theKeyboard->GetHorizontalAxisBinary();
	}
}


//-----------------------------------------------------------------------------------------------
float TheInput::GetVerticalAxisBinary()
{
	if (!IsAppInFocus())
	{
		return 0.f;
	}
	if (g_controllerOne->IsConnected())
	{
		return g_controllerOne->GetVerticalAxisBinary(LEFT_STICK);
	}
	else
	{
		return g_theKeyboard->GetVerticalAxisBinary();
	}
}


//-----------------------------------------------------------------------------------------------
float TheInput::GetHorizontalKeyboardAxis()
{
	if (!IsAppInFocus())
	{
		return 0.f;
	}
	return g_theKeyboard->GetHorizontalAxisBinary();
}


//-----------------------------------------------------------------------------------------------
float TheInput::GetVerticalKeyboardAxis()
{
	if (!IsAppInFocus())
	{
		return 0.f;
	}
	return g_theKeyboard->GetVerticalAxisBinary();
}


//-----------------------------------------------------------------------------------------------
float TheInput::GetHorizontalXboxAxisLeft()
{
	if (!IsAppInFocus())
	{
		return 0.f;
	}
	if (g_controllerOne->IsConnected())
	{
		return g_controllerOne->GetHorizontalAxis(LEFT_STICK);
	}
	else if (g_controllerTwo->IsConnected())
	{
		return g_controllerTwo->GetHorizontalAxis(LEFT_STICK);
	}
	else if (g_controllerThree->IsConnected())
	{
		return g_controllerThree->GetHorizontalAxis(LEFT_STICK);
	}
	else if (g_controllerFour->IsConnected())
	{
		return g_controllerFour->GetHorizontalAxis(LEFT_STICK);
	}
	else
	{
		return 0.f;
	}
}


//-----------------------------------------------------------------------------------------------
float TheInput::GetHorizontalXboxAxisRight()
{
	if (!IsAppInFocus())
	{
		return 0.f;
	}
	if (g_controllerOne->IsConnected())
	{
		return g_controllerOne->GetHorizontalAxis(RIGHT_STICK);
	}
	else if (g_controllerTwo->IsConnected())
	{
		return g_controllerTwo->GetHorizontalAxis(RIGHT_STICK);
	}
	else if (g_controllerThree->IsConnected())
	{
		return g_controllerThree->GetHorizontalAxis(RIGHT_STICK);
	}
	else if (g_controllerFour->IsConnected())
	{
		return g_controllerFour->GetHorizontalAxis(RIGHT_STICK);
	}
	else
	{
		return 0.f;
	}
}


//-----------------------------------------------------------------------------------------------
float TheInput::GetVerticalXboxAxisLeft()
{
	if (!IsAppInFocus())
	{
		return 0.f;
	}

	if (g_controllerOne->IsConnected())
	{
		return g_controllerOne->GetVerticalAxis(LEFT_STICK);
	}
	else if (g_controllerTwo->IsConnected())
	{
		return g_controllerTwo->GetVerticalAxis(LEFT_STICK);
	}
	else if (g_controllerThree->IsConnected())
	{
		return g_controllerThree->GetVerticalAxis(LEFT_STICK);
	}
	else if (g_controllerFour->IsConnected())
	{
		return g_controllerFour->GetVerticalAxis(LEFT_STICK);
	}
	else
	{
		return 0.f;
	}
}


//-----------------------------------------------------------------------------------------------
float TheInput::GetVerticalXboxAxisRight()
{
	if (!IsAppInFocus())
	{
		return 0.f;
	}
	if (g_controllerOne->IsConnected())
	{
		return g_controllerOne->GetVerticalAxis(RIGHT_STICK);
	}
	else if (g_controllerTwo->IsConnected())
	{
		return g_controllerTwo->GetVerticalAxis(RIGHT_STICK);
	}
	else if (g_controllerThree->IsConnected())
	{
		return g_controllerThree->GetVerticalAxis(RIGHT_STICK);
	}
	else if (g_controllerFour->IsConnected())
	{
		return g_controllerFour->GetVerticalAxis(RIGHT_STICK);
	}
	else
	{
		return 0.f;
	}
}


//-----------------------------------------------------------------------------------------------
float TheInput::GetHorizontalXboxAxisLeftBinary()
{
	if (!IsAppInFocus())
	{
		return 0.f;
	}
	if (g_controllerOne->IsConnected())
	{
		return g_controllerOne->GetHorizontalAxisBinary(LEFT_STICK);
	}
	else if (g_controllerTwo->IsConnected())
	{
		return g_controllerTwo->GetHorizontalAxisBinary(LEFT_STICK);
	}
	else if (g_controllerThree->IsConnected())
	{
		return g_controllerThree->GetHorizontalAxisBinary(LEFT_STICK);
	}
	else if (g_controllerFour->IsConnected())
	{
		return g_controllerFour->GetHorizontalAxisBinary(LEFT_STICK);
	}
	else
	{
		return 0.f;
	}
}


//-----------------------------------------------------------------------------------------------
float TheInput::GetHorizontalXboxAxisRightBinary()
{
	if (!IsAppInFocus())
	{
		return 0.f;
	}
	if (g_controllerOne->IsConnected())
	{
		return g_controllerOne->GetHorizontalAxisBinary(RIGHT_STICK);
	}
	else if (g_controllerTwo->IsConnected())
	{
		return g_controllerTwo->GetHorizontalAxisBinary(RIGHT_STICK);
	}
	else if (g_controllerThree->IsConnected())
	{
		return g_controllerThree->GetHorizontalAxisBinary(RIGHT_STICK);
	}
	else if (g_controllerFour->IsConnected())
	{
		return g_controllerFour->GetHorizontalAxisBinary(RIGHT_STICK);
	}
	else
	{
		return 0.f;
	}
}


//-----------------------------------------------------------------------------------------------
float TheInput::GetVerticalXboxAxisLeftBinary()
{
	if (!IsAppInFocus())
	{
		return 0.f;
	}
	if (g_controllerOne->IsConnected())
	{
		return g_controllerOne->GetVerticalAxisBinary(LEFT_STICK);
	}
	else if (g_controllerTwo->IsConnected())
	{
		return g_controllerTwo->GetVerticalAxisBinary(LEFT_STICK);
	}
	else if (g_controllerThree->IsConnected())
	{
		return g_controllerThree->GetVerticalAxisBinary(LEFT_STICK);
	}
	else if (g_controllerFour->IsConnected())
	{
		return g_controllerFour->GetVerticalAxisBinary(LEFT_STICK);
	}
	else
	{
		return 0.f;
	}
}


//-----------------------------------------------------------------------------------------------
float TheInput::GetVerticalXboxAxisRightBinary()
{
	if (!IsAppInFocus())
	{
		return 0.f;
	}
	if (g_controllerOne->IsConnected())
	{
		return g_controllerOne->GetVerticalAxisBinary(RIGHT_STICK);
	}
	else if (g_controllerTwo->IsConnected())
	{
		return g_controllerTwo->GetVerticalAxisBinary(RIGHT_STICK);
	}
	else if (g_controllerThree->IsConnected())
	{
		return g_controllerThree->GetVerticalAxisBinary(RIGHT_STICK);
	}
	else if (g_controllerFour->IsConnected())
	{
		return g_controllerFour->GetVerticalAxisBinary(RIGHT_STICK);
	}
	else
	{
		return 0.f;
	}
}


//-----------------------------------------------------------------------------------------------
void TheInput::Tick()
{
	g_theKeyboard->Update();
	g_controllerOne->Update();
#ifndef __USING_UWP
	IntVector2 endPos = GetMousePos();
	if (IsAppInFocus())
	{
		m_deltaMousePos = endPos - m_defaultMousePosition;
	}
	else
	{
		m_deltaMousePos = IntVector2::Zero;
		return;
	}
#endif


/*

	if (m_isHidingMouse)
	{
		ShowCursor(false);
	}
	else
	{
		ShowCursor(true);
	}*/
#ifndef __USING_UWP
	if (m_isResettingMouse)
	{
		SetMousePos(m_defaultMousePosition);
	}
#endif
}