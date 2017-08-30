#include "Engine/Input/TheKeyboard.hpp"


//-----------------------------------------------------------------------------------------------
TheKeyboard *g_theKeyboard = nullptr;


//-----------------------------------------------------------------------------------------------
TheKeyboard::TheKeyboard()
{
	InitializeArrays();
	Update();
}


//-----------------------------------------------------------------------------------------------
bool TheKeyboard::GetKey(int keyCode) const
{
	return m_isKeyDown[keyCode];
}


//-----------------------------------------------------------------------------------------------
bool TheKeyboard::GetKeyDown(int keyCode) const
{
	return m_wasKeyJustPressed[keyCode];
}


//-----------------------------------------------------------------------------------------------
bool TheKeyboard::GetKeyUp(int keyCode) const
{
	return m_wasKeyJustReleased[keyCode];
}


//-----------------------------------------------------------------------------------------------
float TheKeyboard::GetHorizontalAxisBinary() const
{
	if (GetKey(KB_RIGHT) || GetKey('D'))
	{
		return 1.f;
	}
	else if (GetKey(KB_LEFT) || GetKey('A'))
	{
		return -1.f;
	}
	else
	{
		return 0.f;
	}
}


//-----------------------------------------------------------------------------------------------
float TheKeyboard::GetVerticalAxisBinary() const
{
	if (GetKey(KB_UP) || GetKey('W'))
	{
		return 1.f;
	}
	else if (GetKey(KB_DOWN) || GetKey('S'))
	{
		return -1.f;
	}
	else
	{
		return 0.f;
	}
}


//-----------------------------------------------------------------------------------------------
void TheKeyboard::SetKeyStatus(int keyCode, bool wasPressed)
{
	bool isChanging = m_isKeyDown[keyCode] != wasPressed;
	m_isKeyDown[keyCode] = wasPressed;

	if (wasPressed && isChanging)
	{
		m_wasKeyJustPressed[keyCode] = true;
	}
	else if(!wasPressed && isChanging)
	{
		m_wasKeyJustReleased[keyCode] = true;
	}
}


//-----------------------------------------------------------------------------------------------
void TheKeyboard::Update()
{
	for (int i = 0; i < NUM_KEYS; i++)
	{
		m_wasKeyJustPressed[i] = false;
		m_wasKeyJustReleased[i] = false;
	}
}


//-----------------------------------------------------------------------------------------------
void TheKeyboard::InitializeArrays()
{
	for (int i = 0; i < NUM_KEYS; i++)
	{
		m_isKeyDown[i] = false;
		m_wasKeyJustPressed[i] = false;
		m_wasKeyJustReleased[i] = false;
	}
}