// EngineManagedWrapper.h

#pragma once

using namespace System;
using namespace System::Collections::Generic;
using namespace ManagedInterface;

class REngine;

namespace EngineManagedWrapper {

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
		virtual void OnKeyDown(int keycode);
		virtual void OnKeyUp(int keycode);
		void RunScreenToCameraRayPicking(float x, float y);
		
		virtual IManagedSceneObject^ GetSelection();

		virtual void DeleteSelection();
		virtual void LoadScene(String^ filename);
		virtual void SaveScene(String^ filename);

		virtual void SaveMeshMaterialFromSelection();
		virtual void ExportAllAnimationsToBinaryFiles();
	};
}
