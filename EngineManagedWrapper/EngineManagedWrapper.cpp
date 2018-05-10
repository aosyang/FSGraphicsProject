// This is the main DLL file.

#include "stdafx.h"

#include "EngineManagedWrapper.h"
#include "EditorApp.h"
#include "ManagedSceneObject.h"

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
		m_Application = new EditorApp();
		GEngine.BindApp(m_Application);
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

		m_IsInitialized = GEngine.Initialize((HWND)hWnd.ToPointer(), width, height);
		return m_IsInitialized;
	}

	void RhinoEngineWrapper::RunOneFrame()
	{
		if (m_IsInitialized)
			GEngine.RunOneFrame(true);
	}

	void RhinoEngineWrapper::Shutdown()
	{
		if (m_IsInitialized)
		{
			GEngine.Shutdown();

			delete m_Application;
		}
	}

	void RhinoEngineWrapper::Resize(int width, int height)
	{
		if (m_IsInitialized)
			GEngine.ResizeClientWindow(width, height);
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

	void RhinoEngineWrapper::UpdatePreviewMesh(String^ path, bool replace)
	{
		if (!replace)
			m_Application->AddMeshObjectToScene(ManagedStringRefToConstCharPtr(path));
		else
		{
			RSMeshObject* meshObj = static_cast<RSMeshObject*>(m_Application->GetSelection());
			if (meshObj)
			{
				meshObj->SetMesh(RResourceManager::Instance().FindMesh(ManagedStringRefToConstCharPtr(path)));
			}
		}
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

	EngineManagedWrapper::ManagedSceneObject^ RhinoEngineWrapper::GetSelection()
	{
		RSceneObject* sel = m_Application->GetSelection();
		if (sel)
		{
			switch (sel->GetType())
			{
			case ESceneObjectType::MeshObject:
				return gcnew EngineManagedWrapper::ManagedMeshObject(sel);
				break;

			default:
				break;
			}
		}

		return gcnew EngineManagedWrapper::ManagedSceneObject(sel);
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

	void RhinoEngineWrapper::SaveMeshMaterialFromSelection()
	{
		m_Application->SaveMeshMaterialFromSelection();
	}

	void RhinoEngineWrapper::ExportAllAnimationsToBinaryFiles()
	{
		m_Application->ExportAllAnimationsToBinaryFiles();
	}
}
