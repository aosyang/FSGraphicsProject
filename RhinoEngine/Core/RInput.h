//=============================================================================
// RInput.h by Shiyang Ao, 2016 All Rights Reserved.
//
// Input manager class
//=============================================================================
#pragma once

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

	// Shutdown the input system
	void Shutdown();

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

	// Bind a class member function as a key event
	template<typename T>
	void BindKeyStateEvent(int KeyCode, EBufferedKeyState KeyState, T* Object, void(T::*Func)());

	// Unbind all events from a key state
	void UnbindKeyStateEvents(int KeyCode, EBufferedKeyState KeyState);

	// Unbind all events from a key
	void UnbindAllKeyEvents(int KeyCode);

	// Check condition and execute all registered key binding delegates
	void CheckAndExecuteKeyBindings();

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
	// Is key down this frame
	bool	m_bKeyDown[MAX_KEY_NUM];

	// Is key down last frame
	bool	m_bKeyDownLastFrame[MAX_KEY_NUM];

	POINT	m_CursorPos;
	POINT	m_CursorPosLastFrame;

	bool	m_bCursorLocked;
	POINT	m_CursorLockingPos;

	struct RKeyBindingDelegate
	{
	public:
		RKeyBindingDelegate(int InKeyCode, EBufferedKeyState InKeyState)
			: KeyCode(InKeyCode)
			, KeyState(InKeyState)
		{}

		virtual void Execute() {}

		int					KeyCode;
		EBufferedKeyState	KeyState;
	};

	vector<unique_ptr<RKeyBindingDelegate>> KeyBindingDelegates;
};

template<typename T>
void RInputSystem::BindKeyStateEvent(int KeyCode, EBufferedKeyState KeyState, T* Object, void(T::*Func)())
{
	struct KeyEvent : public RKeyBindingDelegate
	{
	public:
		KeyEvent(int InKeyCode, EBufferedKeyState InKeyState, T* InObject, void(T::*InFunc)())
			: RKeyBindingDelegate(InKeyCode, InKeyState)
			, Object(InObject)
			, Func(InFunc)
		{
		}

		virtual void Execute() override { if (Object) { (Object->*Func)(); } }

	private:
		T*					Object;
		void(T::*Func)();
	};

	unique_ptr<KeyEvent> NewKeyEvent(new KeyEvent(KeyCode, KeyState, Object, Func));
	KeyBindingDelegates.push_back(move(NewKeyEvent));
}

// A utility class to modify 'key down' state of the input system
class RKeyStateModifier
{
public:
	// Modify 'key down' state inside the input system
	void NotifyKeyDownStateChanged(int KeyCode, bool bIsKeyDown);
};

#define RInput RInputSystem::Instance()

