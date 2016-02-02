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
	friend LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
public:
	bool Initialize();
	void UpdateKeyStates();

	RInput_BufferedKeyState GetBufferedKeyState(int keycode);
	bool IsKeyDown(int keycode);

protected:
	RInputSystem();
	~RInputSystem();

	void _SetKeyDown(int keycode, bool keydown);

private:
	bool	m_bKeyDown[MAX_KEY_NUM];
	bool	m_bKeyDownLastFrame[MAX_KEY_NUM];
};

#define RInput RInputSystem::Instance()

#endif
