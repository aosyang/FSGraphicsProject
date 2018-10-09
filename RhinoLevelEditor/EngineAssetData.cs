//=============================================================================
// RScene.h by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

namespace RhinoLevelEditor
{
    class EngineAssetData
    {
        /// <summary>
        /// The file path to the asset 
        /// </summary>
        public string Path
        {
            get { return path; }
            set { path = value; }
        }

        string path;
    }
}
