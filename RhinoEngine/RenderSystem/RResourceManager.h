//=============================================================================
// RResourceManager.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

struct RMaterial;

namespace std
{
	class mutex;
	class condition_variable;
}

struct LoaderThreadTask
{
	string			Filename;
	RResourceBase*	Resource;
	bool			bIsAsync;
};

struct LoaderThreadData
{
	mutex*						TaskQueueMutex;
	condition_variable*			TaskQueueCondition;
	queue<LoaderThreadTask>*	TaskQueue;
	bool*						ShouldQuitThread;
	bool*						HasQuitThread;
};

enum class EResourceLoadMode : UINT8
{
	Immediate,
	Threaded,
};

class RResourceManager : public RSingleton<RResourceManager>
{
	friend class RSingleton<RResourceManager>;
	friend void ResourceLoaderThread(LoaderThreadData* data);
public:
	void Initialize();
	void Destroy();

	void LoadAllResources();
	void UnloadAllResources();
	void UnloadSRVWrappers();

	/// Add a resource to pending notify list
	void AddPendingNotifyResource(RResourceBase* Resource);

	/// Delegate called when a resource has finished async loading
	RDelegate<RResourceBase*> OnResourceFinishedAsyncLoading;

	/// Update the resource manager every frame
	void Update();

	RMesh* LoadFbxMesh(const char* filename, EResourceLoadMode mode = EResourceLoadMode::Threaded);
	RTexture* LoadDDSTexture(const char* filename, EResourceLoadMode mode = EResourceLoadMode::Threaded);

	RTexture* FindTexture(const char* resourcePath);
	RMesh* FindMesh(const char* resourcePath);
	const vector<RMesh*>& GetMeshResources() const;

	static const string& GetAssetsBasePath();
	static string GetResourcePath(const string& path);

	// Wrap a d3d11 srv and get a pointer to texture
	RTexture* WrapSRV(ID3D11ShaderResourceView* srv);

private:
	RResourceManager() {}
	~RResourceManager() {}

	static void ThreadLoadFbxMeshData(LoaderThreadTask* task);
	static bool ThreadLoadRmeshData(LoaderThreadTask* task);
	static void LoadMeshMaterials(const string& mtlFilename, vector<RMaterial>& materials);

	static void ThreadLoadDDSTextureData(LoaderThreadTask* task);

	static string						AssetBasePathName;

	vector<RMesh*>						m_MeshResources;
	vector<RTexture*>					m_TextureResources;
	map<ID3D11ShaderResourceView*, RTexture*>
										m_WrapperTextureResources;

	bool								m_ShouldQuitLoaderThread;
	bool								m_HasLoaderThreadQuit;
	queue<LoaderThreadTask>				m_LoaderThreadTaskQueue;
	LoaderThreadData					m_LoaderThreadData;

	/// A list of loaded resources waiting to notify their states
	vector<RResourceBase*>				PendingNotifyResources;
};
