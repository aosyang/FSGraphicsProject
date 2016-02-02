//=============================================================================
// RInput.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#include "Rhino.h"

#include "RInput.h"


RInputSystem::RInputSystem()
{
}


RInputSystem::~RInputSystem()
{
}

bool RInputSystem::Initialize()
{
	for (int i = 0; i < MAX_KEY_NUM; i++)
	{
		m_bKeyDown[i] = false;
		m_bKeyDownLastFrame[i] = false;
	}

	return true;
}

void RInputSystem::UpdateKeyStates()
{
	for (int i = 0; i < MAX_KEY_NUM; i++)
	{
		m_bKeyDownLastFrame[i] = m_bKeyDown[i];
	}
}

RInput_BufferedKeyState RInputSystem::GetBufferedKeyState(int keycode)
{
	if (m_bKeyDown[keycode])
	{
		return m_bKeyDownLastFrame[keycode] ? BKS_KeyDown : BKS_Pressed;
	}
	else
	{
		return m_bKeyDownLastFrame[keycode] ? BKS_Released : BKS_KeyUp;
	}
}

bool RInputSystem::IsKeyDown(int keycode)
{
	return m_bKeyDown[keycode];
}

void RInputSystem::_SetKeyDown(int keycode, bool keydown)
{
	m_bKeyDown[keycode] = keydown;
}
