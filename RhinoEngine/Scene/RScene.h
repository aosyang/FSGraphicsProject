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
	void Render(const RFrustum* pFrustum=nullptr);
	void RenderDepthPass();

	vector<RSceneObject*>& GetSceneObjects();

	// Constant buffers
	RShaderConstantBuffer<SHADER_SCENE_BUFFER,		CBST_VS|CBST_GS|CBST_PS, 0>		cbScene;
	RShaderConstantBuffer<SHADER_GLOBAL_BUFFER,		CBST_VS|CBST_PS, 1>				cbGlobal;
	RShaderConstantBuffer<SHADER_OBJECT_BUFFER,		CBST_VS, 2>						cbPerObject;
	RShaderConstantBuffer<SHADER_SKINNED_BUFFER,	CBST_VS, 4>						cbBoneMatrices;
	RShaderConstantBuffer<SHADER_LIGHT_BUFFER,		CBST_PS, 2>						cbLight;
	RShaderConstantBuffer<SHADER_MATERIAL_BUFFER,	CBST_PS, 3>						cbMaterial;
private:

	vector<RSceneObject*>		m_SceneObjects;
};

#endif