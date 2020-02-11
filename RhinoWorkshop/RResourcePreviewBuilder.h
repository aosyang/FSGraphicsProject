//=============================================================================
// RResourcePreviewBuilder.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Core/CoreTypes.h"

class RResourceBase;
class RTexture;
class RSceneObject;
class RScene;

class RResourcePreviewBuilder
{
public:
	/// Generate thumbnails for all resources
	void BuildPreviewForAllResources();

	void BuildPreviewForResource(RResourceBase* Resource);

	RTexture* FindPreviewTexture(RResourceBase* Resource) const;

private:
	RTexture* GeneratePreviewTexture(RResourceBase* Resource, int Width, int Height);

	// Check if a resource is a skinned mesh
	bool IsSkinnedMesh(RResourceBase* Resource);

	// Create a preview scene object for the resource
	RSceneObject* CreatePreviewSceneObject(RScene& Scene, RResourceBase* Resource);

private:
	std::map<RResourceBase*, RTexture*> ResourcePreviews;
};
