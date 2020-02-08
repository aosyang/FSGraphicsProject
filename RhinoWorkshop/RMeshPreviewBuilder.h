//=============================================================================
// RMeshPreviewBuilder.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Core/CoreTypes.h"

class RMesh;
class RTexture;

class RMeshPreviewBuilder
{
public:
	void BuildPreviewForAllMeshes();

	RTexture* FindPreviewTexture(RMesh* Mesh) const;

private:
	RTexture* GeneratePreviewTexture(RMesh* Mesh, int Width, int Height);

private:
	std::map<RMesh*, RTexture*> MeshPreviews;
};
