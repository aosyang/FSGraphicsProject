//=============================================================================
// RResourceManager.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RResourceManager.h"
#include "MeshLoader\RFbxMeshLoader.h"

#include <thread>
#include <mutex>
#include <condition_variable>

#include "tinyxml2\tinyxml2.h"

#define ENABLE_THREADED_LOADING 1
#define EXPORT_FBX_AS_BINARY_MESH 1

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
			if (!RResourceManager::ThreadLoadRmeshData(&task))
			{
				RResourceManager::ThreadLoadFbxMeshData(&task);
			}
			break;

		case RT_Texture:
			RTexture* pTexture = RResourceManager::Instance().FindTexture(task.Filename.data());
			if (!pTexture || pTexture->GetResourceState() == RS_EnqueuedForLoading)
			{
				RResourceManager::ThreadLoadDDSTextureData(&task);
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
					LoadDDSTexture(resName.data());
				}
				else if (lowerExt == ".fbx")
				{
					LoadFbxMesh(resName.data());
				}
				else if (lowerExt == ".rmesh")
				{
					string fbxFilename = RFileUtil::ReplaceExtension(resName, "fbx");
					LoadFbxMesh(fbxFilename.data());
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
	for (UINT32 i = 0; i < m_MeshResources.size(); i++)
	{
		delete m_MeshResources[i];
	}
	m_MeshResources.clear();

	TextureResourcesMutex.lock();
	for (UINT32 i = 0; i < m_TextureResources.size(); i++)
	{
		delete m_TextureResources[i];
	}
	m_TextureResources.clear();
	TextureResourcesMutex.unlock();

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

RMesh* RResourceManager::LoadFbxMesh(const char* filename, EResourceLoadMode mode)
{
	RMesh* pMesh = RResourceManager::FindMesh(filename);
	if (pMesh)
		return pMesh;

	pMesh = new RMesh(GetResourcePath(filename));
	pMesh->OnEnqueuedForLoading();
	m_MeshResources.push_back(pMesh);

#if (ENABLE_THREADED_LOADING == 0)
	mode = EResourceLoadMode::Immediate;
#endif

	LoaderThreadTask task;
	task.Filename = string(filename);
	task.Resource = pMesh;
	task.bIsAsync = (mode == EResourceLoadMode::Threaded);

	if (mode == EResourceLoadMode::Immediate)
	{
		if (!ThreadLoadRmeshData(&task))
		{
			ThreadLoadFbxMeshData(&task);
		}
	}
	else
	{
		// Upload task to working queue
		m_TaskQueueMutex.lock();
		m_LoaderThreadTaskQueue.push(task);
		m_TaskQueueMutex.unlock();

		// Notify loader thread to start working
		m_TaskQueueCondition.notify_all();
	}

	return pMesh;
}

inline void MatrixTransfer(RMatrix4* dest, const FbxAMatrix* src)
{
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			dest->m[y][x] = (float)src->Get(y, x);
		}
	}
}

void RResourceManager::ThreadLoadFbxMeshData(LoaderThreadTask* task)
{
	RMesh* MeshResource = static_cast<RMesh*>(task->Resource);

	if (MeshResource)
	{
		std::unique_ptr<RFbxMeshLoader> FbxMeshLoader(new RFbxMeshLoader);

		if (FbxMeshLoader->LoadMeshIntoResource(MeshResource, task->Filename))
		{
			// Notify mesh has been loaded
			MeshResource->OnLoadingFinished(task->bIsAsync);

#if EXPORT_FBX_AS_BINARY_MESH == 1
			string rmeshName = RFileUtil::ReplaceExtension(task->Filename, "rmesh");
			RSerializer serializer;
			serializer.Open(rmeshName, ESerializeMode::Write);
			if (serializer.IsOpen())
			{
				MeshResource->Serialize(serializer);
				serializer.Close();
			}
#endif
		}
	}
}

bool RResourceManager::ThreadLoadRmeshData(LoaderThreadTask* task)
{
	vector<RMeshElement> meshElements;
	vector<RMaterial> materials;

	RLog("Loading mesh [%s]...\n", task->Filename.data());

	string rmeshName = RFileUtil::ReplaceExtension(task->Filename, "rmesh");
	RMesh* MeshResource = static_cast<RMesh*>(task->Resource);

	RSerializer serializer;
	serializer.Open(rmeshName, ESerializeMode::Read);
	if (!serializer.IsOpen())
		return false;
	MeshResource->Serialize(serializer);
	serializer.Close();

	//RAnimation* animation = new RAnimation();
	//string animFilename = RFileUtil::ReplaceExt(task->Filename.size(), "ranim");

	// Load material from file
	string mtlFilename = RFileUtil::ReplaceExtension(task->Filename, "rmtl");
	LoadMeshMaterials(mtlFilename, materials);
	
	if (materials.size())
		MeshResource->SetMaterials(materials.data(), (UINT)materials.size());

	MeshResource->OnLoadingFinished(task->bIsAsync);

	return true;
}

void RResourceManager::LoadMeshMaterials(const string& mtlFilename, vector<RMaterial>& materials)
{
	RMaterial::LoadFromXmlFile(mtlFilename, materials);
}

