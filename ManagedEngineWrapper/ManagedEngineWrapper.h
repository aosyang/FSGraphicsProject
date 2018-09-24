//=============================================================================
// ManagedEngineWrapper.h by Shiyang Ao, 2018 All Rights Reserved.
//
//
//=============================================================================

#pragma once

using namespace System;
using namespace System::Collections::Generic;
using namespace ManagedInterface;

class REngine;

namespace ManagedEngineWrapper {

	class EditorApp;
	ref class ManagedSceneObject;

	public ref class RhinoEngineWrapper : public IManagedEngine
	{
	private:
		EditorApp*	m_Application;
		bool		m_IsInitialized;

	public:
		RhinoEngineWrapper();
		~RhinoEngineWrapper();		// destructor
		!RhinoEngineWrapper();		// finalizer

		virtual bool Initialize(IntPtr hWnd);
		virtual void RunOneFrame();
		virtual void Shutdown();
		virtual void Resize(int width, int height);

		virtual List<String^>^ GetMeshNameList();
		virtual void UpdatePreviewMesh(String^ path, bool replace);

		virtual List<IManagedSceneObject^>^ GetSceneObjectsList();

		virtual void OnKeyDown(int keycode);
		virtual void OnKeyUp(int keycode);
		void RunScreenToCameraRayPicking(float x, float y);
		
		virtual IManagedSceneObject^ GetSelection();
		virtual void SetSelection(IManagedSceneObject^ SelectedSceneObject);

		virtual bool DeleteSelection();
		virtual void LoadScene(String^ filename);
		virtual void SaveScene(String^ filename);

		virtual void SaveMeshMaterialFromSelection();
		virtual void ExportAllAnimationsToBinaryFiles();

	private:
		void UpdateSceneObjectsList();

		List<IManagedSceneObject^>^ SceneObjectsList;
	};
}
