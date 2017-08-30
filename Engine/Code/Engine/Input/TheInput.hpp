#pragma once

#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/Vector2.hpp"

#include <map>
#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class TheInput
{
	friend class EngineSystemManager;
public:
	TheInput();
	~TheInput();

#ifndef __USING_UWP
	void SetInputActionMapping(const std::string& xmlFile);
	void SetShouldHideMouse(bool shouldHideMouse);
	IntVector2 GetMousePos();
	void SetMousePos(IntVector2 toSet);
	void SetMousePos(int x, int y);
	void SetDefaultMousePos(IntVector2 toSet);
	void SetDefaultMousePos(int x, int y);
	void SetAutomaticMouseReset(bool shouldReset);
	IntVector2 GetDeltaMousePosi();
	Vector2 GetDeltaMousePosf();
	IntVector2 GetDefaultMousePosi() const { return m_defaultMousePosition; }
	Vector2 GetDefaultMousePosf() const { return m_defaultMousePosition; }
	void SetKeyStatus(unsigned char asKey, bool wasPressed);
#endif
	void SetInFocus(bool inFocus);

	//Keyboard button presses
	bool GetKey(int keyCode);
	bool GetKeyDown(int keyCode);
	bool GetKeyUp(int keyCode);

	//Xbox button presses
	bool GetButton(int buttonCode);
	bool GetButtonDown(int buttonCode);
	bool GetButtonUp(int buttonCode);
	float GetLeftTrigger();
	float GetRightTrigger();
	float GetLeftTriggerBinary();
	float GetRightTriggerBinary();
	float GetLeftPolarRadians();
	float GetRightPolarRadians();

	//Aliased action presses
	bool GetAction(const std::string& alias);
	bool GetActionDown(const std::string& alias);
	bool GetActionUp(const std::string& alias);

	//Axis funcs
	float GetHorizontalAxis();
	float GetVerticalAxis();
	float GetHorizontalAxisBinary();
	float GetVerticalAxisBinary();
	float GetHorizontalKeyboardAxis();
	float GetVerticalKeyboardAxis();
	float GetHorizontalXboxAxisLeft();
	float GetHorizontalXboxAxisRight();
	float GetVerticalXboxAxisLeft();
	float GetVerticalXboxAxisRight();
	float GetHorizontalXboxAxisLeftBinary();
	float GetHorizontalXboxAxisRightBinary();
	float GetVerticalXboxAxisLeftBinary();
	float GetVerticalXboxAxisRightBinary();



	void Tick();

private:
	bool IsAppInFocus();
	void BuildInputMap();

private:
	bool											m_isInFocus;
	bool											m_isHidingMouse;
	IntVector2										m_defaultMousePosition;
	bool											m_isResettingMouse;
	IntVector2										m_deltaMousePos;
	std::map<std::string, int>						m_globalStringTranslator;
	std::map<std::string, std::vector<int>>			m_actionTranslator;
};


//-----------------------------------------------------------------------------------------------
const int KB_SPACE = 0x20;
const int KB_ESC = 0x1b;
const int KB_LEFT = 0x25;
const int KB_UP = 0x26;
const int KB_RIGHT = 0x27;
const int KB_DOWN = 0x28;
const int KB_SHIFT = 0x10;
const int KB_CTRL = 0x11;
const int KB_ALT = 0x12;
const int KB_F1 = 0x70;
const int KB_F2 = 0x71;
const int KB_F3 = 0x72;
const int KB_F4 = 0x73;
const int KB_F5 = 0x74;
const int KB_F6 = 0x75;
const int KB_F7 = 0x76;
const int KB_F8 = 0x77;
const int KB_F9 = 0x78;
const int KB_F10 = 0x79;
const int KB_F11 = 0x7a;
const int KB_F12 = 0x7b;
const int KB_ENTER = 0x0d;
const int KB_BACKSPACE = 0x08;
const int KB_SLASHQUESTION = 0xBF;
const int KB_TICKTILDE = 0xC0;
const int KB_NUM0 = 0x60;
const int KB_NUM1 = 0x61;
const int KB_NUM2 = 0x62;
const int KB_NUM3 = 0x63;
const int KB_NUM4 = 0x64;
const int KB_NUM5 = 0x65;
const int KB_NUM6 = 0x66;
const int KB_NUM7 = 0x67; 
const int KB_NUM8 = 0x68;
const int KB_NUM9 = 0x69;
const int KB_COMMA = 0xBC;
const int M_LEFT= 0x01;
const int M_RIGHT = 0x02;
const int M_WHEEL_UP = 0x05;
const int M_WHEEL_DOWN = 0x06;


//May seem unnecessary, but every controller I'll ever make will use these constants, and
//typing XINPUT_GAMEPAD every time is too much for me
const int VB_DUP = 0x0001		/*XINPUT_GAMEPAD_DPAD_UP*/;
const int VB_DDOWN = 0x0002		/*XINPUT_GAMEPAD_DPAD_DOWN*/;
const int VB_DLEFT = 0x0004		/*XINPUT_GAMEPAD_DPAD_LEFT*/;
const int VB_DRIGHT = 0x0008		/*XINPUT_GAMEPAD_DPAD_RIGHT*/;
const int VB_RIGHT_STICK = 0x0080		/*XINPUT_GAMEPAD_RIGHT_THUMB*/;
const int VB_LEFT_STICK = 0x0040		/*XINPUT_GAMEPAD_LEFT_THUMB*/;
const int VB_A = 0x1000		/*XINPUT_GAMEPAD_A*/;
const int VB_B = 0x2000		/*XINPUT_GAMEPAD_B*/;
const int VB_X = 0x4000		/*XINPUT_GAMEPAD_X*/;
const int VB_Y = 0x8000		/*XINPUT_GAMEPAD_Y*/;
const int VB_RIGHT_BUMPER = 0x0200		/*XINPUT_GAMEPAD_RIGHT_SHOULDER*/;
const int VB_LEFT_BUMPER = 0x0100		/*XINPUT_GAMEPAD_LEFT_SHOULDER*/;
const int VB_START = 0x0010		/*XINPUT_GAMEPAD_START*/;
const int VB_BACK = 0x0020		/*XINPUT_GAMEPAD_BACK*/;