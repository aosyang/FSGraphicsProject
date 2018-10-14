//=============================================================================
// RResourceManager.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RResourceManager.h"

#include <thread>
#include <mutex>
#include <condition_variable>

// FBX SDK
#include <fbxsdk.h>

#include "tinyxml2\tinyxml2.h"

#define ENABLE_THREADED_LOADING 1
#define EXPORT_FBX_AS_BINARY_MESH 1
#define CONVERT_TO_LEFT_HANDED_MESH 1

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
	vector<RMeshElement> meshElements;
	vector<RMaterial> materials;

	RLog("Loading mesh [%s]...\n", task->Filename.data());

	// Create the FBX SDK manager
	FbxManager* lFbxSdkManager = FbxManager::Create();

	// Create an IOSettings object.
	FbxIOSettings * ios = FbxIOSettings::Create(lFbxSdkManager, IOSROOT);
	lFbxSdkManager->SetIOSettings(ios);

	// ... Configure the FbxIOSettings object ...

	// Import options determine what kind of data is to be imported.
	// True is the default, but here we’ll set some to true explicitly, and others to false.
	(*(lFbxSdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_MATERIAL, true);
	(*(lFbxSdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_TEXTURE, true);
	(*(lFbxSdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_LINK, false);
	(*(lFbxSdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_SHAPE, false);
	(*(lFbxSdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_GOBO, false);
	(*(lFbxSdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_ANIMATION, true);
	(*(lFbxSdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);

	// Create an importer.
	FbxImporter* lImporter = FbxImporter::Create(lFbxSdkManager, "");

	// Declare the path and filename of the file containing the scene.
	// In this case, we are assuming the file is in the same directory as the executable.

	// Initialize the importer.
	bool lImportStatus = lImporter->Initialize(task->Filename.data(), -1, lFbxSdkManager->GetIOSettings());

	if (!lImportStatus) {
		RLogError("Call to FbxImporter::Initialize() failed.\n");
		RLogError("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());

		lFbxSdkManager->Destroy();
		return;
	}

	// File format version numbers to be populated.
	int lFileMajor, lFileMinor, lFileRevision;

	// Populate the FBX file format version numbers with the import file.
	lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);

	// Create a new scene so it can be populated by the imported file.
	FbxScene* lFbxScene = FbxScene::Create(lFbxSdkManager, "myScene");

	// Import the contents of the file into the scene.
	lImporter->Import(lFbxScene);

	// The file has been imported; we can get rid of the importer.
	lImporter->Destroy();

	// Convert mesh, NURBS and patch into triangle mesh
	FbxGeometryConverter lGeomConverter(lFbxSdkManager);

	lGeomConverter.Triangulate(lFbxScene, true, true);
	//bool result = lGeomConverter.SplitMeshesPerMaterial(lFbxScene, true);

	// Load skinning nodes
	vector<FbxNode*> fbxBoneNodes;
	vector<string> meshBoneIdToName;
	int NumFbxNodes = lFbxScene->GetNodeCount();

	// Load bone information into an array
	for (int idxNode = 0; idxNode < NumFbxNodes; idxNode++)
	{
		FbxNode* node = lFbxScene->GetNode(idxNode);
		if (node->GetNodeAttribute() && node->GetNodeAttribute()->GetAttributeType() && node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
		{
			fbxBoneNodes.push_back(node);
			meshBoneIdToName.push_back(node->GetName());

			RLog("  FBX bone node: %s\n", node->GetName());
		}
	}

	// Load animation
	RAnimation* animation = nullptr;

	FbxArray<FbxString*> animStackNameArray;
	lFbxScene->FillAnimStackNameArray(animStackNameArray);
	if (animStackNameArray.GetCount() > 0)
	{
		FbxAnimStack* animStack = lFbxScene->FindMember<FbxAnimStack>(animStackNameArray[0]->Buffer());
		FbxTakeInfo* takeInfo = lFbxScene->GetTakeInfo(*(animStackNameArray[0]));

		FbxArrayDelete(animStackNameArray);

		FbxTime::EMode				animTimeMode;
		FbxTime						TimePerFrame, animStartTime, animEndTime;
		float						animFrameRate;

		TimePerFrame.SetTime(0, 0, 0, 1, 0, lFbxScene->GetGlobalSettings().GetTimeMode());
		animTimeMode = lFbxScene->GetGlobalSettings().GetTimeMode();
		animFrameRate = (float)TimePerFrame.GetFrameRate(animTimeMode);
		animStartTime = takeInfo->mLocalTimeSpan.GetStart();
		animEndTime = takeInfo->mLocalTimeSpan.GetStop();

		int totalFrameCount = (int)(animEndTime.GetFrameCount(animTimeMode) - animStartTime.GetFrameCount(animTimeMode)) + 1;
		animation = new RAnimation(
			NumFbxNodes,
			totalFrameCount,
			(float)animStartTime.GetFrameCountPrecise(animTimeMode),
			(float)animEndTime.GetFrameCountPrecise(animTimeMode),
			animFrameRate);

		map<string, int> nodeNameToId;

		for (int FbxSceneNodeIndex = 0; FbxSceneNodeIndex < NumFbxNodes; FbxSceneNodeIndex++)
		{
			FbxNode* node = lFbxScene->GetNode(FbxSceneNodeIndex);

			for (FbxTime CurrentFrameTime = animStartTime;
				 CurrentFrameTime <= animEndTime;
				 CurrentFrameTime += TimePerFrame)
			{
				RMatrix4 BoneTransform;
				const char* BoneName = node->GetName();

				// Evaluate bone transform in model space
				FbxAMatrix FbxBoneTransform = node->EvaluateGlobalTransform(CurrentFrameTime);

#if CONVERT_TO_LEFT_HANDED_MESH == 1
				FbxVector4 FbxBoneRotation = FbxBoneTransform.GetR();
				FbxBoneTransform[3][2] = -FbxBoneTransform[3][2];
				FbxBoneRotation.Set(-FbxBoneRotation[0], -FbxBoneRotation[1], FbxBoneRotation[2]);
				FbxBoneTransform.SetR(FbxBoneRotation);
#endif
				MatrixTransfer(&BoneTransform, &FbxBoneTransform);

				// Precise frames number in fractions
				float NumFramesAtCurrentTime = (float)CurrentFrameTime.GetFrameCountPrecise(animTimeMode);
				float NumFramesAtStartTime = (float)animStartTime.GetFrameCountPrecise(animTimeMode);

				int FrameIndex = (int)(NumFramesAtCurrentTime - NumFramesAtStartTime);
				animation->AddNodePose(FbxSceneNodeIndex, FrameIndex, &BoneTransform);
				animation->AddNodeNameToId(BoneName, FbxSceneNodeIndex);

				nodeNameToId[BoneName] = FbxSceneNodeIndex;
			}
		}

		for (int FbxSceneNodeIndex = 0; FbxSceneNodeIndex < NumFbxNodes; FbxSceneNodeIndex++)
		{
			FbxNode* node = lFbxScene->GetNode(FbxSceneNodeIndex);
			FbxNode* parent = node->GetParent();
			if (parent && nodeNameToId.find(parent->GetName()) != nodeNameToId.end())
			{
				animation->SetParentId(FbxSceneNodeIndex, nodeNameToId[parent->GetName()]);
			}
			else
			{
				animation->SetParentId(FbxSceneNodeIndex, -1);
			}
		}
	}

	vector<RMatrix4> boneInitInvPose;

	// Load meshes
	for (int idxNode = 0; idxNode < NumFbxNodes; idxNode++)
	{
		RLog("  FBX node [%d/%d]...\n", idxNode + 1, NumFbxNodes);

		FbxNode* node = lFbxScene->GetNode(idxNode);
		FbxMesh* mesh = node->GetMesh();

		if (!mesh)
			continue;

		//mesh->SplitPoints();
		RLog("  Mesh element [%s]\n", node->GetName());
		
		FbxVector4* controlPointArray;
		vector<RVertexType::MeshLoader> vertData;
		vector<int> indexData;
		vector<RVertexType::MeshLoader> flatVertData;
		int VertexComponentMask = 0;

		controlPointArray = mesh->GetControlPoints();
		int controlPointCount = mesh->GetControlPointsCount();

		vertData.resize(controlPointCount);

		// Fill vertex data
		for (int i = 0; i < controlPointCount; i++)
		{
			vertData[i].pos.x = (float)controlPointArray[i][0];
			vertData[i].pos.y = (float)controlPointArray[i][1];
			vertData[i].pos.z = (float)controlPointArray[i][2];

#if CONVERT_TO_LEFT_HANDED_MESH == 1
			vertData[i].pos.z = -vertData[i].pos.z;
#endif

			memset(vertData[i].boneId, -1, sizeof(int) * 4);
			memset(&vertData[i].weight, 0, sizeof(float) * 4);

			VertexComponentMask |= VCM_Pos;
		}

		// Fill normal data
		FbxGeometryElementNormal* normalArray = mesh->GetElementNormal();
		bool hasPerPolygonVertexNormal = (normalArray->GetMappingMode() == FbxGeometryElement::eByPolygonVertex);

		switch (normalArray->GetMappingMode())
		{
		case FbxGeometryElement::eByControlPoint:
			switch (normalArray->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
				for (int i = 0; i < controlPointCount; i++)
				{
					FbxVector4 normal = normalArray->GetDirectArray().GetAt(i);

					vertData[i].normal.x = (float)normal[0];
					vertData[i].normal.y = (float)normal[1];
					vertData[i].normal.z = (float)normal[2];

#if CONVERT_TO_LEFT_HANDED_MESH == 1
					vertData[i].normal.z = -vertData[i].normal.z;
#endif

					VertexComponentMask |= VCM_Normal;
				}
				break;

			case FbxGeometryElement::eIndexToDirect:
				for (int i = 0; i < controlPointCount; i++)
				{
					int index = normalArray->GetIndexArray().GetAt(i);
					FbxVector4 normal = normalArray->GetDirectArray().GetAt(index);

					vertData[i].normal.x = (float)normal[0];
					vertData[i].normal.y = (float)normal[1];
					vertData[i].normal.z = (float)normal[2];

#if CONVERT_TO_LEFT_HANDED_MESH == 1
					vertData[i].normal.z = -vertData[i].normal.z;
#endif

					VertexComponentMask |= VCM_Normal;
				}
				break;
			}
			break;
		}

		// Tangent
		FbxGeometryElementTangent* tangentArray = mesh->GetElementTangent();

		// If the mesh doesn't have tangents, try generating a set of them
		if (!tangentArray)
		{
			if (mesh->GenerateTangentsData(0))
			{
				tangentArray = mesh->GetElementTangent();
			}
		}

		bool hasPerPolygonVertexTangent = (tangentArray && tangentArray->GetMappingMode() == FbxGeometryElement::eByPolygonVertex);

		if (tangentArray && !hasPerPolygonVertexTangent)
		{
			switch (tangentArray->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
				for (int i = 0; i < controlPointCount; i++)
				{
					FbxVector4 tangent = tangentArray->GetDirectArray().GetAt(i);

					vertData[i].tangent.x = (float)tangent[0];
					vertData[i].tangent.y = (float)tangent[1];
					vertData[i].tangent.z = (float)tangent[2];
				
#if CONVERT_TO_LEFT_HANDED_MESH == 1
					vertData[i].tangent.z = -vertData[i].tangent.z;
#endif

					VertexComponentMask |= VCM_Tangent;
				}
				break;

			case FbxGeometryElement::eIndexToDirect:
				for (int i = 0; i < controlPointCount; i++)
				{
					int index = tangentArray->GetIndexArray().GetAt(i);
					FbxVector4 tangent = tangentArray->GetDirectArray().GetAt(index);

					vertData[i].tangent.x = (float)tangent[0];
					vertData[i].tangent.y = (float)tangent[1];
					vertData[i].tangent.z = (float)tangent[2];

#if CONVERT_TO_LEFT_HANDED_MESH == 1
					vertData[i].tangent.z = -vertData[i].tangent.z;
#endif

					VertexComponentMask |= VCM_Tangent;
				}
				break;
			}
		}

		FbxGeometryElementUV* uvArray[2] =
		{
			mesh->GetElementUV(0),
			mesh->GetElementUV(1),
		};

		bool hasPerPolygonVertexUV[2];
		
		for (int uvLayer = 0; uvLayer < 2; uvLayer++)
		{
			hasPerPolygonVertexUV[uvLayer] = uvArray[uvLayer] ? (uvArray[uvLayer]->GetMappingMode() == FbxGeometryElement::eByPolygonVertex) : false;

			if (uvArray[uvLayer] && !hasPerPolygonVertexUV)
			{
				switch (uvArray[uvLayer]->GetReferenceMode())
				{
				case FbxGeometryElement::eDirect:
					for (int i = 0; i < controlPointCount; i++)
					{
						RVertexType::Vec2Data& vertUV = uvLayer == 0 ? vertData[i].uv0 : vertData[i].uv1;
							
						FbxVector2 uv = uvArray[i]->GetDirectArray().GetAt(i);

						vertUV.x = (float)uv[0];
						vertUV.y = 1.0f - (float)uv[1];

						if (uvLayer == 0)
							VertexComponentMask |= VCM_UV0;
						else
							VertexComponentMask |= VCM_UV1;
					}
					break;

				case FbxGeometryElement::eIndexToDirect:
					for (int i = 0; i < controlPointCount; i++)
					{
						RVertexType::Vec2Data& vertUV = uvLayer == 0 ? vertData[i].uv0 : vertData[i].uv1;
						
						int index = uvArray[uvLayer]->GetIndexArray().GetAt(i);
						FbxVector2 uv = uvArray[uvLayer]->GetDirectArray().GetAt(index);

						vertUV.x = (float)uv[0];
						vertUV.y = 1.0f - (float)uv[1];

						if (uvLayer == 0)
							VertexComponentMask |= VCM_UV0;
						else
							VertexComponentMask |= VCM_UV1;
					}
					break;
				}
			}
		}

		bool hasDeformer = (mesh->GetDeformer(0, FbxDeformer::eSkin) != NULL);
		if (hasDeformer)
		{
			int deformerCount = mesh->GetDeformerCount();

			for (int idxSkinDeformer = 0; idxSkinDeformer < deformerCount; idxSkinDeformer++)
			{
				// A deformer on a mesh is a skinning controller that keeps all cluster (bone) information
				FbxSkin* skinDeformer = (FbxSkin*)mesh->GetDeformer(idxSkinDeformer, FbxDeformer::eSkin);

				int clusterCount = skinDeformer->GetClusterCount();
				for (int idxCluster = 0; idxCluster < clusterCount; idxCluster++)
				{
					// A cluster is structure contains bone node, affected points and weight of affection
					// Binding pose matrix can also be retrieved from cluster
					FbxCluster* cluster = skinDeformer->GetCluster(idxCluster);

					if (!cluster->GetLink())
						continue;

					int boneId = (int)(std::find(fbxBoneNodes.begin(), fbxBoneNodes.end(), cluster->GetLink()) - fbxBoneNodes.begin());
					assert(boneId < MAX_BONE_COUNT);

					// Store inversed initial transform for each bone to apply skinning with correct binding pose
					FbxAMatrix clusterInitTransform;
					cluster->GetTransformLinkMatrix(clusterInitTransform);
#if CONVERT_TO_LEFT_HANDED_MESH == 1
					FbxVector4 rotation = clusterInitTransform.GetR();
					clusterInitTransform[3][2] = -clusterInitTransform[3][2];
					rotation.Set(-rotation[0], -rotation[1], rotation[2]);
					clusterInitTransform.SetR(rotation);
#endif
					clusterInitTransform = clusterInitTransform.Inverse();

					if (boneInitInvPose.size() == 0 && fbxBoneNodes.size())
						boneInitInvPose.resize(fbxBoneNodes.size());

					if (boneInitInvPose.size() != 0)
						MatrixTransfer(boneInitInvPose.data() + boneId, &clusterInitTransform);

					int cpIndicesCount = cluster->GetControlPointIndicesCount();
					for (int idxCpIndex = 0; idxCpIndex < cpIndicesCount; idxCpIndex++)
					{
						// Note: A control point is a point affected by this cluster (bone)

						int index = cluster->GetControlPointIndices()[idxCpIndex];
						float weight = (float)cluster->GetControlPointWeights()[idxCpIndex];

						// Store bone id and weight in an empty slot of vertex skinning attributes
						for (int i = 0; i < 4; i++)
						{
							if (vertData[index].boneId[i] == -1)
							{
								vertData[index].boneId[i] = boneId;
								vertData[index].weight[i] = weight;

								VertexComponentMask |= VCM_BoneId;
								VertexComponentMask |= VCM_BoneWeights;

								break;
							}
						}
					}

				}
			}
		}

		// Set bone id in unused slot to 0 so shader won't mess up
		for (UINT32 n = 0; n < vertData.size(); n++)
		{
			for (int i = 0; i < 4; i++)
			{
				if (vertData[n].boneId[i] == -1)
					vertData[n].boneId[i] = 0;
			}
		}

		// Fill triangle data
		int polyCount = mesh->GetPolygonCount();

		for (int idxPoly = 0; idxPoly < polyCount; idxPoly++)
		{
			// Loop through index buffer
			int vertCountPerPoly = mesh->GetPolygonSize(idxPoly);

			assert(vertCountPerPoly == 3);

			// Note: Assume mesh has been triangulated
			int triangle[3];

			for (int idxVert = 0; idxVert < vertCountPerPoly; idxVert++)
			{
				//triangle[idxVert] = mesh->GetPolygonVertex(idxPoly, idxVert);
				triangle[idxVert] = idxPoly * 3 + idxVert;
				int iv = mesh->GetPolygonVertex(idxPoly, idxVert);

				RVertexType::MeshLoader vertex = vertData[iv];
				
				if (hasPerPolygonVertexNormal)
				{
					FbxVector4 normal;
					mesh->GetPolygonVertexNormal(idxPoly, idxVert, normal);

					vertex.normal.x = (float)normal[0];
					vertex.normal.y = (float)normal[1];
					vertex.normal.z = (float)normal[2];

#if CONVERT_TO_LEFT_HANDED_MESH == 1
					vertex.normal.z = -vertex.normal.z;
#endif

					VertexComponentMask |= VCM_Normal;
				}

				if (hasPerPolygonVertexTangent)
				{
					switch (tangentArray->GetReferenceMode())
					{
					case FbxGeometryElement::eDirect:
						{
							FbxVector4 tangent = tangentArray->GetDirectArray().GetAt(idxPoly * 3 + idxVert);

							vertex.tangent.x = (float)tangent[0];
							vertex.tangent.y = (float)tangent[1];
							vertex.tangent.z = (float)tangent[2];

#if CONVERT_TO_LEFT_HANDED_MESH == 1
							vertex.tangent.z = -vertex.tangent.z;
#endif

							VertexComponentMask |= VCM_Tangent;
						}
						break;
					case FbxGeometryElement::eIndexToDirect:
						{
							int index = tangentArray->GetIndexArray().GetAt(idxPoly * 3 + idxVert);
							FbxVector4 tangent = tangentArray->GetDirectArray().GetAt(index);

							vertex.tangent.x = (float)tangent[0];
							vertex.tangent.y = (float)tangent[1];
							vertex.tangent.z = (float)tangent[2];

#if CONVERT_TO_LEFT_HANDED_MESH == 1
							vertex.tangent.z = -vertex.tangent.z;
#endif

							VertexComponentMask |= VCM_Tangent;
						}
						break;
					}
				}

				for (int uvLayer = 0; uvLayer < 2; uvLayer++)
				{
					if (uvArray[uvLayer] && hasPerPolygonVertexUV[uvLayer])
					{
						int idxUV = idxPoly * 3 + idxVert;
						if (uvArray[uvLayer]->GetReferenceMode() != FbxGeometryElement::eDirect)
						{
							idxUV = uvArray[uvLayer]->GetIndexArray().GetAt(idxPoly * 3 + idxVert);
						}

						FbxVector2 uv = uvArray[uvLayer]->GetDirectArray().GetAt(idxUV);

						RVertexType::Vec2Data& vertUV = uvLayer == 0 ? vertex.uv0 : vertex.uv1;

						vertUV.x = (float)uv[0];
						vertUV.y = 1.0f - (float)uv[1];

						if (uvLayer == 0)
							VertexComponentMask |= VCM_UV0;
						else
							VertexComponentMask |= VCM_UV1;
					}
				}

				flatVertData.push_back(vertex);
			}

			// Change triangle clockwise if necessary
#if CONVERT_TO_LEFT_HANDED_MESH == 1
			indexData.push_back(triangle[0]);
			indexData.push_back(triangle[2]);
			indexData.push_back(triangle[1]);
#else
			indexData.push_back(triangle[0]);
			indexData.push_back(triangle[1]);
			indexData.push_back(triangle[2]);
#endif
		}

		// Load textures
		int NumFbxMaterials = node->GetSrcObjectCount<FbxSurfaceMaterial>();

		for (int idxMat = 0; idxMat < NumFbxMaterials; idxMat++)
		{
			RMaterial meshMaterial = { 0 };
			FbxSurfaceMaterial* material = (FbxSurfaceMaterial*)node->GetSrcObject<FbxSurfaceMaterial>(idxMat);

			const char* texType[] =
			{
				FbxSurfaceMaterial::sDiffuse,
				FbxSurfaceMaterial::sNormalMap,
				FbxSurfaceMaterial::sSpecular,
			};

			for (int idxTexProp = 0; idxTexProp < 3; idxTexProp++)
			{
				FbxProperty prop = material->FindProperty(texType[idxTexProp]);
				int layeredTexCount = prop.GetSrcObjectCount<FbxLayeredTexture>();
				string textureName;

				for (int idxLayeredTex = 0; idxLayeredTex < layeredTexCount; idxLayeredTex++)
				{
					FbxLayeredTexture* layeredTex = FbxCast<FbxLayeredTexture>(prop.GetSrcObject<FbxLayeredTexture>(idxLayeredTex));
					int texCount = layeredTex->GetSrcObjectCount<FbxTexture>();

					for (int idxTex = 0; idxTex < texCount; idxTex++)
					{
						FbxFileTexture* texture = FbxCast<FbxFileTexture>(layeredTex->GetSrcObject<FbxTexture>(idxTex));
						textureName = texture->GetFileName();
					}
				}

				if (layeredTexCount == 0)
				{
					int texCount = prop.GetSrcObjectCount<FbxTexture>();

					for (int idxTex = 0; idxTex < texCount; idxTex++)
					{
						FbxFileTexture* texture = FbxCast<FbxFileTexture>(prop.GetSrcObject<FbxTexture>(idxTex));
						textureName = texture->GetFileName();
					}
				}

				if (textureName.length() != 0)
				{
					if (!RFileUtil::CheckIsRelativePath(textureName))
					{
						textureName = RFileUtil::GetFileNameInPath(textureName);
					}

					string ddsFilename = RFileUtil::ReplaceExtension(textureName, "dds");
					RTexture* texture = RResourceManager::Instance().FindTexture(ddsFilename.data());

					if (!texture)
					{
						texture = RResourceManager::Instance().LoadDDSTexture(RResourceManager::GetResourcePath(ddsFilename).data(), EResourceLoadMode::Immediate);
					}

					meshMaterial.Textures[meshMaterial.TextureNum] = texture;
					meshMaterial.TextureNum++;
				}
			}

			materials.push_back(meshMaterial);
		}

		if (NumFbxMaterials == 0)
			materials.push_back(RMaterial{ 0 });

		// Optimize mesh
		RLog("Optimizing mesh...\n");

		map<RVertexType::MeshLoader, int> meshVertIndexTable;
		vector<RVertexType::MeshLoader> optimizedVertData;
		vector<UINT> optimizedIndexData;
		UINT index = 0;
		for (UINT i = 0; i < indexData.size(); i++)
		{
			RVertexType::MeshLoader& v = flatVertData[indexData[i]];
			auto iterResult = meshVertIndexTable.find(v);
			if (iterResult == meshVertIndexTable.end())
			{
				meshVertIndexTable[v] = index;
				optimizedVertData.push_back(v);
				optimizedIndexData.push_back(index);
				index++;
			}
			else
			{
				optimizedIndexData.push_back(iterResult->second);
			}
		}

		// Hack: don't use uv1 on skinned mesh
		if (hasDeformer)
			VertexComponentMask &= ~VCM_UV1;

		RMeshElement meshElem;

		meshElem.SetVertices(optimizedVertData, VertexComponentMask);
		meshElem.SetTriangles(optimizedIndexData);
		meshElem.UpdateRenderBuffer();
		meshElem.SetName(node->GetName());

		UINT flag = 0;
		if (hasDeformer)
			flag |= MEF_Skinned;

		meshElem.SetFlag(flag);

		meshElements.push_back(meshElem);

		RLog("Mesh loaded with %d vertices and %d triangles (unoptimized: vert %d, triangle %d).\n",
			(int)optimizedVertData.size(), (int)optimizedIndexData.size() / 3, (int)flatVertData.size(), (int)indexData.size() / 3);
	}

	lFbxScene->Destroy();
	lFbxSdkManager->Destroy();

	// Load material from file
	string mtlFilename = RFileUtil::ReplaceExtension(task->Filename, "rmtl");
	LoadMeshMaterials(mtlFilename, materials);

	RMesh* MeshResource = static_cast<RMesh*>(task->Resource);

	MeshResource->SetMeshElements(meshElements.data(), (UINT)meshElements.size());
	MeshResource->SetMaterials(materials.data(), (UINT)materials.size());
	MeshResource->UpdateAabb();
	MeshResource->SetAnimation(animation);
	MeshResource->SetBoneNameList(meshBoneIdToName);
	MeshResource->SetBoneInitInvMatrices(boneInitInvPose);

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
	RMaterial::LoadFromXMLFile(mtlFilename, materials);
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

	RTexture* tex = new RTexture("[Internal]", srv);
	tex->OnLoadingFinished(false);
	m_WrapperTextureResources[srv] = tex;
	return tex;
}

