//=============================================================================
// REngine.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "REngine.h"
#include "Scene/RSceneManager.h"
#include "Scene/RSceneComponentFactory.h"

#include "AI/NavigationSystem/RNavigationSystem.h"
#include "Physics/RPhysicsEngine.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

#include "Animation/RAnimGraph.h"

#include "RenderSystem/RRenderSystem.h"
#include "RenderSystem/RShaderManager.h"
#include "RenderSystem/RDebugRenderer.h"
#include "RenderSystem/RPostProcessorManager.h"
#include "Resource/RResourceManager.h"
#include "RScriptSystem.h"
#include "RInput.h"
#include "IApp.h"


static TCHAR szWindowClass[] = _T("rhinoapp");

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

REngine::REngine()
	: m_bIsEditor					(false),
	  m_bIsInitialized				(false),
	  m_hInst						(nullptr),
	  m_hWnd						(nullptr),
	  m_bFullScreen					(false),
	  m_UseEngineRenderWindow		(false),
	  m_Application					(nullptr),
	  FrameCounter(1)
{
	SetProcessDPIAware();
}


REngine::~REngine()
{
}

bool REngine::Initialize(const REngineInitParam& InitParam /*= REngineInitParam()*/)
{
	m_Application = InitParam.Application;

	RegisterEngineTypes();

	m_bIsInitialized = true;

	int width = 1024,
		height = 768;

	CreateImGuiContext();

	if (!CreateRenderWindow(width, height))
		return false;

	return InitializeSubsystems(m_hWnd, width, height);
}

bool REngine::InitializeSubsystems(HWND hWnd, int width, int height)
{
	m_bIsInitialized = true;

	if (!m_hWnd)
		m_hWnd = hWnd;

	if (!RInput.Initialize())
	{
		return false;
	}

	if (!GRenderer.Initialize(m_hWnd, width, height, true))
	{
		return false;
	}

	InitImGuiWindowAndDevice(m_hWnd);

	if (!GPhysicsEngine.Initialize())
	{
		return false;
	}

	// Initialize resource manager
	RResourceManager::Instance().Initialize();

	// Initialize shaders
	GShaderManager.LoadShaders(RShaderManager::GetShaderRootPath());

	// Initialize debug renderer
	GDebugRenderer.Initialize();

	GScriptSystem.Initialize();
	GScriptSystem.Start();

	GPostProcessorManager.Initialize();
	GSceneManager.Initialize();

	GNavigationSystem.Initialize();

	if (!m_Application->Initialize())
		return false;

	return true;
}

void REngine::Shutdown()
{
	GSceneManager.Shutdown();
	GPostProcessorManager.Release();

	GScriptSystem.Shutdown();

	// Unload shaders
	GShaderManager.UnloadAllShaders();

	// Destroy resource manager
	RResourceManager::Instance().Destroy();

	GPhysicsEngine.Shutdown();

	ShutdownImGui();

	GRenderer.Shutdown();
	RInput.Shutdown();

	if (m_UseEngineRenderWindow)
		DestroyRenderWindow();

	m_bIsInitialized = false;
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
		ImGuiIO& io = ImGui::GetIO();
		bool bKeyCapturedByGui = (io.WantCaptureKeyboard || io.WantCaptureMouse);
		if (!bKeyCapturedByGui)
		{
			RInput._UpdateKeyStates(m_hWnd);
		}

		// Handle win32 window messages
		MSG msg;
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

		if (bKeyCapturedByGui)
		{
			// Key states are changed during window message callback.
			// If GUI is capturing keyboard/mouse event, let's clear any key states for this frame.
			RInput._ClearKeyStates();
		}

		RunOneFrame();

		CalculateFrameStats();

		// Press ESC to quit loop
		if (RInput.IsKeyDown(VK_ESCAPE))
			bRunLoop = false;

		FrameCounter++;
	}
}

void REngine::RunOneFrame(bool update_input)
{
	// Update the resource manager
	RResourceManager::Instance().Update();

	if (update_input)
	{
		RInput._UpdateKeyStates(m_hWnd);
	}

	RInput.CheckAndExecuteKeyBindings();

	m_Timer.Tick();

	BeginImGuiFrame();

	m_Application->UpdateScene(m_Timer);

	const float DeltaTime = m_Timer.DeltaTime();

	// Update all registered scenes with their objects
	GSceneManager.Update(DeltaTime);
	GPhysicsEngine.Simulate(DeltaTime);
	GSceneManager.Update_PostPhysics(DeltaTime);

	if (!m_bIsEditor)
	{
		GScriptSystem.UpdateScriptableObjects();
	}

	GRenderer.Stats.Reset();
	if (m_Application->UsingCustomRenderPipeline())
	{
		m_Application->RenderScene();
	}
	else
	{
		GRenderer.RenderFrame();
	}

	EndImGuiFrame();
	GRenderer.Present();
}

void REngine::ResizeClientWindow(int width, int height)
{
	// Resize ImGui display
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2((float)width, (float)height);

	GRenderer.ResizeClient(width, height);
	GPostProcessorManager.RecreateLostResources();
	m_Application->OnResize(width, height);
}

RECT REngine::GetWindowRectInfo() const
{
	RECT rect;
	GetWindowRect(m_hWnd, &rect);

	return rect;
}

RECT REngine::GetClientRectInfo() const
{
	RECT rect;
	GetClientRect(m_hWnd, &rect);

	return rect;
}

void REngine::RegisterEngineTypes()
{
	RegisterEngineComponentClasses();

	RAnimGraph::RegisterAnimNodeTypes();
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
	wcex.hIcon = LoadIcon(m_hInst, IDI_APPLICATION);
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

	m_UseEngineRenderWindow = true;

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
			<< L" - " << GRenderer.GetAdapterName() << L"    "
			<< L"FPS: " << fps << L"    "
			<< L"Frame Time: " << mspf << L" (ms)";
		SetWindowText(m_hWnd, outs.str().c_str());

		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

void REngine::CreateImGuiContext()
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	unsigned char* tex_pixels = NULL;
	int tex_w, tex_h;
	io.Fonts->GetTexDataAsRGBA32(&tex_pixels, &tex_w, &tex_h);
}

void REngine::InitImGuiWindowAndDevice(HWND hWnd)
{
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(GRenderer.D3DDevice(), GRenderer.D3DImmediateContext());
}

void REngine::ShutdownImGui()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void REngine::BeginImGuiFrame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void REngine::EndImGuiFrame()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Handling ImGui events
	extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
	{
		return true;
	}

	switch (message)
	{
	case WM_DESTROY:
	case WM_CLOSE:
		PostQuitMessage(0);
		break;

	case WM_SIZE:
		if (GEngine.IsInitialized())
		{
			GEngine.ResizeClientWindow(LOWORD(lParam), HIWORD(lParam));
		}
		break;

	case WM_KEYDOWN:
		RInput.SetKeyDownState((int)wParam, true);
		break;

	case WM_KEYUP:
		RInput.SetKeyDownState((int)wParam, false);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}
