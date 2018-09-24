//=============================================================================
// IManagedEngine.cs by Shiyang Ao, 2018 All Rights Reserved.
//
//
//=============================================================================
using System;
using System.Collections.Generic;

namespace ManagedInterface
{
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
        void UpdatePreviewMesh(string path, bool replace);

        List<IManagedSceneObject> GetSceneObjectsList();

        void OnKeyDown(int keycode);
        void OnKeyUp(int keycode);

        IManagedSceneObject GetSelection();
        void SetSelection(IManagedSceneObject SelectedSceneObject);

        bool DeleteSelection();
        void LoadScene(string filename);
        void SaveScene(string filename);

        void SaveMeshMaterialFromSelection();
        void ExportAllAnimationsToBinaryFiles();
    }
}
