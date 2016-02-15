//=============================================================================
// RResourceManager.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#ifndef _RRESOURCEMANAGER_H
#define _RRESOURCEMANAGER_H

#include "RMesh.h"

struct LoaderThreadTask
{
	string	Filename;
	RMesh*	Resource;
};

struct LoaderThreadData
{
	mutex*						TaskQueueMutex;
	condition_variable*			TaskQueueCondition;
	queue<LoaderThreadTask>*	TaskQueue;
	bool*						ShouldQuitThread;
};

class RResourceManager : public RSingleton<RResourceManager>
{
	friend class RSingleton<RResourceManager>;
public:
	void Initialize();
	void Destroy();
	void UnloadAllMeshes();

	RMesh* LoadFbxMesh(const char* filename, ID3D11InputLayout* inputLayout);
	ID3D11ShaderResourceView* LoadDDSTexture(const char* filename);

private:
	RResourceManager() {}
	~RResourceManager() {}

	vector<RMesh*>						m_MeshResources;
	vector<ID3D11ShaderResourceView*>	m_TextureResources;

	bool								m_ShouldQuitLoaderThread;
	mutex								m_TaskQueueMutex;
	condition_variable					m_TaskQueueCondition;
	queue<LoaderThreadTask>				m_LoaderThreadTaskQueue;
	LoaderThreadData					m_LoaderThreadData;
};

#endif
