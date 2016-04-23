// This is the main DLL file.

#include "stdafx.h"
#include "Rhino.h"

#include "EngineManagedWrapper.h"
#include "EditorApp.h"

#pragma comment(lib, "User32.lib")

#include <direct.h>

using namespace System::Runtime::InteropServices;

namespace EngineManagedWrapper
{
	const char* ManagedStringRefToConstCharPtr(String^ str)
	{
		IntPtr pNativeStr = Marshal::StringToHGlobalAnsi(str);
		return static_cast<const char*>(pNativeStr.ToPointer());
	}

	RhinoEngineWrapper::RhinoEngineWrapper()
	{
		m_IsInitialized = false;
		m_Engine = new REngine();
		m_Application = new EditorApp();
		m_Engine->BindApp(m_Application);
	}

	RhinoEngineWrapper::~RhinoEngineWrapper()
	{
		this->!RhinoEngineWrapper();
	}

	RhinoEngineWrapper::!RhinoEngineWrapper()
	{
	}

	bool RhinoEngineWrapper::Initialize(IntPtr hWnd)
	{
		RECT rect;
		GetClientRect((HWND)hWnd.ToPointer(), &rect);

		int width = rect.right - rect.left;
		int height = rect.bottom - rect.top;

		m_IsInitialized = m_Engine->Initialize((HWND)hWnd.ToPointer(), width, height);
		return m_IsInitialized;
	}

	void RhinoEngineWrapper::RunOneFrame()
	{
		if (m_IsInitialized)
			m_Engine->RunOneFrame(true);
	}

	void RhinoEngineWrapper::Shutdown()
	{
		if (m_IsInitialized)
		{
			m_Engine->Shutdown();

			delete m_Application;
			delete m_Engine;
		}
	}

	void RhinoEngineWrapper::Resize(int width, int height)
	{
		if (m_IsInitialized)
			m_Engine->ResizeClientWindow(width, height);
	}

	List<String^>^ RhinoEngineWrapper::GetMeshNameList()
	{
		List<String^>^ list = gcnew List<String^>();

		const vector<RMesh*>& meshList = RResourceManager::Instance().GetMeshResources();
		for (vector<RMesh*>::const_iterator iter = meshList.begin(); iter != meshList.end(); iter++)
		{
			list->Add(gcnew String((*iter)->GetPath().data()));
		}

		return list;
	}

	void RhinoEngineWrapper::UpdatePreviewMesh(String^ path)
	{
		m_Application->AddMeshObjectToScene(ManagedStringRefToConstCharPtr(path));
	}

	void RhinoEngineWrapper::OnKeyDown(int keycode)
	{
		RInput._SetKeyDown(keycode, true);
	}

	void RhinoEngineWrapper::OnKeyUp(int keycode)
	{
		RInput._SetKeyDown(keycode, false);
	}

	void RhinoEngineWrapper::RunScreenToCameraRayPicking(float x, float y)
	{
		//m_Application->RunScreenToCameraRayPicking(x, y);
	}

	void RhinoEngineWrapper::DeleteSelection()
	{
		m_Application->DeleteSelection();
	}

	void RhinoEngineWrapper::LoadScene(String^ filename)
	{
		m_Application->LoadScene(ManagedStringRefToConstCharPtr(filename));
	}

	void RhinoEngineWrapper::SaveScene(String^ filename)
	{
		m_Application->SaveScene(ManagedStringRefToConstCharPtr(filename));
	}
}
