//=============================================================================
// REngine.h by Shiyang Ao, 2016 All Rights Reserved.
//
// Engine class
//=============================================================================
#pragma once

#include "RTimer.h"
#include "RSingleton.h"

#include <Windows.h>

class IApp;

struct REngineInitParam
{
	//REngineInitParam()
	//	: Application(nullptr)
	//{}

	IApp* Application;
};

class REngine : public RSingleton<REngine>
{
	friend class RSingleton<REngine>;
public:
	// Initialize all engine components
	bool Initialize(const REngineInitParam& InitParam);

	bool InitializeSubsystems(HWND hWnd, int width, int height);

	// Shutdown the engine and destroy all engine components
	void Shutdown();

	// Engine main loop
	void Run();

	void RunOneFrame(bool update_input = false);

	void ResizeClientWindow(int width, int height);
	RECT GetWindowRectInfo() const;
	RECT GetClientRectInfo() const;
	RTimer& GetTimer() { return m_Timer; }

	/// Get number of frames since engine started. The first frame starts from 1.
	UINT64 GetFrameCounter() const { return FrameCounter; }

	/// Has engine been initialized
	bool IsInitialized() const { return m_bIsInitialized; }

	void SetEditorMode(bool editor) { m_bIsEditor = editor; }
	bool IsEditor() const { return m_bIsEditor; }

	void BeginImGuiFrame();
	void EndImGuiFrame();

protected:
	REngine();
	~REngine();

private:
	bool CreateRenderWindow(int width, int height, bool fullscreen = false, int bpp = 32);
	void DestroyRenderWindow();
	void CalculateFrameStats();

	void CreateImGuiContext();
	void InitImGuiWindowAndDevice(HWND hWnd);
	void ShutdownImGui();

	bool				m_bIsEditor;

	bool				m_bIsInitialized;
	HINSTANCE			m_hInst;
	HWND				m_hWnd;
	bool				m_bFullScreen;
	bool				m_UseEngineRenderWindow;
	IApp*				m_Application;
	RTimer				m_Timer;
	UINT64				FrameCounter;
};

#define GEngine REngine::Instance()

