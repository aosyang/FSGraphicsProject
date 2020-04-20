//=============================================================================
// RScene.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Core/CoreTypes.h"

#include "RSceneObject.h"

class RSMeshObject;
class RMesh;
class RCamera;
struct RenderViewInfo;

class RScene
{
public:
	RScene();

	void Initialize();
	void Release();

	/// Create a mesh object with mesh file path and add it to the scene
	RSMeshObject* CreateMeshObject(const char* meshPath);

	/// Create a mesh object with mesh resource and add it to the scene
	RSMeshObject* CreateMeshObject(RMesh* mesh);

	/// Create a scene object in the scene
	RSceneObject* CreateSceneObject(const char* name = "");

	/// Create a object of class derived from scene object
	template<typename T>
	T* CreateSceneObjectOfType(const char* name = "", int Flags = 0);

	/// Clone an object in the scene
	RSceneObject* CloneObject(RSceneObject* obj);

	/// Find object in the scene by name
	RSceneObject* FindObject(const char* name) const;

	/// Find all objects in the scene of given type
	template<typename T>
	std::vector<T*> FindAllObjectsOfType(bool bMatchExactType = false) const;

	/// Generate a unique object name with the given name
	std::string GenerateUniqueObjectName(const std::string& ObjectName);

	/// Generate a unique object name for cloned object
	std::string GenerateUniqueObjectNameForClone(const std::string& ObjectName);

	/// Check if the name has been used by any object in the scene
	bool DoesObjectNameExist(const std::string& Name) const;

	/// Destroy a scene object
	void DestroyObject(RSceneObject* obj);

	/// Destroy all scene objects
	void DestroyAllObjects();

	/// Load scene from file on disk
	void LoadFromFile(const std::string& MapAssetPath);

	/// Save scene to file on disk
	void SaveToFile(const char* filename);

	/// Set current render camera for the scene
	void SetRenderCamera(RCamera* Camera);

	/// Get current render camera for the scene
	RCamera* GetRenderCamera() const;

	/// Notify the scene about creating a new camera
	void NotifyCameraCreated(RCamera* Camera);

	/// Notify the scene about destroying a camera
	void NotifyCameraDestroying(RCamera* Camera);

	/// Resolve collisions for a moving bounding box in the scene
	RVec3 TestMovingAabbWithScene(const RAabb& aabb, const RVec3& moveVec, std::list<RSceneObject*> IgnoredObjects = std::list<RSceneObject*>());

	void Render(const RenderViewInfo& View);
	void RenderDepthPass(const RFrustum* pFrustum = nullptr);

	void UpdateScene(float DeltaTime);
	void UpdateScene_PostPhysics(float DeltaTime);

	std::vector<RSceneObject*> EnumerateSceneObjects() const;
protected:

	void AddSceneObjectInternal(RSceneObject* SceneObject);

	/// Does culling get applied to a scene object?
	/// If frustum is null, this function returns false
	bool IsSceneObjectCulledByFrustum(RSceneObject* SceneObject, const RFrustum* Frustum) const;

private:

	std::vector<RSceneObject*>		m_SceneObjects;
	RCamera*					m_RenderCamera;			// Default camera will be used for frustum culling
};

template<typename T>
T* RScene::CreateSceneObjectOfType(const char* name /*= ""*/, int Flags /*= 0*/)
{
	T* SceneObject = new T(RConstructingParams(this, Flags));
	SceneObject->SetName(name);

	AddSceneObjectInternal(SceneObject);

	return SceneObject;
}

template<typename T>
std::vector<T*> RScene::FindAllObjectsOfType(bool bMatchExactType /*= false*/) const
{
	std::vector<T*> Results;

	if (bMatchExactType)
	{
		for (auto SceneObject : m_SceneObjects)
		{
			if (SceneObject->IsExactType<T>())
			{
				Results.push_back(static_cast<T*>(SceneObject));
			}
		}
	}
	else
	{
		for (auto SceneObject : m_SceneObjects)
		{
			if (T* Object = SceneObject->CastTo<T>())
			{
				Results.push_back(Object);
			}
		}
	}

	return Results;
}
