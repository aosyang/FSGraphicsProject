//=============================================================================
// RResourceManager.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RResourceManager.h"

struct MESH_VERTEX
{
	RVec3 pos;
	RVec2 uv0;
	RVec3 normal;
	RVec3 tangent;
	RVec2 uv1;

	bool operator<(const MESH_VERTEX& rhs) const
	{
		return lexicographical_compare((const float*)this, (const float*)this + sizeof(MESH_VERTEX) / 4, (const float*)&rhs, (const float*)&rhs + sizeof(MESH_VERTEX) / 4);
	}
};

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

void RResourceManager::UnloadAllResources()
{
	for (UINT32 i = 0; i < m_MeshResources.size(); i++)
	{
		delete m_MeshResources[i];
	}
	m_MeshResources.clear();

	for (UINT32 i = 0; i < m_TextureResources.size(); i++)
	{
		delete m_TextureResources[i];
	}
	m_TextureResources.clear();

	map<ID3D11ShaderResourceView*, RTexture*>::iterator iter;
	for (iter = m_WrapperTextureResources.begin(); iter != m_WrapperTextureResources.end(); iter++)
	{
		// Delete wrapper textures without releasing shader resource view
		iter->second->SetSRV(nullptr);
		delete iter->second;
	}
	m_WrapperTextureResources.clear();
}

RMesh* RResourceManager::LoadFbxMesh(const char* filename, ID3D11InputLayout* inputLayout)
{
	RMesh* pMesh = new RMesh(GetResourcePath(filename), inputLayout);
	pMesh->m_State = RS_EnqueuedForLoading;
	m_MeshResources.push_back(pMesh);

	LoaderThreadTask task;
	task.Filename = string(filename);
	task.Resource = pMesh;

	// Upload task to working queue
	m_TaskQueueMutex.lock();
	m_LoaderThreadTaskQueue.push(task);
	m_TaskQueueMutex.unlock();

	// Notify loader thread to start working
	m_TaskQueueCondition.notify_all();

	return pMesh;
}

void RResourceManager::ThreadLoadFbxMeshData(LoaderThreadTask* task)
{
	vector<RMeshElement> meshElements;
	vector<RMaterial> materials;

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

	// Load meshes

	int nodeCount = lFbxScene->GetNodeCount();
	for (int idxNode = 0; idxNode < nodeCount; idxNode++)
	{
		sprintf_s(msg_buf, sizeof(msg_buf), "Loading FBX node [%d/%d]...\n", idxNode + 1, nodeCount);
		OutputDebugStringA(msg_buf);

		FbxNode* node = lFbxScene->GetNode(idxNode);
		FbxMesh* mesh = node->GetMesh();

		if (!mesh)
			continue;

		//mesh->SplitPoints();
		sprintf_s(msg_buf, sizeof(msg_buf), "[%s]\n", node->GetName());
		OutputDebugStringA(msg_buf);
		
		FbxVector4* controlPointArray;
		vector<MESH_VERTEX> vertData;
		vector<int> indexData;
		vector<MESH_VERTEX> flatVertData;

		controlPointArray = mesh->GetControlPoints();
		int controlPointCount = mesh->GetControlPointsCount();

		vertData.resize(controlPointCount);

		// Fill vertex data
		for (int i = 0; i < controlPointCount; i++)
		{
			vertData[i].pos.x = (float)controlPointArray[i][0];
			vertData[i].pos.y = (float)controlPointArray[i][1];
			vertData[i].pos.z = (float)controlPointArray[i][2];
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
				}
				break;
			}
			break;

		case FbxGeometryElement::eByPolygonVertex:
			switch (normalArray->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
				// TODO
				break;

			case FbxGeometryElement::eIndexToDirect:
				// TODO
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
					}
					break;
				}
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

				MESH_VERTEX vertex = vertData[iv];
				
				if (hasPerPolygonVertexNormal)
				{
					FbxVector4 normal;
					mesh->GetPolygonVertexNormal(idxPoly, idxVert, normal);

					vertex.normal.x = (float)normal[0];
					vertex.normal.y = (float)normal[1];
					vertex.normal.z = (float)normal[2];
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
					}
					break;
					case FbxGeometryElement::eIndexToDirect:
					{
						int index = tangentArray->GetIndexArray().GetAt(idxPoly * 3 + idxVert);
						FbxVector4 tangent = tangentArray->GetDirectArray().GetAt(index);

						vertex.tangent.x = (float)tangent[0];
						vertex.tangent.y = (float)tangent[1];
						vertex.tangent.z = (float)tangent[2];
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
		RMaterial meshMaterial = { 0 };
		int matCount = node->GetSrcObjectCount<FbxSurfaceMaterial>();

		for (int idxMat = 0; idxMat < matCount; idxMat++)
		{
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
						meshMaterial.Textures[meshMaterial.TextureNum] = texture;
						meshMaterial.TextureNum++;
					}
				}
			}
		}

		materials.push_back(meshMaterial);

		// Optimize mesh
		sprintf_s(msg_buf, "Optimizing mesh...\n");
		OutputDebugStringA(msg_buf);
		map<MESH_VERTEX, int> meshVertIndexTable;
		vector<MESH_VERTEX> optimizedVertData;
		vector<int> optimizedIndexData;
		int index = 0;
		for (UINT i = 0; i < indexData.size(); i++)
		{
			MESH_VERTEX& v = flatVertData[indexData[i]];
			map<MESH_VERTEX, int>::iterator iterResult = meshVertIndexTable.find(v);
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

		RMeshElement meshElem;
		meshElem.CreateVertexBuffer(optimizedVertData.data(), sizeof(MESH_VERTEX), optimizedVertData.size());
		meshElem.CreateIndexBuffer(optimizedIndexData.data(), sizeof(UINT32), optimizedIndexData.size());
		meshElements.push_back(meshElem);

		sprintf_s(msg_buf, "Mesh loaded with %d vertices and %d triangles (unoptimized: vert %d, triangle %d).\n",
			optimizedVertData.size(), optimizedIndexData.size() / 3, flatVertData.size(), indexData.size() / 3);
		OutputDebugStringA(msg_buf);
	}

	lFbxScene->Destroy();
	lFbxSdkManager->Destroy();

	static_cast<RMesh*>(task->Resource)->SetMeshElements(meshElements.data(), meshElements.size());
	static_cast<RMesh*>(task->Resource)->SetMaterials(materials.data(), materials.size());
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
	m_TextureResources.push_back(pTexture);

	LoaderThreadTask task;
	task.Filename = string(filename);
	task.Resource = pTexture;

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

	DirectX::CreateDDSTextureFromFile(RRenderer.D3DDevice(), wszName, nullptr, &srv);

	static_cast<RTexture*>(task->Resource)->SetSRV(srv);
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
	for (vector<RTexture*>::iterator iter = m_TextureResources.begin(); iter != m_TextureResources.end(); iter++)
	{
		if (strcasecmp((*iter)->GetPath().data(), resourcePath) == 0)
		{
			return *iter;
		}
	}

	return nullptr;
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

