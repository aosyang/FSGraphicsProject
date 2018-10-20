//=============================================================================
// RFbxMeshLoader.h by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include <fbxsdk.h>

class RFbxMeshLoader
{
public:
	bool LoadMeshIntoResource(RMesh* MeshResource, const char* FileName);
	bool LoadMeshIntoResource(RMesh* MeshResource, const std::string& FileName);

private:
	/// Load materials from fbx node
	void LoadFbxMaterials(FbxNode* SceneNode, vector<RMaterial> &OutMaterials);

};
