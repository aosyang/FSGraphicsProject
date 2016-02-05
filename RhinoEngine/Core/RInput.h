//=============================================================================
// RInput.h by Shiyang Ao, 2016 All Rights Reserved.
//
// Input manager class
//=============================================================================
#ifndef _RINPUT_H
#define _RINPUT_H

#include "RSingleton.h"

enum RInput_BufferedKeyState
{
	BKS_KeyUp,
	BKS_KeyDown,
	BKS_Pressed,
	BKS_Released,
};

#define MAX_KEY_NUM 0xFF

class RInputSystem : public RSingleton<RInputSystem>
{
	friend class RSingleton<RInputSystem>;
	friend class REngine;
	friend LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
public:
	// Initialize the input system
	bool Initialize();

	// Freeze cursor position
	void LockCursor();

	// Unfreeze cursor position
	void UnlockCursor();

	void ShowCursor();
	void HideCursor();

	void GetCursorPos(int& x, int& y);
	void GetCursorRelPos(int& dx, int& dy);

	RInput_BufferedKeyState GetBufferedKeyState(int keycode);
	bool IsKeyDown(int keycode);

protected:
	RInputSystem();
	~RInputSystem();

	// Update input device states
	//   - Called by REngine once per frame
	void _UpdateKeyStates();

	void _SetKeyDown(int keycode, bool keydown);

private:
	bool	m_bKeyDown[MAX_KEY_NUM];
	bool	m_bKeyDownLastFrame[MAX_KEY_NUM];

	POINT	m_CursorPos;
	POINT	m_CursorPosLastFrame;

	bool	m_bCursorLocked;
	POINT	m_CursorLockingPos;
};

#define RInput RInputSystem::Instance()

#endif
