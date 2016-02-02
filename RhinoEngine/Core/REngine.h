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

	// Specify the application for the engine to run
	template<typename T>
	inline void BindApp() { SAFE_DELETE(m_Application); m_Application = new T(); }

	// Shutdown the engine and destroy all engine components
	void Shutdown();

	// Engine main loop
	void Run();

	void ResizeApp(int width, int height);
private:
	bool CreateRenderWindow(int width, int height, bool fullscreen=false, int bpp=32);
	void DestroyRenderWindow();
	void CalculateFrameStats();

	HINSTANCE	m_hInst;
	HWND		m_hWnd;
	bool		m_bFullScreen;
	IApp*		m_Application;
	RTimer		m_Timer;
};

#endif
