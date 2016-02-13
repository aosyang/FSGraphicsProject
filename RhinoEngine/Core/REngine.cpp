//=============================================================================
// REngine.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#include "Rhino.h"

#include "REngine.h"

REngine* gEngine = nullptr;

static TCHAR szWindowClass[] = _T("rhinoapp");

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

REngine::REngine()
{
	m_hInst = NULL;
	m_hWnd = NULL;
	m_bFullScreen = false;
	m_Application = NULL;
}


REngine::~REngine()
{
}

bool REngine::Initialize()
{
	gEngine = this;

	SetProcessDPIAware();

	int width = 1024,
		height = 768;

	if (!CreateRenderWindow(width, height))
		return false;

	if (!RInput.Initialize())
		return false;

	if (!RRenderer.Initialize(m_hWnd, width, height, true))
		return false;

	if (!m_Application->Initialize())
		return false;

	return true;
}

void REngine::Shutdown()
{
	//delete m_Application;
	RRenderer.Shutdown();
	DestroyRenderWindow();
	gEngine = nullptr;
}

void REngine::Run()
{
	// Main message loop
	// Note: http://www.directxtutorial.com/Lesson.aspx?lessonid=9-1-4
	//		 This link shows why we should use PeekMessage instead of GetMessage here.

	//MSG msg;
	//while (GetMessage(&msg, NULL, 0, 0))
	//{
	//	TranslateMessage(&msg);
	//	DispatchMessage(&msg);
	//}

	bool bRunLoop = true;
	m_Timer.Reset();

	// Main game loop
	while (bRunLoop)
	{
		MSG msg;

		RInput._UpdateKeyStates();

		// Handle win32 window messages
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				bRunLoop = false;
				break;
			}
			else
			{
				// Translate the message and dispatch it to WndProc()
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		m_Timer.Tick();

		m_Application->UpdateScene(m_Timer);
		m_Application->RenderScene();

		CalculateFrameStats();

		// Press ESC to quit loop
		if (RInput.IsKeyDown(VK_ESCAPE))
			bRunLoop = false;
	}
}

void REngine::ResizeApp(int width, int height)
{
	m_Application->OnResize(width, height);
}

bool REngine::CreateRenderWindow(int width, int height, bool fullscreen, int bpp)
{
	m_hInst = GetModuleHandle(NULL);
	m_bFullScreen = fullscreen;

	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(WNDCLASSEX));

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = m_hInst;
	wcex.hIcon = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_APPLICATION));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL,
			_T("Call to RegisterClassEx failed!"),
			_T("Rhino Engine"),
			NULL);

		return false;
	}

	// Window style: WS_POPUP				- No caption, no maximize/minumize buttons
	//				 WS_OVERLAPPEDWINDOW	- Normal window
	DWORD dwStyle = m_bFullScreen ? WS_POPUP : WS_OVERLAPPEDWINDOW;

	// Adjust window size according to window style.
	// This will make sure correct client area.
	RECT win_rect = { 0, 0, width, height };
	AdjustWindowRect(&win_rect, dwStyle, false);

	int pos_x, pos_y;

	if (m_bFullScreen)
	{
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize			= sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth	= (unsigned long)width;
		dmScreenSettings.dmPelsHeight	= (unsigned long)height;
		dmScreenSettings.dmBitsPerPel	= bpp;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Change the display settings to full screen.
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		pos_x = pos_y = 0;
	}
	else
	{
		pos_x = (GetSystemMetrics(SM_CXSCREEN) - (win_rect.right - win_rect.left)) / 2;
		pos_y = (GetSystemMetrics(SM_CYSCREEN) - (win_rect.bottom - win_rect.top)) / 2;
	}

	// Create window and validate
	m_hWnd = CreateWindow(
		szWindowClass,
		m_Application->WindowTitle(),
		dwStyle,
		pos_x, pos_y,
		win_rect.right - win_rect.left,
		win_rect.bottom - win_rect.top,
		NULL,
		NULL,
		m_hInst,
		NULL);

	if (!m_hWnd)
	{
		MessageBox(NULL,
			_T("Call to CreateWindow failed!"),
			_T("Rhino Engine"),
			NULL);

		return false;
	}

	ShowWindow(m_hWnd, SW_SHOW);
	SetForegroundWindow(m_hWnd);
	SetFocus(m_hWnd);

	return true;
}

void REngine::DestroyRenderWindow()
{
	if (m_bFullScreen)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	DestroyWindow(m_hWnd);
	m_hWnd = NULL;
	UnregisterClass(szWindowClass, m_hInst);
	m_hInst = NULL;
}

void REngine::CalculateFrameStats()
{
	// Code computes the average frames per second, and also the 
	// average time it takes to render one frame.  These stats 
	// are appended to the window caption bar.

	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	// Compute averages over one second period.
	if ((m_Timer.TotalTime() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;

		std::wostringstream outs;
		outs.precision(6);
		outs << m_Application->WindowTitle()
			<< L" - " << RRenderer.GetAdapterName() << L"    "
			<< L"FPS: " << fps << L"    "
			<< L"Frame Time: " << mspf << L" (ms)";
		SetWindowText(m_hWnd, outs.str().c_str());

		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
	case WM_CLOSE:
		PostQuitMessage(0);
		break;

	case WM_SIZE:
		RRenderer.ResizeClient(LOWORD(lParam), HIWORD(lParam));
		if (gEngine) gEngine->ResizeApp(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_KEYDOWN:
		RInput._SetKeyDown(wParam, true);
		break;

	case WM_KEYUP:
		RInput._SetKeyDown(wParam, false);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}
