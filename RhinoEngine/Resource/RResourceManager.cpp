//=============================================================================
// RResourceManager.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RResourceManager.h"
#include "RFbxMeshLoader.h"

#include <thread>
#include <mutex>
#include <condition_variable>

#include "tinyxml2/tinyxml2.h"

static std::mutex								m_TaskQueueMutex;
static std::condition_variable					m_TaskQueueCondition;
static std::mutex								m_PendingNotifyResourceMutex;

static std::mutex	TextureResourcesMutex;

std::string RResourceManager::AssetsBasePathName = "../Assets/";

void ResourceLoaderThread(LoaderThreadData* data)
{
	while (1)
	{
		LoaderThreadTask task;

		{
			std::unique_lock<std::mutex> uniqueLock(*data->TaskQueueMutex);
			if (data->TaskQueue->size() == 0)
			{
				RLog("=== Resource loader thread is idle ===\n");
			}

			// Block thread until we get signal for another task or need to quit thread
			data->TaskQueueCondition->wait(uniqueLock, [data]{ return data->TaskQueue->size() != 0 || *data->ShouldQuitThread; });

			if (*data->ShouldQuitThread)
				break;

			task = data->TaskQueue->front();
			data->TaskQueue->pop();
		}

		// Load resource in this task
		if (task.Resource->GetResourceState() == RS_EnqueuedForLoading)
		{
			task.Resource->LoadResourceData(true);
		}
	}

	// Notify main thread that loader thread has quit
	*data->HasQuitThread = true;
}

void RResourceManager::Initialize()
{
	RegisterResourceTypes();

	// Check if assets folder exists at given path. If not, go to the upper folder and search for it again.
	if (!RFileUtil::CheckPathExists(AssetsBasePathName))
	{
		std::vector<std::string> SearchedPaths;
		SearchedPaths.push_back(AssetsBasePathName);

		std::string FallbackPath = std::string("../") + AssetsBasePathName;
		if (RFileUtil::CheckPathExists(FallbackPath))
		{
			AssetsBasePathName = FallbackPath;
		}
		else
		{
			SearchedPaths.push_back(FallbackPath);

			RLogError("Assets folder does not exist in following searching paths:\n");
			for (const std::string& Path : SearchedPaths)
			{
				RLog("    %s\n", RFileUtil::GetFullPath(Path).c_str());
			}
		}
	}

	// Create resource loader thread
	m_ShouldQuitLoaderThread = false;
	m_HasLoaderThreadQuit = false;

	m_LoaderThreadData.TaskQueue = &m_LoaderThreadTaskQueue;
	m_LoaderThreadData.TaskQueueMutex = &m_TaskQueueMutex;
	m_LoaderThreadData.TaskQueueCondition = &m_TaskQueueCondition;
	m_LoaderThreadData.ShouldQuitThread = &m_ShouldQuitLoaderThread;
	m_LoaderThreadData.HasQuitThread = &m_HasLoaderThreadQuit;

	std::thread t(ResourceLoaderThread, &m_LoaderThreadData);
	t.detach();
}

void RResourceManager::Destroy()
{
	// terminate loader thread
	{
		std::unique_lock<std::mutex> uniqueLock(m_TaskQueueMutex);
		m_ShouldQuitLoaderThread = true;
	}
	m_TaskQueueCondition.notify_all();

	// Wait until loader thread has quit
	while (!m_HasLoaderThreadQuit)
	{
		Sleep(1);
	}

	UnloadAllResources();
}

void RResourceManager::LoadAllResources(EResourceLoadMode LoadMode /*= EResourceLoadMode::Threaded*/)
{
	std::vector<std::string> ResourcePaths = RFileUtil::GetFilesInDirectoryAndSubdirectories(GetAssetsBasePath(), "*.*");
	for (auto& Path : ResourcePaths)
	{
		LoadResourceAutoDetectType(Path, LoadMode);
	}
}

void RResourceManager::UnloadAllResources()
{
	for (auto Container : ResourceContainers)
	{
		Container->ReleaseAllResources();
		delete Container;
	}

	UnloadSRVWrappers();
}

void RResourceManager::UnloadSRVWrappers()
{
	for (const auto& Iter : m_WrapperTextureResources)
	{
		delete Iter.second;
	}
	m_WrapperTextureResources.clear();
}

