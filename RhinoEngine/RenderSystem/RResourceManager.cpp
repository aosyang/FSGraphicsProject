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

#include "tinyxml2/tinyxml2.h"

#define ENABLE_THREADED_LOADING 1

static mutex								m_TaskQueueMutex;
static condition_variable					m_TaskQueueCondition;

static mutex	TextureResourcesMutex;

void ResourceLoaderThread(LoaderThreadData* data)
{
	while (1)
	{
		LoaderThreadTask task;

		{
			unique_lock<mutex> uniqueLock(*data->TaskQueueMutex);
			data->TaskQueueCondition->wait(uniqueLock, [&]{ return data->TaskQueue->size() != 0 || *data->ShouldQuitThread; });

			if (*data->ShouldQuitThread)
				break;

			task = data->TaskQueue->front();
			data->TaskQueue->pop();
		}

		// Load resource in this task
		switch (task.Resource->GetResourceType())
		{
		case RT_Mesh:
			RResourceManager::ThreadLoadFbxMeshData(&task);
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
}

void RResourceManager::Initialize()
{
	// Create resource loader thread
	m_ShouldQuitLoaderThread = false;

	m_LoaderThreadData.TaskQueue = &m_LoaderThreadTaskQueue;
	m_LoaderThreadData.TaskQueueMutex = &m_TaskQueueMutex;
	m_LoaderThreadData.TaskQueueCondition = &m_TaskQueueCondition;
	m_LoaderThreadData.ShouldQuitThread = &m_ShouldQuitLoaderThread;

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

				string lowerExt = resName.substr(resName.size() - 4);
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
			}
			else
			{
				if (FindFileData.cFileName[0] != '.')
				{
					dir_queue.push(string(FindFileData.cFileName) + "/");
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
	map<ID3D11ShaderResourceView*, RTexture*>::iterator iter;
	for (iter = m_WrapperTextureResources.begin(); iter != m_WrapperTextureResources.end(); iter++)
	{
		// Delete wrapper textures without releasing shader resource view
		iter->second->m_SRV = nullptr;
		delete iter->second;
	}
	m_WrapperTextureResources.clear();
}

RMesh* RResourceManager::LoadFbxMesh(const char* filename, ResourceLoadingMode mode)
{
	RMesh* pMesh = new RMesh(GetResourcePath(filename));
	pMesh->m_State = RS_EnqueuedForLoading;
	m_MeshResources.push_back(pMesh);

	LoaderThreadTask task;
	task.Filename = string(filename);
	task.Resource = pMesh;

#if (ENABLE_THREADED_LOADING == 0)
	mode = RLM_Immediate;
#endif

	if (mode == RLM_Immediate)
	{
		ThreadLoadFbxMeshData(&task);
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
	RAabb mesh_aabb;

	char msg_buf[1024];
	sprintf_s(msg_buf, sizeof(msg_buf), "Loading mesh [%s]...\n", task->Filename.data());
	OutputDebugStringA(msg_buf);

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
		printf("Call to FbxImporter::Initialize() failed.\n");
		printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());

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

	// Load bone information into an array
	for (int idxNode = 0; idxNode < lFbxScene->GetNodeCount(); idxNode++)
	{
		FbxNode* node = lFbxScene->GetNode(idxNode);
		if (node->GetNodeAttribute() && node->GetNodeAttribute()->GetAttributeType() && node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
		{
			fbxBoneNodes.push_back(node);
			meshBoneIdToName.push_back(node->GetName());

			sprintf_s(msg_buf, sizeof(msg_buf), "  FBX bone node: %s\n", node->GetName());
			OutputDebugStringA(msg_buf);
		}
	}

	// Load animation
	RAnimation* animation = new RAnimation();
	string animFilename = task->Filename.substr(0, task->Filename.size() - 3) + "ranim";

	if (!animation->LoadFromFile(animFilename.c_str()))
	{
		SAFE_DELETE(animation);

		FbxArray<FbxString*> animStackNameArray;
		lFbxScene->FillAnimStackNameArray(animStackNameArray);
		if (animStackNameArray.GetCount() > 0)
		{
			FbxAnimStack* animStack = lFbxScene->FindMember<FbxAnimStack>(animStackNameArray[0]->Buffer());
			FbxTakeInfo* takeInfo = lFbxScene->GetTakeInfo(*(animStackNameArray[0]));

			FbxArrayDelete(animStackNameArray);

			FbxTime::EMode				animTimeMode;
			FbxTime						frameTime, animStartTime, animEndTime;
			float						animFrameRate;

			frameTime.SetTime(0, 0, 0, 1, 0, lFbxScene->GetGlobalSettings().GetTimeMode());
			animTimeMode = lFbxScene->GetGlobalSettings().GetTimeMode();
			animFrameRate = (float)frameTime.GetFrameRate(animTimeMode);
			animStartTime = takeInfo->mLocalTimeSpan.GetStart();
			animEndTime = takeInfo->mLocalTimeSpan.GetStop();

			int totalFrameCount = (int)(animEndTime.GetFrameCount(animTimeMode) - animStartTime.GetFrameCount(animTimeMode)) + 1;
			animation = new RAnimation(
				lFbxScene->GetNodeCount(),
				totalFrameCount,
				(float)animStartTime.GetFrameCountPrecise(animTimeMode),
				(float)animEndTime.GetFrameCountPrecise(animTimeMode),
				animFrameRate);

			map<string, int> nodeNameToId;

			for (int idxNode = 0; idxNode < lFbxScene->GetNodeCount(); idxNode++)
			{
				FbxNode* node = lFbxScene->GetNode(idxNode);

				for (FbxTime animTime = animStartTime;
					animTime <= animEndTime;
					animTime += frameTime)
				{
					RMatrix4 matrix;

					FbxAMatrix childTransform = node->EvaluateGlobalTransform(animTime);
					MatrixTransfer(&matrix, &childTransform);
					int frameIdx = (int)((float)animTime.GetFrameCountPrecise(animTimeMode) - (float)animStartTime.GetFrameCountPrecise(animTimeMode));
					animation->AddNodePose(idxNode, frameIdx, &matrix);
					animation->AddNodeNameToId(node->GetName(), idxNode);

					nodeNameToId[node->GetName()] = idxNode;
				}
			}

			for (int idxNode = 0; idxNode < lFbxScene->GetNodeCount(); idxNode++)
			{
				FbxNode* node = lFbxScene->GetNode(idxNode);
				FbxNode* parent = node->GetParent();
				if (parent && nodeNameToId.find(parent->GetName()) != nodeNameToId.end())
				{
					animation->SetParentId(idxNode, nodeNameToId[parent->GetName()]);
				}
				else
				{
					animation->SetParentId(idxNode, -1);
				}
			}
		}
	}

	BoneMatrices* boneInitInvPose = nullptr;

	// Load meshes
	int nodeCount = lFbxScene->GetNodeCount();
	for (int idxNode = 0; idxNode < nodeCount; idxNode++)
	{
		sprintf_s(msg_buf, sizeof(msg_buf), "  FBX node [%d/%d]...\n", idxNode + 1, nodeCount);
		OutputDebugStringA(msg_buf);

		FbxNode* node = lFbxScene->GetNode(idxNode);
		FbxMesh* mesh = node->GetMesh();

		if (!mesh)
			continue;

		//mesh->SplitPoints();
		sprintf_s(msg_buf, sizeof(msg_buf), "  Mesh element [%s]\n", node->GetName());
		OutputDebugStringA(msg_buf);
		
		FbxVector4* controlPointArray;
		vector<RVertex::MESH_LOADER_VERTEX> vertData;
		vector<int> indexData;
		vector<RVertex::MESH_LOADER_VERTEX> flatVertData;
		int VertexComponentMask = 0;

		controlPointArray = mesh->GetControlPoints();
		int controlPointCount = mesh->GetControlPointsCount();

		vertData.resize(controlPointCount);

		RAabb aabb;

		// Fill vertex data
		for (int i = 0; i < controlPointCount; i++)
		{
			vertData[i].pos.x = (float)controlPointArray[i][0];
			vertData[i].pos.y = (float)controlPointArray[i][1];
			vertData[i].pos.z = (float)controlPointArray[i][2];

			memset(vertData[i].boneId, -1, sizeof(int) * 4);
			memset(vertData[i].weight, 0, sizeof(float) * 4);

			VertexComponentMask |= VCM_Pos;

			aabb.Expand(vertData[i].pos);
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

					VertexComponentMask |= VCM_Normal;
				}
				break;
			}
			break;
		}

		// Tangent
		FbxGeometryElementTangent* tangentArray = mesh->GetElementTangent();
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
						RVec2& vertUV = uvLayer == 0 ? vertData[i].uv0 : vertData[i].uv1;
							
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
						RVec2& vertUV = uvLayer == 0 ? vertData[i].uv0 : vertData[i].uv1;
						
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

					int boneId = std::find(fbxBoneNodes.begin(), fbxBoneNodes.end(), cluster->GetLink()) - fbxBoneNodes.begin();
					assert(boneId < MAX_BONE_COUNT);

					// Store inversed initial transform for each bone to apply skinning with correct binding pose
					FbxAMatrix clusterInitTransform;
					cluster->GetTransformLinkMatrix(clusterInitTransform);
					clusterInitTransform = clusterInitTransform.Inverse();

					if (!boneInitInvPose)
						boneInitInvPose = new BoneMatrices;
					MatrixTransfer(&boneInitInvPose->boneMatrix[boneId], &clusterInitTransform);

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

				RVertex::MESH_LOADER_VERTEX vertex = vertData[iv];
				
				if (hasPerPolygonVertexNormal)
				{
					FbxVector4 normal;
					mesh->GetPolygonVertexNormal(idxPoly, idxVert, normal);

					vertex.normal.x = (float)normal[0];
					vertex.normal.y = (float)normal[1];
					vertex.normal.z = (float)normal[2];

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

						RVec2& vertUV = uvLayer == 0 ? vertex.uv0 : vertex.uv1;

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
			indexData.push_back(triangle[0]);
			indexData.push_back(triangle[1]);
			indexData.push_back(triangle[2]);
		}

		// Load textures
		int matCount = node->GetSrcObjectCount<FbxSurfaceMaterial>();

		for (int idxMat = 0; idxMat < matCount; idxMat++)
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
					string ddsFilename = textureName;
					size_t pos = textureName.find_last_of("/\\");
					if (pos != string::npos)
					{
						ddsFilename = textureName.substr(pos + 1);
					}

					pos = ddsFilename.find_last_of(".");
					if (pos != string::npos)
					{
						ddsFilename = ddsFilename.substr(0, pos);
					}

					ddsFilename += ".dds";

					RTexture* texture = RResourceManager::Instance().FindTexture(ddsFilename.data());

					if (!texture)
					{
						texture = RResourceManager::Instance().LoadDDSTexture(RResourceManager::GetResourcePath(ddsFilename).data(), RLM_Immediate);
					}

					meshMaterial.Textures[meshMaterial.TextureNum] = texture;
					meshMaterial.TextureNum++;
				}
			}

			materials.push_back(meshMaterial);
		}

		if (matCount == 0)
			materials.push_back(RMaterial{ 0 });

		// Optimize mesh
		sprintf_s(msg_buf, "Optimizing mesh...\n");
		OutputDebugStringA(msg_buf);
		map<RVertex::MESH_LOADER_VERTEX, int> meshVertIndexTable;
		vector<RVertex::MESH_LOADER_VERTEX> optimizedVertData;
		vector<int> optimizedIndexData;
		int index = 0;
		for (UINT i = 0; i < indexData.size(); i++)
		{
			RVertex::MESH_LOADER_VERTEX& v = flatVertData[indexData[i]];
			map<RVertex::MESH_LOADER_VERTEX, int>::iterator iterResult = meshVertIndexTable.find(v);
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

		int stride = RVertexDeclaration::Instance().GetVertexStride(VertexComponentMask);
		ID3D11InputLayout* inputLayout = RVertexDeclaration::Instance().GetInputLayoutByVertexComponents(VertexComponentMask);
		BYTE* compactVertexData = new BYTE[optimizedVertData.size() * stride];
		RVertexDeclaration::Instance().CopyVertexComponents(compactVertexData, optimizedVertData.data(), (int)optimizedVertData.size(), VertexComponentMask);

		meshElem.CreateVertexBuffer(compactVertexData, stride, optimizedVertData.size(), inputLayout);

		delete[] compactVertexData;

		meshElem.CreateIndexBuffer(optimizedIndexData.data(), sizeof(UINT32), optimizedIndexData.size());
		meshElem.SetName(node->GetName());
		meshElem.SetAabb(aabb);

		UINT flag = 0;
		if (hasDeformer)
			flag |= MEF_Skinned;

		meshElem.SetFlag(flag);

		meshElements.push_back(meshElem);


		mesh_aabb.Expand(aabb);

		sprintf_s(msg_buf, "Mesh loaded with %d vertices and %d triangles (unoptimized: vert %d, triangle %d).\n",
			optimizedVertData.size(), optimizedIndexData.size() / 3, flatVertData.size(), indexData.size() / 3);
		OutputDebugStringA(msg_buf);
	}

	lFbxScene->Destroy();
	lFbxSdkManager->Destroy();

	// Load material from file
	string mtlFilename = task->Filename.substr(0, task->Filename.length() - 3) + "rmtl";
	tinyxml2::XMLDocument* doc = new tinyxml2::XMLDocument();
	if (doc->LoadFile(mtlFilename.c_str()) == tinyxml2::XML_SUCCESS)
	{
		vector<RMaterial> xmlMaterials;

		tinyxml2::XMLElement* root = doc->RootElement();
		tinyxml2::XMLElement* elem = root->FirstChildElement("MeshElement");
		while (elem)
		{
			const char* shaderName = elem->Attribute("Shader");
			RMaterial material = { nullptr, 0 };
			material.Shader = RShaderManager::Instance().GetShaderResource(shaderName);

			tinyxml2::XMLElement* elem_tex = elem->FirstChildElement();
			while (elem_tex)
			{
				const char* textureName = elem_tex->GetText();

				RTexture* texture = RResourceManager::Instance().FindTexture(textureName);

				if (!texture)
				{
					texture = RResourceManager::Instance().LoadDDSTexture(RResourceManager::GetResourcePath(textureName).data(), RLM_Immediate);
				}

				material.Textures[material.TextureNum++] = texture;
				elem_tex = elem_tex->NextSiblingElement();
			}

			xmlMaterials.push_back(material);
			elem = elem->NextSiblingElement();
		}

		if (xmlMaterials.size())
		{
			materials = xmlMaterials;
		}
	}
	delete doc;

	static_cast<RMesh*>(task->Resource)->SetMeshElements(meshElements.data(), meshElements.size());
	static_cast<RMesh*>(task->Resource)->SetMaterials(materials.data(), materials.size());
	static_cast<RMesh*>(task->Resource)->SetAabb(mesh_aabb);
	static_cast<RMesh*>(task->Resource)->SetAnimation(animation);
	static_cast<RMesh*>(task->Resource)->SetBoneNameList(meshBoneIdToName);
	static_cast<RMesh*>(task->Resource)->SetBoneInitInvMatrices(boneInitInvPose);
	static_cast<RMesh*>(task->Resource)->SetResourceTimestamp(REngine::GetTimer().TotalTime());
	task->Resource->m_State = RS_Loaded;
}

