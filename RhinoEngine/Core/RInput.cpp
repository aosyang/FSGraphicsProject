//=============================================================================
// RInput.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "RInput.h"
#include <Windows.h>


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

	::GetPhysicalCursorPos(&m_CursorPosLastFrame);
	::GetPhysicalCursorPos(&m_CursorPos);

	return true;
}

void RInputSystem::Shutdown()
{
	KeyBindingDelegates.clear();
}

void RInputSystem::UnbindKeyStateEvents(int KeyCode, EBufferedKeyState KeyState)
{
	for (int Index = (int)KeyBindingDelegates.size() - 1; Index >= 0; Index--)
	{
		auto& KeyBindingDelegate = KeyBindingDelegates[Index];
		if (KeyBindingDelegate->KeyCode == KeyCode && KeyBindingDelegate->KeyState == KeyState)
		{
			KeyBindingDelegates.erase(KeyBindingDelegates.begin() + Index);
		}
	}
}

void RInputSystem::UnbindAllKeyEvents(int KeyCode)
{
	for (int Index = (int)KeyBindingDelegates.size() - 1; Index >= 0; Index--)
	{
		auto& KeyBindingDelegate = KeyBindingDelegates[Index];
		if (KeyBindingDelegate->KeyCode == KeyCode)
		{
			KeyBindingDelegates.erase(KeyBindingDelegates.begin() + Index);
		}
	}
}

void RInputSystem::CheckAndExecuteKeyBindings()
{
	for (auto& KeyBindingDelegate : KeyBindingDelegates)
	{
		if (GetBufferedKeyState(KeyBindingDelegate->KeyCode) == KeyBindingDelegate->KeyState)
		{
			KeyBindingDelegate->Execute();
		}
	}
}

void RInputSystem::_UpdateKeyStates(HWND hWnd)
{
	// Update keyboard key states
	for (int i = 0; i < MAX_KEY_NUM; i++)
	{
		m_bKeyDownLastFrame[i] = m_bKeyDown[i];
	}

	// Update mouse button states
	for (int i = VK_LBUTTON; i <= VK_MBUTTON; i++)
	{
		m_bKeyDown[i] = (GetKeyState(i) & 0x80) != 0;
	}

	// Update system key states
	for (int i = VK_LSHIFT; i <= VK_RMENU; i++)
	{
		m_bKeyDown[i] = (GetKeyState(i) & 0x80) != 0;
	}

	m_CursorPosLastFrame = m_CursorPos;
	::GetPhysicalCursorPos(&m_CursorPos);

	// Convert cursor point to client
	m_CursorClientPos = m_CursorPos;
	::ScreenToClient(hWnd, &m_CursorClientPos);

	if (m_bCursorLocked)
	{
		::SetPhysicalCursorPos(m_CursorLockingPos.x, m_CursorLockingPos.y);
		m_CursorPosLastFrame = m_CursorLockingPos;
	}
}

void RInputSystem::_ClearKeyStates()
{
	// Update keyboard key states
	for (int i = 0; i < MAX_KEY_NUM; i++)
	{
		m_bKeyDown[i] = false;
		m_bKeyDownLastFrame[i] = false;
	}

	// Update mouse button states
	for (int i = VK_LBUTTON; i <= VK_MBUTTON; i++)
	{
		m_bKeyDown[i] = false;
		m_bKeyDownLastFrame[i] = false;
	}

	// Update system key states
	for (int i = VK_LSHIFT; i <= VK_RMENU; i++)
	{
		m_bKeyDown[i] = false;
		m_bKeyDownLastFrame[i] = false;
	}
}

void RInputSystem::LockCursor()
{
	if (!m_bCursorLocked)
	{
		::GetPhysicalCursorPos(&m_CursorLockingPos);

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

void RInputSystem::GetCursorScreenPosition(int& x, int& y) const
{
	x = m_CursorPos.x;
	y = m_CursorPos.y;
}

void RInputSystem::GetCursorClientPosition(int& x, int& y) const
{
	x = m_CursorClientPos.x;
	y = m_CursorClientPos.y;
}

void RInputSystem::GetRelativeCursorPosition(int& dx, int& dy) const
{
	dx = m_CursorPos.x - m_CursorPosLastFrame.x;
	dy = m_CursorPos.y - m_CursorPosLastFrame.y;
}

EBufferedKeyState RInputSystem::GetBufferedKeyState(int KeyCode) const
{
	if (m_bKeyDown[KeyCode])
	{
		return m_bKeyDownLastFrame[KeyCode] ? EBufferedKeyState::KeyDown : EBufferedKeyState::Pressed;
	}
	else
	{
		return m_bKeyDownLastFrame[KeyCode] ? EBufferedKeyState::Released : EBufferedKeyState::KeyUp;
	}
}

bool RInputSystem::IsKeyDown(int KeyCode) const
{
	return m_bKeyDown[KeyCode];
}

void RInputSystem::SetKeyDownState(int KeyCode, bool bIsKeyDown)
{
	m_bKeyDown[KeyCode] = bIsKeyDown;
}

void RKeyStateModifier::NotifyKeyDownStateChanged(int KeyCode, bool bIsKeyDown)
{
	RInput.SetKeyDownState(KeyCode, bIsKeyDown);
}