void RResourceManager::AddPendingNotifyResource(RResourceBase* Resource)
{
	std::unique_lock<std::mutex> UniqueLock(m_PendingNotifyResourceMutex);

	if (find(PendingNotifyResources.begin(), PendingNotifyResources.end(), Resource) == PendingNotifyResources.end())
	{
		PendingNotifyResources.push_back(Resource);
	}
}

void RResourceManager::Update()
{
	std::unique_lock<std::mutex> UniqueLock(m_PendingNotifyResourceMutex);

	for (int i = (int)PendingNotifyResources.size() - 1; i >= 0; i--)
	{
		RResourceBase* Resource = PendingNotifyResources[i];

		if (Resource->IsLoaded() && Resource->AreReferencedResourcesLoaded())
		{
			OnResourceFinishedAsyncLoading.Execute(Resource);

			auto Index = PendingNotifyResources.begin() + i;
			PendingNotifyResources.erase(Index);
		}
	}
}

void RResourceManager::RegisterResourceTypes()
{
	RegisterResourceType<RMesh>();
	RegisterResourceType<RTexture>();
}

RResourceBase* RResourceManager::LoadResourceAutoDetectType(const std::string& Path, EResourceLoadMode mode)
{
	std::string FileExt = Path.substr(Path.find_last_of('.'));
	for (UINT i = 0; i < FileExt.size(); i++)
	{
		FileExt[i] = tolower(FileExt[i]);
	}

	for (auto& LoaderData : ResourceLoaders)
	{
		for (auto& LoaderExt : LoaderData.SupportedExtensions)
		{
			if (LoaderExt == FileExt)
			{
				return LoaderData.ResourceLoader->Load(Path.c_str(), mode);
			}
		}
	}

	return nullptr;
}

void RResourceManager::LockTaskQueue()
{
	m_TaskQueueMutex.lock();
}

void RResourceManager::UnlockTaskQueue()
{
	m_TaskQueueMutex.unlock();
}

void RResourceManager::NotifyThreadTaskEnqueued()
{
	m_TaskQueueCondition.notify_all();
}

std::vector<RMesh*> RResourceManager::GetMeshResources()
{
	return GetResourceContainer<RMesh>().GetResourceArrayCopy();
}

std::vector<RResourceBase*> RResourceManager::EnumerateAllResources()
{
	std::vector<RResourceBase*> Resources;
	for (auto& Container : ResourceContainers)
	{
		auto Array = Container->GetResourceBaseArray();
		Resources.insert(Resources.end(), Array.begin(), Array.end());
	}

	return Resources;
}

const std::string& RResourceManager::GetAssetsBasePath()
{
	return AssetsBasePathName;
}

std::string RResourceManager::GetRelativePathToResource(const std::string& ResourcePath)
{
	char currentPath[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, currentPath);

	std::string targetPath = std::string(currentPath) + "\\" + GetAssetsBasePath() + ResourcePath;

	char fullTargetPath[MAX_PATH];
	char** parts = { nullptr };
	GetFullPathNameA(targetPath.data(), MAX_PATH, fullTargetPath, parts);

	char RelativePath[MAX_PATH];
	PathRelativePathToA(RelativePath, currentPath, FILE_ATTRIBUTE_DIRECTORY, fullTargetPath, FILE_ATTRIBUTE_NORMAL);

	for (size_t i = 0; i < strlen(RelativePath); i++)
	{
		if (RelativePath[i] == '\\')
		{
			RelativePath[i] = '/';
		}
	}

	return RelativePath;
}

RTexture* RResourceManager::WrapShaderResourceViewInTexture(ID3D11ShaderResourceView* ShaderResourceView, bool bTransferOwnership /*= false*/)
{
	RTexture* Texture = nullptr;

	if (m_WrapperTextureResources.find(ShaderResourceView) != m_WrapperTextureResources.end())
	{
		Texture = m_WrapperTextureResources[ShaderResourceView];
		assert(Texture->HasOwnershipOfResource() == bTransferOwnership);
	}
	else
	{
		Texture = new RTexture(ShaderResourceView, bTransferOwnership);
		Texture->OnLoadingFinished(false);
		m_WrapperTextureResources[ShaderResourceView] = Texture;
	}

	return Texture;
}

