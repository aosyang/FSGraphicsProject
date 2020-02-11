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
	std::string			AssetPath;

	/// The resource object
	RResourceBase*	Resource;

	/// Whether this is an asynchronous loading  task
	bool			bIsAsync;
};

struct LoaderThreadData
{
	std::mutex*						TaskQueueMutex;
	std::condition_variable*		TaskQueueCondition;
	std::queue<LoaderThreadTask>*	TaskQueue;
	bool*							ShouldQuitThread;
	bool*							HasQuitThread;
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

	void LoadAllResources(EResourceLoadMode LoadMode = EResourceLoadMode::Threaded);
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
	T* LoadResource(const std::string& AssetPath, EResourceLoadMode mode = EResourceLoadMode::Threaded);

	/// Find resource by path
	template<typename T>
	T* FindResource(const std::string& Path, bool bAllowPartialMatch = false);

	/// Get an array of all mesh resources
	std::vector<RMesh*> GetMeshResources();

	template<typename T>
	std::vector<T*> EnumerateResourcesOfType();

	std::vector<RResourceBase*> EnumerateAllResources();

	/// Create a new resource
	template<typename T>
	T* CreateNewResource(const std::string& AssetPath);

	/// Root path of assets folder
	static const std::string& GetAssetsBasePath();

	/// Get relative path to resource from working directory
	static std::string GetRelativePathToResource(const std::string& ResourcePath);

	/// Wrap a d3d11 shader resource view and get a pointer to texture
	RTexture* WrapShaderResourceViewInTexture(ID3D11ShaderResourceView* ShaderResourceView, bool bTransferOwnership = false);

private:
	RResourceManager() {}
	~RResourceManager() {}

	void RegisterResourceTypes();

	template<typename T>
	void RegisterResourceType();

	/// Load resource, detecting file type by registered extensions
	RResourceBase* LoadResourceAutoDetectType(const std::string& Path, EResourceLoadMode mode = EResourceLoadMode::Threaded);

	/// Create a new resource container for resource type
	template<typename T>
	RResourceContainer<T>* CreateResourceContainer();

	/// Get resource container for resource type
	template<typename T>
	RResourceContainer<T>& GetResourceContainer();

	template<typename T>
	const RResourceContainer<T>& GetResourceContainer() const;

	static void LockTaskQueue();
	static void UnlockTaskQueue();
	static void NotifyThreadTaskEnqueued();

	static std::string						AssetsBasePathName;

	/// Registered resource containers for resource types
	std::vector<RResourceContainerBase*>		ResourceContainers;

	/// The internal resource loader interface. Used for resource loader registration
	class IResourceLoader
	{
	public:
		virtual RResourceBase* Load(const char*, EResourceLoadMode) = 0;
	};

	struct ResourceLoaderData
	{
		ResourceLoaderData(std::unique_ptr<IResourceLoader>& InLoaderFunc, const std::vector<std::string>& Exts)
			: ResourceLoader(move(InLoaderFunc))
			, SupportedExtensions(Exts)
		{}

		std::unique_ptr<IResourceLoader> ResourceLoader;
		std::vector<std::string>				SupportedExtensions;
	};

	std::vector<ResourceLoaderData>	ResourceLoaders;

	typedef std::map<ID3D11ShaderResourceView*, RTexture*> WrapperTextureMap;
	WrapperTextureMap					m_WrapperTextureResources;

	bool								m_ShouldQuitLoaderThread;
	bool								m_HasLoaderThreadQuit;
	std::queue<LoaderThreadTask>				m_LoaderThreadTaskQueue;
	LoaderThreadData					m_LoaderThreadData;

	/// A list of loaded resources waiting to notify their states
	std::vector<RResourceBase*>				PendingNotifyResources;
};

template<typename T>
T* RResourceManager::LoadResource(const std::string& AssetPath, EResourceLoadMode mode)
{
	// Find resource in resource container
	RResourceContainer<T>& ResourceContainer = GetResourceContainer<T>();
	T* Resource = ResourceContainer.Find(AssetPath);
	if (Resource != nullptr)
	{
		// The resource has been already loaded. Return it now.
		return Resource;
	}

	std::string RelativePath = RFileUtil::CombinePath(RResourceManager::GetAssetsBasePath(), AssetPath);
	Resource = new T(RelativePath);
	Resource->OnEnqueuedForLoading();

	ResourceContainer.Add(Resource);

	// Assign to asset path
	Resource->SetAssetPath(AssetPath);

#if (ENABLE_THREADED_LOADING == 0)
	mode = EResourceLoadMode::Immediate;
#endif

	if (mode == EResourceLoadMode::Immediate)
	{
		bool Result = Resource->LoadResourceData(false);

		// Failed to load the asset. Remove the resource and return null
		if (!Result)
		{
			ResourceContainer.Remove(Resource);
			delete Resource;
			Resource = nullptr;
		}
	}
	else
	{
		LoaderThreadTask task;
		task.AssetPath = AssetPath;
		task.Resource = Resource;
		task.bIsAsync = (mode == EResourceLoadMode::Threaded);

		// Upload task to working queue
		LockTaskQueue();
		m_LoaderThreadTaskQueue.push(task);
		UnlockTaskQueue();

		// Notify loader thread to start working
		NotifyThreadTaskEnqueued();
	}

	// TODO: Multi-threaded loading will still return the resource pointer even if the loading may fail
	return Resource;
}

template<typename T>
T* RResourceManager::FindResource(const std::string& Path, bool bAllowPartialMatch /*= false*/)
{
	RResourceContainer<T>& ResourceContainer = GetResourceContainer<T>();
	return ResourceContainer.Find(Path.c_str(), bAllowPartialMatch);
}

template<typename T>
std::vector<T*> RResourceManager::EnumerateResourcesOfType()
{
	return GetResourceContainer<T>().GetResourceArrayCopy();
}

template<typename T>
T* RResourceManager::CreateNewResource(const std::string& AssetPath)
{
	T* NewResource = new T(GetAssetsBasePath() + AssetPath);
	NewResource->SetAssetPath(AssetPath);
	GetResourceContainer<T>().Add(NewResource);
	return NewResource;
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

	std::unique_ptr<IResourceLoader> ResourceLoader(new ResourceTypeLoader());
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

template<typename T>
const RResourceContainer<T>& RResourceManager::GetResourceContainer() const
{
	return const_cast<RResourceManager*>(this)->GetResourceContainer();
}
