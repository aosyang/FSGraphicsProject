//=============================================================================
// RFbxMeshLoader.h by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

class RFbxMeshLoader
{
public:
	/// Load data for mesh resource from file
	bool LoadDataForMeshResource(RMesh* MeshResource, const char* FileName);
	bool LoadDataForMeshResource(RMesh* MeshResource, const std::string& FileName);

private:
	/// Optimize mesh buffer by combining duplicated vertices
	void OptimizeMesh(vector<UINT>& IndexData, vector<RVertexType::MeshLoader>& VertexData) const;
};
