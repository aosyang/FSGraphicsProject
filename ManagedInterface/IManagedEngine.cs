using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ManagedInterface
{
    public interface IManagedEngine
    {
        bool Initialize(IntPtr hWnd);
        void Shutdown();
        void RunOneFrame();
        void Resize(int Width, int Height);

        List<String> GetMeshNameList();
        void UpdatePreviewMesh(String path, bool replace);
        void OnKeyDown(int keycode);
        void OnKeyUp(int keycode);

        IManagedSceneObject GetSelection();

        void DeleteSelection();
        void LoadScene(String filename);
        void SaveScene(String filename);

        void SaveMeshMaterialFromSelection();
        void ExportAllAnimationsToBinaryFiles();
    }
}
