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

	void RemoveAllObjects();

	void LoadFromFile(const char* filename);
	void SaveToFile(const char* filename);

	void Render(const RFrustum* pFrustum=nullptr);

	vector<RSceneObject*>& GetSceneObjects();

	// Constant buffers
	RShaderConstantBuffer<SHADER_OBJECT_BUFFER,		CBST_VS, 0>				cbPerObject;
	RShaderConstantBuffer<SHADER_SCENE_BUFFER,		CBST_VS|CBST_GS, 1>		cbScene;
	RShaderConstantBuffer<SHADER_SKINNED_BUFFER,	CBST_VS, 3>				cbBoneMatrices;
	RShaderConstantBuffer<SHADER_LIGHT_BUFFER,		CBST_PS, 0>				cbLight;
	RShaderConstantBuffer<SHADER_MATERIAL_BUFFER,	CBST_PS, 1>				cbMaterial;
	RShaderConstantBuffer<SHADER_SCREEN_BUFFER,		CBST_PS, 2>				cbScreen;
private:

	vector<RSceneObject*>		m_SceneObjects;
};

#endif