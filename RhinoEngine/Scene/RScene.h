//=============================================================================
// RScene.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#ifndef _RSCENE_H
#define _RSCENE_H

class RScene
{
public:
	void Initialize();
	void Release();

	/// Create a mesh object with mesh file path and add it to the scene
	RSMeshObject* CreateMeshObject(const char* meshName);

	/// Create a mesh object with mesh resource and add it to the scene
	RSMeshObject* CreateMeshObject(RMesh* mesh);

	RSceneObject* CreateSceneObject(const char* name = "");

	/// Create a object of class derived from scene object
	template<typename T>
	T* CreateSceneObjectOfType(const char* name = "");

	/// Clone an object in the scene
	RSceneObject* CloneObject(RSceneObject* obj);

	/// Find object in the scene by name
	RSceneObject* FindObject(const char* name) const;

	/// Add an object to the scene
	void DestroyObject(RSceneObject* obj);
	void DestroyAllObjects();

	/// Load scene from file on disk
	void LoadFromFile(const char* filename);

	/// Save scene to file on disk
	void SaveToFile(const char* filename);

	RVec3 TestMovingAabbWithScene(const RAabb& aabb, const RVec3& moveVec);
	void Render(const RFrustum* pFrustum = nullptr);
	void RenderDepthPass(const RFrustum* pFrustum = nullptr);

	void UpdateScene();

	vector<RSceneObject*>& GetSceneObjects();
private:

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

#endif