RTexture* RResourceManager::LoadDDSTexture(const char* filename, EResourceLoadMode mode)
{
	RTexture* pTexture = FindTexture(filename);
	if (pTexture)
		return pTexture;

	pTexture = new RTexture(GetResourcePath(filename));
	pTexture->OnEnqueuedForLoading();

	TextureResourcesMutex.lock();
	m_TextureResources.push_back(pTexture);
	TextureResourcesMutex.unlock();

	LoaderThreadTask task;
	task.Filename = string(filename);
	task.Resource = pTexture;

#if (ENABLE_THREADED_LOADING == 0)
	mode = EResourceLoadMode::Immediate;
#endif

	if (mode == EResourceLoadMode::Immediate)
	{
		ThreadLoadDDSTextureData(&task);
	}
	else // mode == EResourceLoadMode::Threaded
	{
		// Upload task to working queue
		m_TaskQueueMutex.lock();
		m_LoaderThreadTaskQueue.push(task);
		m_TaskQueueMutex.unlock();

		// Notify loader thread to start working
		m_TaskQueueCondition.notify_all();
	}

	return pTexture;
}

void RResourceManager::ThreadLoadDDSTextureData(LoaderThreadTask* task)
{
	ID3D11ShaderResourceView* srv;
	size_t char_len;
	wchar_t wszName[1024];
	mbstowcs_s(&char_len, wszName, 1024, task->Filename.data(), task->Filename.size());

	RLog("Loading texture [%s]...\n", task->Filename.data());

	bool bIsSRGBTexture = false;

	unique_ptr<tinyxml2::XMLDocument> XmlDoc(new tinyxml2::XMLDocument());
	string MetaFileName = task->Filename + ".meta";
	if (XmlDoc->LoadFile(MetaFileName.c_str()) == tinyxml2::XML_SUCCESS)
	{
		tinyxml2::XMLElement* MetaElem = XmlDoc->FirstChildElement("Metadata");
		if (MetaElem)
		{
			static const char NameSRGB[] = "SRGB";
			MetaElem->QueryBoolAttribute(NameSRGB, &bIsSRGBTexture);
		}
	}

	ID3D11Resource* pResource;
	HRESULT hr;
	
	if (bIsSRGBTexture)
	{
		hr = DirectX::CreateDDSTextureFromFileEx(GRenderer.D3DDevice(), wszName, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, true, &pResource, &srv);
	}
	else
	{
		hr = DirectX::CreateDDSTextureFromFile(GRenderer.D3DDevice(), wszName, &pResource, &srv);
	}

	if (FAILED(hr))
	{
		RLog("*** Failed to load texture [%s] ***\n", task->Filename.data());
	}

	RTexture* TextureResource = static_cast<RTexture*>(task->Resource);

	if (pResource)
	{
		ID3D11Texture2D* pTexture;
		pResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&pTexture);

		if (pTexture)
		{
			D3D11_TEXTURE2D_DESC desc;
			pTexture->GetDesc(&desc);

			TextureResource->m_Width = desc.Width;
			TextureResource->m_Height = desc.Height;

			pTexture->Release();
		}

		pResource->Release();
	}

	TextureResource->m_SRV = srv;
	TextureResource->OnLoadingFinished(task->bIsAsync);
}

// case-insensitive string comparison
int strcasecmp(const char* str1, const char* str2)
{
	int n = 0;
	while (str1[n] != 0 && str2[n] != 0)
	{
		char lower_ch1 = tolower(str1[n]);
		char lower_ch2 = tolower(str2[n]);
		if (lower_ch1 != lower_ch2)
			return lower_ch1 < lower_ch2 ? -1 : 1;
		n++;
	}

	if (str1[n] == 0 && str2[n] == 0)
		return 0;
	else if (str1[n] == 0)
		return -1;
	else
		return 1;
}

RTexture* RResourceManager::FindTexture(const char* resourcePath)
{
	unique_lock<mutex> lock(TextureResourcesMutex);

	if (resourcePath)
	{
		for (auto Iter : m_TextureResources)
		{
			if (strcasecmp(Iter->GetPath().c_str(), resourcePath) == 0)
			{
				return Iter;
			}

			// If searching path contains file name only, also try matching file names
			const string ResourceName = RFileUtil::GetFileNameInPath(Iter->GetPath());
			if (strcasecmp(ResourceName.c_str(), resourcePath) == 0)
			{
				return Iter;
			}
		}
	}

	return nullptr;
}

RMesh* RResourceManager::FindMesh(const char* resourcePath)
{
	for (auto Iter : m_MeshResources)
	{
		if (strcasecmp(Iter->GetPath().c_str(), resourcePath) == 0)
		{
			return Iter;
		}

		// If searching path contains file name only, also try matching file names
		const string ResourceName = RFileUtil::GetFileNameInPath(Iter->GetPath());
		if (strcasecmp(ResourceName.c_str(), resourcePath) == 0)
		{
			return Iter;
		}
	}

	return nullptr;
}

const vector<RMesh*>& RResourceManager::GetMeshResources() const
{
	return m_MeshResources;
}

const string& RResourceManager::GetAssetsBasePath()
{
	return AssetBasePathName;
}

string RResourceManager::GetResourcePath(const string& path)
{
	char currentPath[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, currentPath);

	string targetPath = string(currentPath) + "\\" + GetAssetsBasePath() + path;

	char fullTargetPath[MAX_PATH];
	char** parts = { nullptr };
	GetFullPathNameA(targetPath.data(), MAX_PATH, fullTargetPath, parts);

	char relPath[MAX_PATH];
	PathRelativePathToA(relPath, currentPath, FILE_ATTRIBUTE_DIRECTORY, fullTargetPath, FILE_ATTRIBUTE_NORMAL);

	for (size_t i = 0; i < strlen(relPath); i++)
	{
		if (relPath[i] == '\\')
			relPath[i] = '/';
	}

	return relPath;
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

