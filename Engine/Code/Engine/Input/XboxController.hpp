#pragma once

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/TheInput.hpp"


//-----------------------------------------------------------------------------------------------
const float DEADZONE_RADIANS = 10.f; //Out of the range of -pi to +pi (return range of atan2)


//-----------------------------------------------------------------------------------------------
enum EStick {RIGHT_STICK, LEFT_STICK};
enum ETrigger {RIGHT_TRIGGER, LEFT_TRIGGER};


//-----------------------------------------------------------------------------------------------
typedef unsigned long DWORD;


//-----------------------------------------------------------------------------------------------
class XboxController;


//-----------------------------------------------------------------------------------------------
extern XboxController* g_controllerOne;
extern XboxController* g_controllerTwo;
extern XboxController* g_controllerThree;
extern XboxController* g_controllerFour;


//-----------------------------------------------------------------------------------------------
class XboxController
{
	friend class TheInput;
public:
	bool				IsConnected() const;

public:
	XboxController();
	XboxController(int controllerNum); //Shifted by one (accepts 1-4, not 0-3)
	void				SetValues();
	bool				GetButton(int buttonCode) const;
	bool				GetButtonDown(int buttonCode) const;
	bool				GetButtonUp(int buttonCode) const;
	float				GetHorizontalAxis(EStick stick) const;
	float				GetHorizontalAxisBinary(EStick stick) const;
	float				GetVerticalAxis(EStick stick) const;
	float				GetVerticalAxisBinary(EStick stick) const;
	float				GetPolarRadians(EStick stick) const;
	float				GetTrigger(ETrigger trigger) const;
	float				GetTriggerBinary(ETrigger trigger) const;
	void				Update();
	DWORD				m_errorMsg;
	XboxController		operator=(const XboxController& rightController);

private:
	void				SetAxes();
	void				SetTriggers();
	void				GetNormalizedCorrectedVector(Vector2& outVec, EStick stick) const;
	int					m_controllerNum;
	float				m_leftStickX;
	float				m_leftStickY;
	float				m_rightStickX;
	float				m_rightStickY;
	float				m_rightTrigger;
	float				m_leftTrigger;
	int					m_currButtons;
	int					m_prevButtons;
};