//=============================================================================
// RScene.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

class RScene
{
public:
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
	T* CreateSceneObjectOfType(const char* name = "");

	/// Clone an object in the scene
	RSceneObject* CloneObject(RSceneObject* obj);

	/// Find object in the scene by name
	RSceneObject* FindObject(const char* name) const;

	/// Find all objects in the scene of given type
	template<typename T>
	vector<T*> FindAllObjectsOfType() const;

	/// Generate a unique object name with the given name
	string GenerateUniqueObjectName(const string& ObjectName);

	/// Generate a unique object name for cloned object
	string GenerateUniqueObjectNameForClone(const string& ObjectName);

	/// Check if the name has been used by any object in the scene
	bool DoesObjectNameExist(const string& Name) const;

	/// Add an object to the scene
	void DestroyObject(RSceneObject* obj);
	void DestroyAllObjects();

	/// Load scene from file on disk
	void LoadFromFile(const char* filename);

	/// Save scene to file on disk
	void SaveToFile(const char* filename);

	RVec3 TestMovingAabbWithScene(const RAabb& aabb, const RVec3& moveVec, list<RSceneObject*> IgnoredObjects = list<RSceneObject*>());
	void Render(const RFrustum* pFrustum = nullptr);
	void RenderDepthPass(const RFrustum* pFrustum = nullptr);

	void UpdateScene();

	vector<RSceneObject*>& GetSceneObjects();
private:

	bool XmlReadObjectTransform(tinyxml2::XMLElement* ObjectElement, RVec3& OutPosition, RQuat& OutRotation, RVec3& OutScale);
	void XmlWriteObjectTransform(tinyxml2::XMLElement* ObjectElement, RSceneObject* SceneObject);

	bool XmlReadObjectTransformAsMatrix(tinyxml2::XMLElement* ObjectElement, RMatrix4& OutMatrix);
	void XmlWriteObjectTransformAsMatrix(tinyxml2::XMLElement* ObjectElement, RSceneObject* SceneObject);

	vector<RSceneObject*>		m_SceneObjects;
};

template<typename T>
T* RScene::CreateSceneObjectOfType(const char* name /*= ""*/)
{
	T* SceneObject = new T(this);
	SceneObject->SetName(name);

	m_SceneObjects.push_back(SceneObject);

	return SceneObject;
}

template<typename T>
vector<T*> RScene::FindAllObjectsOfType() const
{
	vector<T*> Results;

	for (auto SceneObject : m_SceneObjects)
	{
		if (SceneObject->GetRuntimeTypeId() == T::_StaticGetRuntimeTypeId())
		{
			Results.push_back(static_cast<T*>(SceneObject));
		}
	}

	return Results;
}
