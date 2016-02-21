//=============================================================================
// RResourceManager.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#ifndef _RRESOURCEMANAGER_H
#define _RRESOURCEMANAGER_H

struct LoaderThreadTask
{
	string			Filename;
	RBaseResource*	Resource;
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
	void UnloadAllResources();

	RMesh* LoadFbxMesh(const char* filename, ID3D11InputLayout* inputLayout);
	RTexture* LoadDDSTexture(const char* filename);

	// Wrap a d3d11 srv and get a pointer to texture
	RTexture* WrapSRV(ID3D11ShaderResourceView* srv);

private:
	RResourceManager() {}
	~RResourceManager() {}

	vector<RMesh*>						m_MeshResources;
	vector<RTexture*>					m_TextureResources;
	map<ID3D11ShaderResourceView*, RTexture*>
										m_WrapperTextureResources;

	bool								m_ShouldQuitLoaderThread;
	mutex								m_TaskQueueMutex;
	condition_variable					m_TaskQueueCondition;
	queue<LoaderThreadTask>				m_LoaderThreadTaskQueue;
	LoaderThreadData					m_LoaderThreadData;
};

#endif
