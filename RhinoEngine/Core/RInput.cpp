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

	m_bCursorLocked = false;

	::GetCursorPos(&m_CursorPosLastFrame);
	::GetCursorPos(&m_CursorPos);

	return true;
}

void RInputSystem::UpdateKeyStates()
{
	for (int i = 0; i < MAX_KEY_NUM; i++)
	{
		m_bKeyDownLastFrame[i] = m_bKeyDown[i];
	}

	// Update mouse buttons
	for (int i = VK_LBUTTON; i <= VK_MBUTTON; i++)
	{
		m_bKeyDown[i] = (GetKeyState(i) & 0x80) != 0;
	}

	// Update system keys
	for (int i = VK_LSHIFT; i <= VK_RMENU; i++)
	{
		m_bKeyDown[i] = (GetKeyState(i) & 0x80) != 0;
	}

	m_CursorPosLastFrame = m_CursorPos;
	::GetCursorPos(&m_CursorPos);

	if (m_bCursorLocked)
	{
		::SetCursorPos(m_CursorLockingPos.x, m_CursorLockingPos.y);
		m_CursorPosLastFrame = m_CursorLockingPos;
	}
}

void RInputSystem::LockCursor()
{
	if (!m_bCursorLocked)
	{
		::GetCursorPos(&m_CursorLockingPos);

		m_bCursorLocked = true;
	}
}

void RInputSystem::UnlockCursor()
{
	if (m_bCursorLocked)
	{
		m_bCursorLocked = false;;
	}
}

void RInputSystem::ShowCursor()
{
	::ShowCursor(TRUE);
}

void RInputSystem::HideCursor()
{
	::ShowCursor(FALSE);
}

void RInputSystem::GetCursorPos(int& x, int& y)
{
	x = m_CursorPos.x;
	y = m_CursorPos.y;
}

void RInputSystem::GetCursorRelPos(int& dx, int& dy)
{
	dx = m_CursorPos.x - m_CursorPosLastFrame.x;
	dy = m_CursorPos.y - m_CursorPosLastFrame.y;
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
