#pragma once

#include "Engine/Input/TheInput.hpp"


//-----------------------------------------------------------------------------------------------
class TheKeyboard;


//-----------------------------------------------------------------------------------------------
extern TheKeyboard *g_theKeyboard;


//-----------------------------------------------------------------------------------------------
const int NUM_KEYS = 256;


//-----------------------------------------------------------------------------------------------
class TheKeyboard
{
	friend class TheInput;

private:
	TheKeyboard();
	void	InitializeArrays();
	bool	GetKey(int keyCode) const;
	bool	GetKeyDown(int keyCode) const;
	bool	GetKeyUp(int keyCode) const;
	float	GetHorizontalAxisBinary() const;
	float	GetVerticalAxisBinary() const;
	void	SetKeyStatus(int keyCode, bool wasPressed);
	void	Update();

private:
	bool	m_isKeyDown[NUM_KEYS];
	bool	m_wasKeyJustPressed[NUM_KEYS];
	bool	m_wasKeyJustReleased[NUM_KEYS];
};