//=============================================================================
// RInput.h by Shiyang Ao, 2016 All Rights Reserved.
//
// Input manager class
//=============================================================================
#ifndef _RINPUT_H
#define _RINPUT_H

#include "RSingleton.h"

class RKeyStateModifier;

enum class EBufferedKeyState : UINT8
{
	KeyDown,
	KeyUp,
	Pressed,
	Released,
};

const int MAX_KEY_NUM = 0xFF;

// The input system of the engine
class RInputSystem : public RSingleton<RInputSystem>
{
	friend class RSingleton<RInputSystem>;
	friend class REngine;
	friend class RKeyStateModifier;
	friend LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
public:
	// Initialize the input system
	bool Initialize();

	// Freeze the position of mouse cursor so it can't move
	void LockCursor();

	// Unfreeze the position of mouse cursor
	void UnlockCursor();

	void ShowCursor();
	void HideCursor();

	void GetCursorPosition(int& x, int& y) const;
	void GetRelativeCursorPosition(int& dx, int& dy) const;

	EBufferedKeyState GetBufferedKeyState(int KeyCode) const;
	bool IsKeyDown(int KeyCode) const;

protected:
	RInputSystem();
	~RInputSystem();

	// Update input device states
	//   - Called by REngine once per frame
	void _UpdateKeyStates();

	// Set a new 'key down' state for a key
	// Called by low-level key event handlers
	void SetKeyDownState(int KeyCode, bool bIsKeyDown);

private:
	bool	m_bKeyDown[MAX_KEY_NUM];
	bool	m_bKeyDownLastFrame[MAX_KEY_NUM];

	POINT	m_CursorPos;
	POINT	m_CursorPosLastFrame;

	bool	m_bCursorLocked;
	POINT	m_CursorLockingPos;
};

// A utility class to modify 'key down' state of the input system
class RKeyStateModifier
{
public:
	// Modify 'key down' state inside the input system
	void NotifyKeyDownStateChanged(int KeyCode, bool bIsKeyDown);
};

#define RInput RInputSystem::Instance()

#endif
