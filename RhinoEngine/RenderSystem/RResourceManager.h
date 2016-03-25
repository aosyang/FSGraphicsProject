//=============================================================================
// RResourceManager.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#ifndef _RRESOURCEMANAGER_H
#define _RRESOURCEMANAGER_H

namespace std
{
	class mutex;
	class condition_variable;
}

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

enum ResourceLoadingMode
{
	RLM_Immediate,
	RLM_Threaded,
};

class RResourceManager : public RSingleton<RResourceManager>
{
	friend class RSingleton<RResourceManager>;
	friend void ResourceLoaderThread(LoaderThreadData* data);
public:
	void Initialize();
	void Destroy();
	void UnloadAllResources();
	void UnloadSRVWrappers();

	RMesh* LoadFbxMesh(const char* filename);
	RTexture* LoadDDSTexture(const char* filename, ResourceLoadingMode mode = RLM_Threaded);

	RTexture* FindTexture(const char* resourcePath);
	RMesh* FindMesh(const char* resourcePath);
	const vector<RMesh*>& GetMeshResources() const;

	static string GetAssetsBasePath();
	static string GetResourcePath(const string& path);

	// Wrap a d3d11 srv and get a pointer to texture
	RTexture* WrapSRV(ID3D11ShaderResourceView* srv);

private:
	RResourceManager() {}
	~RResourceManager() {}

	static void ThreadLoadFbxMeshData(LoaderThreadTask* task);
	static void ThreadLoadDDSTextureData(LoaderThreadTask* task);

	vector<RMesh*>						m_MeshResources;
	vector<RTexture*>					m_TextureResources;
	map<ID3D11ShaderResourceView*, RTexture*>
										m_WrapperTextureResources;

	bool								m_ShouldQuitLoaderThread;
	queue<LoaderThreadTask>				m_LoaderThreadTaskQueue;
	LoaderThreadData					m_LoaderThreadData;
};

#endif
