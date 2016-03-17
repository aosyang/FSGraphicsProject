//=============================================================================
// REngine.h by Shiyang Ao, 2016 All Rights Reserved.
//
// Engine class
//=============================================================================
#ifndef _RENGINE_H
#define _RENGINE_H

class REngine
{
public:
	REngine();
	~REngine();

	// Initialize all engine components
	bool Initialize();

	bool Initialize(HWND hWnd, int width, int height);

	// Specify the application for the engine to run
	inline void BindApp(IApp* app) { m_Application = app; }

	// Shutdown the engine and destroy all engine components
	void Shutdown();

	// Engine main loop
	void Run();

	void RunOneFrame();

	void ResizeClientWindow(int width, int height);

	static RTimer& GetTimer() { return m_Timer; }
private:
	bool CreateRenderWindow(int width, int height, bool fullscreen=false, int bpp=32);
	void DestroyRenderWindow();
	void CalculateFrameStats();

	HINSTANCE			m_hInst;
	HWND				m_hWnd;
	bool				m_bFullScreen;
	bool				m_UseEngineRenderWindow;
	IApp*				m_Application;
	static RTimer		m_Timer;
};

#endif
