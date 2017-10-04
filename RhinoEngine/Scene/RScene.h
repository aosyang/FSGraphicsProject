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

	RSMeshObject* CreateMeshObject(const char* meshName);
	RSMeshObject* CreateMeshObject(RMesh* mesh);

	RSceneObject* CloneObject(RSceneObject* obj);
	RSceneObject* FindObject(const char* name) const;

	bool AddObjectToScene(RSceneObject* obj);
	void RemoveObjectFromScene(RSceneObject* obj);
	void DestroyObject(RSceneObject* obj);
	void DestroyAllObjects();

	void LoadFromFile(const char* filename);
	void SaveToFile(const char* filename);

	RVec3 TestMovingAabbWithScene(const RAabb& aabb, const RVec3& moveVec);
	void Render(const RFrustum* pFrustum = nullptr);
	void RenderDepthPass(const RFrustum* pFrustum = nullptr);

	vector<RSceneObject*>& GetSceneObjects();
private:

	vector<RSceneObject*>		m_SceneObjects;
};

#endif