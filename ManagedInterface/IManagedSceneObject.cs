//=============================================================================
// IManagedSceneObject.cs by Shiyang Ao, 2018 All Rights Reserved.
//
//
//=============================================================================

namespace ManagedInterface
{
    public interface IManagedSceneObject
    {
        bool IsValid();

        /// <summary>
        /// The readable name of scene object
        /// </summary>
        string DisplayName
        {
            get;
        }
    }
}
