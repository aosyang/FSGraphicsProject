//=============================================================================
// IManagedEngine.cs by Shiyang Ao, 2018 All Rights Reserved.
//
//
//=============================================================================
using System;
using System.Collections.Generic;
using System.Drawing;

namespace ManagedInterface
{
    /// <summary>
    /// Managed delegate for OnResourceLoaded event
    /// </summary>
    /// <param name="ResourceName"></param>
    public delegate void AsyncResourceLoadedHandler(string ResourceName);

    public interface IManagedEngine
    {
        bool Initialize(IntPtr hWnd);
        void Shutdown();
        void RunOneFrame();
        void Resize(int Width, int Height);

        /// <summary>
        /// Get a list of mesh assets names
        /// </summary>
        /// <returns></returns>
        List<string> GetMeshNameList();
        bool IsMeshAssetReady(string MeshName);
        Bitmap GenerateMeshThumbnailBitmap(string MeshName, int Width, int Height);

        // Add a mesh object to scene with mesh asset
        void AddMeshObjectToScene(string MeshAssetPath);

        // Replace mesh asset for selected object
        void ReplaceMeshAssetForSelection(string MeshAssetPath);

        List<IManagedSceneObject> GetSceneObjectsList();

        void OnKeyDown(int keycode);
        void OnKeyUp(int keycode);
        void SetInputEnabled(bool bEnabled);

        IManagedSceneObject GetSelection();
        void SetSelection(IManagedSceneObject SelectedSceneObject);

        bool DeleteSelection();
        void LoadScene(string filename);
        void SaveScene(string filename);

        void SaveMeshMaterialFromSelection();
        void ExportAllAnimationsToBinaryFiles();

        /// Set the managed delegate for async resource loaded
        void SetAsyncResourceLoadedHandler(AsyncResourceLoadedHandler AsyncResourceLoaded);
    }
}
