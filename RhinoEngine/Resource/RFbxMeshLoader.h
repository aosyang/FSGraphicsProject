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
	/// Load animation of the scene
	RAnimation* LoadFbxSceneAnimation(FbxScene* Scene) const;

	/// Load materials from fbx node
	void LoadFbxMaterials(FbxNode* SceneNode, vector<RMaterial>& OutMaterials) const;

	/// Optimize mesh buffer by combining duplicated vertices
	void OptimizeMesh(vector<UINT>& IndexData, vector<RVertexType::MeshLoader>& VertexData) const;

	/// A helper function to convert fbx matrix to RMatrix4
	void MatrixTransfer(RMatrix4* dest, const FbxAMatrix* src) const;

};
