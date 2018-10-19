//=============================================================================
// ManagedEngineWrapper.h by Shiyang Ao, 2018 All Rights Reserved.
//
//
//=============================================================================

#pragma once

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Drawing;
using namespace ManagedInterface;

class REngine;

namespace ManagedEngineWrapper {

	class EditorApp;
	ref class ManagedSceneObject;

	/// Wrapper delegate of async resource loaded callback
	delegate void AsyncResourceLoadedWrapper(const char* ResourceName);

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
		virtual bool IsMeshAssetReady(String^ MeshName);
		virtual Bitmap^ GenerateMeshThumbnailBitmap(String^ MeshName, int Width, int Height);
		virtual void AddMeshObjectToScene(String^ MeshAssetPath);
		virtual void ReplaceMeshAssetForSelection(String^ MeshAssetPath);

		virtual List<IManagedSceneObject^>^ GetSceneObjectsList();

		virtual void OnKeyDown(int keycode);
		virtual void OnKeyUp(int keycode);
		virtual void SetInputEnabled(bool bEnabled);

		void RunScreenToCameraRayPicking(float x, float y);
		
		virtual IManagedSceneObject^ GetSelection();
		virtual void SetSelection(IManagedSceneObject^ SelectedSceneObject);

		virtual bool DeleteSelection();
		virtual void LoadScene(String^ filename);
		virtual void SaveScene(String^ filename);

		virtual void SaveMeshMaterialFromSelection();
		virtual void ExportAllAnimationsToBinaryFiles();

		/// Wrapper function to convert resource name of OnAsyncResourceLoaded from native type to managed type
		void OnAsyncResourceLoaded(const char* ResourceName);

		/// Set the managed delegate for async resource loaded
		virtual void SetAsyncResourceLoadedHandler(ManagedInterface::AsyncResourceLoadedHandler^ AsyncResourceLoaded);

	private:
		void UpdateSceneObjectsList();
		
		/// Render a thumbnail preview image for mesh asset
		Bitmap^ RenderThumbnailForMesh(RMesh* MeshAsset, int Width, int Height);

		List<IManagedSceneObject^>^ SceneObjectsList;

		/// Managed delegate handler for async resource loaded
		ManagedInterface::AsyncResourceLoadedHandler^ AsyncResourceLoadedCallback;

		AsyncResourceLoadedWrapper^ AsyncResourceLoadedWrapperDelegate;
	};
}
