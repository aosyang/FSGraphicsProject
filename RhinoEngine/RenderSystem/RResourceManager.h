//=============================================================================
// RResourceManager.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#ifndef _RRESOURCEMANAGER_H
#define _RRESOURCEMANAGER_H

#include "RMesh.h"

class RResourceManager : public RSingleton<RResourceManager>
{
	friend class RSingleton<RResourceManager>;
public:

	void UnloadAllMeshes();

	RMesh* LoadFbxMesh(const char* filename, ID3D11InputLayout* inputLayout);
	ID3D11ShaderResourceView* LoadDDSTexture(const char* filename);

private:
	RResourceManager() {}
	~RResourceManager() {}

	vector<RMesh*>						m_MeshResources;
	vector<ID3D11ShaderResourceView*>	m_TextureResources;
};

#endif
