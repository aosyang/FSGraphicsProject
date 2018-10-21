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

static mutex								m_TaskQueueMutex;
static condition_variable					m_TaskQueueCondition;
static mutex								m_PendingNotifyResourceMutex;

static mutex	TextureResourcesMutex;

string RResourceManager::AssetBasePathName = "../Assets/";

void ResourceLoaderThread(LoaderThreadData* data)
{
	while (1)
	{
		LoaderThreadTask task;

		{
			unique_lock<mutex> uniqueLock(*data->TaskQueueMutex);
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
		switch (task.Resource->GetResourceType())
		{
		case RT_Mesh:
			task.Resource->LoadResourceData(true);
			break;

		case RT_Texture:
			RTexture* pTexture = RResourceManager::Instance().FindResource<RTexture>(task.Filename.data());
			if (!pTexture || pTexture->GetResourceState() == RS_EnqueuedForLoading)
			{
				task.Resource->LoadResourceData(true);
			}
			break;
		}
	}

	// Notify main thread that loader thread has quit
	*data->HasQuitThread = true;
}

void RResourceManager::Initialize()
{
	// Create resource loader thread
	m_ShouldQuitLoaderThread = false;
	m_HasLoaderThreadQuit = false;

	m_LoaderThreadData.TaskQueue = &m_LoaderThreadTaskQueue;
	m_LoaderThreadData.TaskQueueMutex = &m_TaskQueueMutex;
	m_LoaderThreadData.TaskQueueCondition = &m_TaskQueueCondition;
	m_LoaderThreadData.ShouldQuitThread = &m_ShouldQuitLoaderThread;
	m_LoaderThreadData.HasQuitThread = &m_HasLoaderThreadQuit;

	thread t(ResourceLoaderThread, &m_LoaderThreadData);
	t.detach();
}

void RResourceManager::Destroy()
{
	// terminate loader thread
	{
		unique_lock<mutex> uniqueLock(m_TaskQueueMutex);
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

void RResourceManager::LoadAllResources()
{
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind;

	// Load resources including sub-directories
	queue<string> dir_queue;
	dir_queue.push("");

	do
	{
		string dir_name = dir_queue.front();
		dir_queue.pop();
		string resFindingPath = GetAssetsBasePath() + dir_name + "*.*";
		hFind = FindFirstFileA(resFindingPath.data(), &FindFileData);

		do
		{
			if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
			{
				string resName = GetAssetsBasePath() + dir_name + FindFileData.cFileName;
				
				string lowerExt = resName.substr(resName.find_last_of('.'));
				for (UINT i = 0; i < lowerExt.size(); i++)
				{
					lowerExt[i] = tolower(lowerExt[i]);
				}

				if (lowerExt == ".dds")
				{
					LoadResource<RTexture>(resName.data());
				}
				else if (lowerExt == ".fbx")
				{
					LoadResource<RMesh>(resName.data());
				}
				else if (lowerExt == ".rmesh")
				{
					string fbxFilename = RFileUtil::ReplaceExtension(resName, "fbx");
					LoadResource<RMesh>(fbxFilename.data());
				}
			}
			else
			{
				if (FindFileData.cFileName[0] != '.')
				{
					dir_queue.push(dir_name + string(FindFileData.cFileName) + "/");
				}
			}

		} while (FindNextFileA(hFind, &FindFileData) != 0);
	} while (dir_queue.size());
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
		// Delete wrapper textures without releasing shader resource view
		Iter.second->m_SRV = nullptr;
		delete Iter.second;
	}
	m_WrapperTextureResources.clear();
}

void RResourceManager::AddPendingNotifyResource(RResourceBase* Resource)
{
	unique_lock<mutex> UniqueLock(m_PendingNotifyResourceMutex);

	if (find(PendingNotifyResources.begin(), PendingNotifyResources.end(), Resource) == PendingNotifyResources.end())
	{
		PendingNotifyResources.push_back(Resource);
	}
}

void RResourceManager::Update()
{
	unique_lock<mutex> UniqueLock(m_PendingNotifyResourceMutex);

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

vector<RMesh*> RResourceManager::GetMeshResources()
{
	return GetResourceContainer<RMesh>().GetResourceArrayCopy();
}

const string& RResourceManager::GetAssetsBasePath()
{
	return AssetBasePathName;
}

string RResourceManager::GetRelativePathToResource(const string& ResourcePath)
{
	char currentPath[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, currentPath);

	string targetPath = string(currentPath) + "\\" + GetAssetsBasePath() + ResourcePath;

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

RTexture* RResourceManager::WrapSRV(ID3D11ShaderResourceView* srv)
{
	if (m_WrapperTextureResources.find(srv) != m_WrapperTextureResources.end())
		return m_WrapperTextureResources[srv];

	RTexture* tex = new RTexture(srv);
	tex->OnLoadingFinished(false);
	m_WrapperTextureResources[srv] = tex;
	return tex;
}

