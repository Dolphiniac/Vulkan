#include "Engine/Input/XboxController.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

#include <winerror.h>
#include <cstdlib>
#include <math.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Xinput.h>
#ifndef __USING_UWP
#pragma comment(lib, "xinput9_1_0")
#endif


//-----------------------------------------------------------------------------------------------
XboxController* g_controllerOne = nullptr;
XboxController* g_controllerTwo = nullptr;
XboxController* g_controllerThree = nullptr;
XboxController* g_controllerFour = nullptr;


//-----------------------------------------------------------------------------------------------
XboxController::XboxController(int controllerNum)
: m_controllerNum(controllerNum)
, m_leftStickX(0.f)
, m_leftStickY(0.f)
, m_rightStickX(0.f)
, m_rightStickY(0.f)
, m_rightTrigger(0.f)
, m_leftTrigger(0.f)
, m_currButtons(0)
, m_prevButtons(0)
, m_errorMsg(ERROR_DEVICE_NOT_CONNECTED)
{
	SetValues();
	SetAxes();
	SetTriggers();
}


//-----------------------------------------------------------------------------------------------
void XboxController::SetValues()
{
	XINPUT_STATE state;
	memset(&state, 0, sizeof(state));

	m_errorMsg = XInputGetState(m_controllerNum - 1, &state);

	if (IsConnected())
	{
		m_leftStickX = (float)state.Gamepad.sThumbLX;
		m_leftStickY = (float)state.Gamepad.sThumbLY;
		m_rightStickX = (float)state.Gamepad.sThumbRX;
		m_rightStickY = (float)state.Gamepad.sThumbRY;
		m_rightTrigger = (float)state.Gamepad.bRightTrigger;
		m_leftTrigger = (float)state.Gamepad.bLeftTrigger;
		m_prevButtons = m_currButtons;
		m_currButtons = state.Gamepad.wButtons;
	}
}


//-----------------------------------------------------------------------------------------------
bool XboxController::GetButton(int buttonCode) const
{
	return (m_currButtons & buttonCode) != 0;
}


//-----------------------------------------------------------------------------------------------
bool XboxController::GetButtonDown(int buttonCode) const
{
	return (GetButton(buttonCode)) && (m_prevButtons & buttonCode) == 0;
}


//-----------------------------------------------------------------------------------------------
bool XboxController::GetButtonUp(int buttonCode) const
{
	return (!GetButton(buttonCode)) && (m_prevButtons & buttonCode) != 0;
}


//-----------------------------------------------------------------------------------------------
float XboxController::GetHorizontalAxis(EStick stick) const
{
	return (stick == LEFT_STICK) ? m_leftStickX : m_rightStickX;
}


//-----------------------------------------------------------------------------------------------
float XboxController::GetHorizontalAxisBinary(EStick stick) const
{
	float valueX = (stick == LEFT_STICK) ? m_leftStickX : m_rightStickX;

	if (valueX < 0.f)
	{
		return -1.f;
	}
	else if (valueX > 0.f)
	{
		return 1.f;
	}
	else
	{
		return 0.f;
	}
}


//-----------------------------------------------------------------------------------------------
float XboxController::GetVerticalAxis(EStick stick) const
{
	return (stick == LEFT_STICK) ? m_leftStickY : m_rightStickY;
}


//-----------------------------------------------------------------------------------------------
float XboxController::GetVerticalAxisBinary(EStick stick) const
{
	float valueY = (stick == LEFT_STICK) ? m_leftStickY : m_rightStickY;

	if (valueY < 0.f)
	{
		return -1.f;
	}
	else if (valueY > 0.f)
	{
		return 1.f;
	}
	else
	{
		return 0.f;
	}
}


//-----------------------------------------------------------------------------------------------
float XboxController::GetPolarRadians(EStick stick) const
{
	if (stick == LEFT_STICK)
	{
		if (m_leftStickX == 0.f && m_leftStickY == 0.f)
		{
			return DEADZONE_RADIANS;
		}
		return atan2f(m_leftStickY, m_leftStickX);
	}
	else
	{
		if (m_rightStickX == 0.f && m_rightStickY == 0.f)
		{
			return DEADZONE_RADIANS;
		}
		return atan2f(m_rightStickY, m_rightStickX);
	}
}


//-----------------------------------------------------------------------------------------------
float XboxController::GetTrigger(ETrigger trigger) const
{
	return (trigger == LEFT_TRIGGER) ? m_leftTrigger : m_rightTrigger;
}


//-----------------------------------------------------------------------------------------------
float XboxController::GetTriggerBinary(ETrigger trigger) const
{
	float trigVal = (trigger == LEFT_TRIGGER) ? m_leftTrigger : m_rightTrigger;

	if (trigVal > 0.f)
	{
		return 1.f;
	}
	else
	{
		return 0.f;
	}
}


//-----------------------------------------------------------------------------------------------
void XboxController::SetAxes()
{
	Vector2 leftVec(m_leftStickX, m_leftStickY);
	Vector2 rightVec(m_rightStickX, m_rightStickY);
	GetNormalizedCorrectedVector(leftVec, LEFT_STICK);
	GetNormalizedCorrectedVector(rightVec, RIGHT_STICK);
	m_leftStickX = leftVec.x;
	m_leftStickY = leftVec.y;
	m_rightStickX = rightVec.x;
	m_rightStickY = rightVec.y;
}


//-----------------------------------------------------------------------------------------------
void XboxController::SetTriggers()
{
	m_rightTrigger = RangeMap(m_rightTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD, 255.f, 0.f, 1.f);
	m_leftTrigger = RangeMap(m_leftTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD, 255.f, 0.f, 1.f);
}


//-----------------------------------------------------------------------------------------------
//Based on in-class lecture by Squirrel Eiserloh
void XboxController::GetNormalizedCorrectedVector(Vector2& outVec, EStick stick) const
{
	static float maxAxisValue = 32768.f;
	float capAxisValue = 32000.f;

	float deadzone = (stick == LEFT_STICK) ? (float)XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE : (float)XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;

	//Normalize
	capAxisValue /= maxAxisValue;
	deadzone /= maxAxisValue;
	
	float x = (float)outVec.x / maxAxisValue;
	float y = (float)outVec.y / maxAxisValue;
	
	float radius = sqrtf((x*x) + (y*y));
	float thetaRadians = atan2f(y, x);

	radius = RangeMap(radius, deadzone, capAxisValue, 0.f, 1.f);
	radius = Clampf(radius, 0.f, 1.f);

	outVec.x = radius * cos(thetaRadians);
	outVec.y = radius * sin(thetaRadians);
}


//-----------------------------------------------------------------------------------------------
bool XboxController::IsConnected() const
{
	return (m_errorMsg == ERROR_SUCCESS);
}


//-----------------------------------------------------------------------------------------------
void XboxController::Update()
{
	SetValues();
	SetAxes();
	SetTriggers();
}


//-----------------------------------------------------------------------------------------------
XboxController::XboxController()
: m_controllerNum(1)
, m_leftStickX(0.f)
, m_leftStickY(0.f)
, m_rightStickX(0.f)
, m_rightStickY(0.f)
, m_rightTrigger(0.f)
, m_leftTrigger(0.f)
, m_currButtons(0)
, m_prevButtons(0)
, m_errorMsg(ERROR_DEVICE_NOT_CONNECTED)
{
}


//-----------------------------------------------------------------------------------------------
XboxController XboxController::operator=(const XboxController& rightController)
{
	XboxController result(rightController.m_controllerNum);
	return result;
}