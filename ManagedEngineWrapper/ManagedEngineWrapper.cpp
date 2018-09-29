//=============================================================================
// ManagedEngineWrapper.cpp by Shiyang Ao, 2018 All Rights Reserved.
//
// This is the main DLL file.
//=============================================================================

#include "stdafx.h"

#include "ManagedEngineWrapper.h"
#include "EditorApp.h"
#include "ManagedSceneObject.h"

#pragma comment(lib, "User32.lib")

#include <direct.h>

using namespace System::Runtime::InteropServices;

namespace ManagedEngineWrapper
{
	RKeyStateModifier g_KeyStateModifier;

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
		{
			GEngine.RunOneFrame(true);
		}
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
		for (auto Iter : meshList)
		{
			list->Add(gcnew String(Iter->GetPath().data()));
		}

		return list;
	}

	void RhinoEngineWrapper::UpdatePreviewMesh(String^ path, bool replace)
	{
		if (!replace)
		{
			m_Application->AddMeshObjectToScene(ManagedStringRefToConstCharPtr(path));
			UpdateSceneObjectsList();
		}
		else
		{
			RSMeshObject* meshObj = static_cast<RSMeshObject*>(m_Application->GetSelection());
			if (meshObj)
			{
				meshObj->SetMesh(RResourceManager::Instance().FindMesh(ManagedStringRefToConstCharPtr(path)));
			}
		}
	}

	List<IManagedSceneObject^>^ RhinoEngineWrapper::GetSceneObjectsList()
	{
		return SceneObjectsList;
	}

	void RhinoEngineWrapper::OnKeyDown(int keycode)
	{
		g_KeyStateModifier.NotifyKeyDownStateChanged(keycode, true);
	}

	void RhinoEngineWrapper::OnKeyUp(int keycode)
	{
		g_KeyStateModifier.NotifyKeyDownStateChanged(keycode, false);
	}

	void RhinoEngineWrapper::SetInputEnabled(bool bEnabled)
	{
		m_Application->SetInputEnabled(bEnabled);
	}

	void RhinoEngineWrapper::RunScreenToCameraRayPicking(float x, float y)
	{
		//m_Application->RunScreenToCameraRayPicking(x, y);
	}

	IManagedSceneObject^ RhinoEngineWrapper::GetSelection()
	{
		RSceneObject* sel = m_Application->GetSelection();
		if (sel)
		{
			if (!SceneObjectsList)
			{
				UpdateSceneObjectsList();
			}

			for (int i = 0; i < SceneObjectsList->Count; i++)
			{
				auto SceneObject = safe_cast<ManagedSceneObject^>(SceneObjectsList[i]);
				if (SceneObject->GetRawSceneObjectPtr() == sel)
				{
					return SceneObject;
				}
			}
		}

		return gcnew ManagedEngineWrapper::ManagedSceneObject(sel);
	}

	void RhinoEngineWrapper::SetSelection(IManagedSceneObject^ SelectedSceneObject)
	{
		if (SelectedSceneObject != nullptr)
		{
			ManagedSceneObject^ SceneObject = safe_cast<ManagedSceneObject^>(SelectedSceneObject);
			if (SceneObject != nullptr)
			{
				m_Application->SetSelection(SceneObject->GetRawSceneObjectPtr());
				return;
			}
		}

		m_Application->SetSelection(nullptr);
	}

	bool RhinoEngineWrapper::DeleteSelection()
	{
		return m_Application->DeleteSelection();
	}

	void RhinoEngineWrapper::LoadScene(String^ filename)
	{
		m_Application->LoadScene(ManagedStringRefToConstCharPtr(filename));
		UpdateSceneObjectsList();
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

	void RhinoEngineWrapper::UpdateSceneObjectsList()
	{
		SceneObjectsList = gcnew List<IManagedSceneObject ^>();

		for (auto SceneObject : m_Application->GetSceneObjects())
		{
			if (SceneObject->IsType<RSMeshObject>())
			{
				SceneObjectsList->Add(gcnew ManagedMeshObject(SceneObject));
			}
			else
			{
				SceneObjectsList->Add(gcnew ManagedSceneObject(SceneObject));
			}
		}
	}

}
