//=============================================================================
// RResourceManager.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "RenderSystem/RMaterial.h"
#include "RResourceContainer.h"

namespace std
{
	class mutex;
	class condition_variable;
}

#define ENABLE_THREADED_LOADING 1

struct LoaderThreadTask
{
	/// The asset path for accessing the resource in the engine
	string			AssetPath;

	/// The resource object
	RResourceBase*	Resource;

	/// Whether this is an asynchronous loading  task
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

/// The resource manager of engine
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

	/// Load resource from path
	template<typename T>
	T* LoadResource(const string& AssetPath, EResourceLoadMode mode = EResourceLoadMode::Threaded);

	/// Find resource by path
	template<typename T>
	T* FindResource(const string& Path);

	/// Get an array of all mesh resources
	vector<RMesh*> GetMeshResources();

	/// Root path of assets folder
	static const string& GetAssetsBasePath();

	/// Get relative path to resource from working directory
	static string GetRelativePathToResource(const string& ResourcePath);

	// Wrap a d3d11 srv and get a pointer to texture
	RTexture* WrapSRV(ID3D11ShaderResourceView* srv);

private:
	RResourceManager() {}
	~RResourceManager() {}

	void RegisterResourceTypes();

	template<typename T>
	void RegisterResourceType();

	/// Load resource, detecting file type by registered extensions
	RResourceBase* LoadResourceAutoDetectType(const string& Path, EResourceLoadMode mode = EResourceLoadMode::Threaded);

	/// Create a new resource container for resource type
	template<typename T>
	RResourceContainer<T>* CreateResourceContainer();

	/// Get resource container for resource type
	template<typename T>
	RResourceContainer<T>& GetResourceContainer();

	static void LockTaskQueue();
	static void UnlockTaskQueue();
	static void NotifyThreadTaskEnqueued();

	static string						AssetsBasePathName;

	/// Registered resource containers for resource types
	vector<RResourceContainerBase*>		ResourceContainers;

	class IResourceLoader
	{
	public:
		virtual RResourceBase* Load(const char*, EResourceLoadMode) = 0;
	};

	struct ResourceLoaderData
	{
		ResourceLoaderData(unique_ptr<IResourceLoader>& InLoaderFunc, const vector<string>& Exts)
			: ResourceLoader(move(InLoaderFunc))
			, SupportedExtensions(Exts)
		{}

		unique_ptr<IResourceLoader> ResourceLoader;
		vector<string>				SupportedExtensions;
	};

	vector<ResourceLoaderData>	ResourceLoaders;

	typedef map<ID3D11ShaderResourceView*, RTexture*> WrapperTextureMap;
	WrapperTextureMap					m_WrapperTextureResources;

	bool								m_ShouldQuitLoaderThread;
	bool								m_HasLoaderThreadQuit;
	queue<LoaderThreadTask>				m_LoaderThreadTaskQueue;
	LoaderThreadData					m_LoaderThreadData;

	/// A list of loaded resources waiting to notify their states
	vector<RResourceBase*>				PendingNotifyResources;
};

template<typename T>
T* RResourceManager::LoadResource(const string& AssetPath, EResourceLoadMode mode)
{
	// Find resource in resource container
	RResourceContainer<T>& ResourceContainer = GetResourceContainer<T>();
	T* Resource = ResourceContainer.Find(AssetPath);
	if (Resource != nullptr)
	{
		// The resource has been already loaded. Return it now.
		return Resource;
	}

	string RelativePath = RFileUtil::CombinePath(RResourceManager::GetAssetsBasePath(), AssetPath);
	Resource = new T(RelativePath);
	Resource->OnEnqueuedForLoading();

	ResourceContainer.Add(Resource);

#if (ENABLE_THREADED_LOADING == 0)
	mode = EResourceLoadMode::Immediate;
#endif

	LoaderThreadTask task;
	task.AssetPath = AssetPath;
	task.Resource = Resource;
	task.bIsAsync = (mode == EResourceLoadMode::Threaded);

	// Assign to asset path
	Resource->SetAssetPath(AssetPath);

	if (mode == EResourceLoadMode::Immediate)
	{
		Resource->LoadResourceData(false);
	}
	else
	{
		// Upload task to working queue
		LockTaskQueue();
		m_LoaderThreadTaskQueue.push(task);
		UnlockTaskQueue();

		// Notify loader thread to start working
		NotifyThreadTaskEnqueued();
	}

	return Resource;
}

template<typename T>
T* RResourceManager::FindResource(const string& Path)
{
	RResourceContainer<T>& ResourceContainer = GetResourceContainer<T>();
	return ResourceContainer.Find(Path.c_str());
}

template<typename T>
void RResourceManager::RegisterResourceType()
{
	// Functor class for template resource loader function
	class ResourceTypeLoader : public IResourceLoader
	{
	public:
		virtual RResourceBase* Load(const char* Path, EResourceLoadMode mode) override
		{
			return RResourceManager::Instance().LoadResource<T>(Path, mode);
		}
	};

	unique_ptr<IResourceLoader> ResourceLoader(new ResourceTypeLoader());
	ResourceLoaders.emplace_back(ResourceLoader, T::GetSupportedExtensions());
}

template<typename T>
RResourceContainer<T>* RResourceManager::CreateResourceContainer()
{
	RResourceContainer<T>* ResourceContainer = new RResourceContainer<T>();
	ResourceContainers.push_back(ResourceContainer);

	return ResourceContainer;
}

template<typename T>
RResourceContainer<T>& RResourceManager::GetResourceContainer()
{
	static RResourceContainer<T>* ResourceContainer = nullptr;

	// Create a container for the resource type if we don't have one
	if (ResourceContainer == nullptr)
	{
		ResourceContainer = CreateResourceContainer<T>();
	}

	assert(ResourceContainer);
	return *ResourceContainer;
}
