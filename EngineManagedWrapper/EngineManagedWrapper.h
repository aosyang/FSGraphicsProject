// EngineManagedWrapper.h

#pragma once

using namespace System;
using namespace System::Collections::Generic;

class REngine;

namespace EngineManagedWrapper {

	class EditorApp;
	ref class ManagedSceneObject;

	public ref class RhinoEngineWrapper
	{
	private:
		REngine*	m_Engine;
		EditorApp*	m_Application;
		bool		m_IsInitialized;

	public:
		RhinoEngineWrapper();
		~RhinoEngineWrapper();
		!RhinoEngineWrapper();

		bool Initialize(IntPtr hWnd);
		void RunOneFrame();
		void Shutdown();
		void Resize(int width, int height);

		List<String^>^ GetMeshNameList();
		void UpdatePreviewMesh(String^ path, bool replace);
		void OnKeyDown(int keycode);
		void OnKeyUp(int keycode);
		void RunScreenToCameraRayPicking(float x, float y);
		
		ManagedSceneObject^ GetSelection();

		void DeleteSelection();
		void LoadScene(String^ filename);
		void SaveScene(String^ filename);

		void SaveMeshMaterialFromSelection();
		void ExportAllAnimationsToBinaryFiles();
	};
}