RTexture* RResourceManager::LoadDDSTexture(const char* filename, ResourceLoadingMode mode)
{
	RTexture* pTexture = FindTexture(filename);
	if (pTexture)
		return pTexture;

	pTexture = new RTexture(GetResourcePath(filename));
	pTexture->m_State = RS_EnqueuedForLoading;

	TextureResourcesMutex.lock();
	m_TextureResources.push_back(pTexture);
	TextureResourcesMutex.unlock();

	LoaderThreadTask task;
	task.Filename = string(filename);
	task.Resource = pTexture;

#if (ENABLE_THREADED_LOADING == 0)
	mode = RLM_Immediate;
#endif

	if (mode == RLM_Immediate)
	{
		ThreadLoadDDSTextureData(&task);
	}
	else // mode == RLM_Threaded
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

	char msg_buf[1024];
	sprintf_s(msg_buf, sizeof(msg_buf), "Loading texture [%s]...\n", task->Filename.data());
	OutputDebugStringA(msg_buf);

	ID3D11Resource* pResource;

	HRESULT hr = DirectX::CreateDDSTextureFromFile(RRenderer.D3DDevice(), wszName, &pResource, &srv);
	if (FAILED(hr))
	{
		sprintf_s(msg_buf, sizeof(msg_buf), "*** Failed to load texture [%s] ***\n", task->Filename.data());
		OutputDebugStringA(msg_buf);
	}

	if (pResource)
	{
		ID3D11Texture2D* pTexture;
		pResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&pTexture);

		if (pTexture)
		{
			D3D11_TEXTURE2D_DESC desc;
			pTexture->GetDesc(&desc);

			static_cast<RTexture*>(task->Resource)->m_Width = desc.Width;
			static_cast<RTexture*>(task->Resource)->m_Height = desc.Height;
		}
	}

	static_cast<RTexture*>(task->Resource)->m_SRV = srv;
	task->Resource->m_State = RS_Loaded;
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
	for (vector<RTexture*>::iterator iter = m_TextureResources.begin(); iter != m_TextureResources.end(); iter++)
	{
		if (strcasecmp((*iter)->GetPath().data(), resourcePath) == 0)
		{
			return *iter;
		}
	}

	return nullptr;
}

RMesh* RResourceManager::FindMesh(const char* resourcePath)
{
	for (vector<RMesh*>::iterator iter = m_MeshResources.begin(); iter != m_MeshResources.end(); iter++)
	{
		if (strcasecmp((*iter)->GetPath().data(), resourcePath) == 0)
		{
			return *iter;
		}
	}

	return nullptr;
}

const vector<RMesh*>& RResourceManager::GetMeshResources() const
{
	return m_MeshResources;
}

string RResourceManager::GetAssetsBasePath()
{
	return string("../Assets/");
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
	tex->m_State = RS_Loaded;
	m_WrapperTextureResources[srv] = tex;
	return tex;
}

