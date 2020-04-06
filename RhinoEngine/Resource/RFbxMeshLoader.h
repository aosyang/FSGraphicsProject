//=============================================================================
// RFbxMeshLoader.h by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Core/CoreTypes.h"
#include "RenderSystem/RVertexDeclaration.h"

class RMesh;

class RFbxMeshLoader
{
public:
	/// Load data for mesh resource from file
	bool LoadDataForMeshResource(RMesh* MeshResource, const char* FileName);
	bool LoadDataForMeshResource(RMesh* MeshResource, const std::string& FileName);

private:
	/// Optimize mesh buffer by combining duplicated vertices
	void OptimizeMesh(std::vector<UINT>& IndexData, std::vector<RVertexType::MeshLoader>& VertexData) const;
};
