//=============================================================================
// REngine.h by Shiyang Ao, 2016 All Rights Reserved.
//
// Engine class
//=============================================================================
#pragma once

class REngine : public RSingleton<REngine>
{
	friend class RSingleton<REngine>;
public:
	// Initialize all engine components
	bool Initialize();

	bool InitializeSubsystems(HWND hWnd, int width, int height);

	// Specify the application for the engine to run
	inline void BindApp(IApp* app) { m_Application = app; }

	// Shutdown the engine and destroy all engine components
	void Shutdown();

	// Engine main loop
	void Run();

	void RunOneFrame(bool update_input = false);

	void ResizeClientWindow(int width, int height);
	RECT GetWindowRectInfo() const;

	RTimer& GetTimer() { return m_Timer; }

	/// Has engine been initialized
	bool IsInitialized() const { return m_bIsInitialized; }

	void SetEditorMode(bool editor) { m_bIsEditor = editor; }
	bool IsEditor() const { return m_bIsEditor; }

	void SetUseCustomRenderingPipeline(bool useCustomRendering)		{ m_UseCustomRenderingPipeline = useCustomRendering; }
	bool IsUsingCustomRenderingPipeline() const						{ return m_UseCustomRenderingPipeline; }

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

	void BeginImGuiFrame();
	void EndImGuiFrame();

	bool				m_bIsEditor;

	bool				m_bIsInitialized;
	HINSTANCE			m_hInst;
	HWND				m_hWnd;
	bool				m_bFullScreen;
	bool				m_UseEngineRenderWindow;
	IApp*				m_Application;
	RTimer				m_Timer;

	/// Whether engine should call IApp::RenderScene
	bool				m_UseCustomRenderingPipeline;
};

#define GEngine REngine::Instance()

