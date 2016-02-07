//=============================================================================
// RSMeshObject.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#include "RSMeshObject.h"

#include <fbxsdk.h>

#include <map>
#include <algorithm>
using namespace std;

struct MESH_VERTEX
{
	XMFLOAT3 pos;
	XMFLOAT2 uv;
	XMFLOAT3 normal;

	bool operator<(const MESH_VERTEX& rhs) const
	{
		return lexicographical_compare((const float*)this, (const float*)this + 8, (const float*)&rhs, (const float*)&rhs + 8);
	}
};

RSMeshObject::RSMeshObject()
	: RSceneObject(),
	  m_InputLayout(nullptr)
{

}

RSMeshObject::~RSMeshObject()
{
	for (UINT32 i = 0; i < m_MeshElements.size(); i++)
	{
		m_MeshElements[i].Release();
	}
}

void RSMeshObject::LoadFbxMesh(const char* filename, ID3D11InputLayout* inputLayout)
{
	m_InputLayout = inputLayout;

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
	bool lImportStatus = lImporter->Initialize(filename, -1, lFbxSdkManager->GetIOSettings());

	if (!lImportStatus) {
		printf("Call to FbxImporter::Initialize() failed.\n");
		printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
		exit(-1);
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
		char msg_buf[1024];
		sprintf_s(msg_buf, sizeof(msg_buf), "Loading FBX node [%d/%d]...\n", idxNode + 1, nodeCount);
		OutputDebugStringA(msg_buf);

		FbxNode* node = lFbxScene->GetNode(idxNode);
		FbxMesh* mesh = node->GetMesh();

		if (!mesh)
			continue;

		//mesh->SplitPoints();
		sprintf_s(msg_buf, sizeof(msg_buf), "[%s]\n", node->GetName());
		OutputDebugStringA(msg_buf);
		
		//int matCount = node->GetSrcObjectCount<FbxSurfaceMaterial>();
		//for (int idxMat = 0; idxMat < matCount; idxMat++)
		//{
		//	FbxSurfaceMaterial* material = (FbxSurfaceMaterial*)node->GetSrcObject<FbxSurfaceMaterial>(idxMat);

		//	if (material)
		//	{
		//		FbxProperty prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);

		//		int layerTexCount = prop.GetSrcObjectCount<FbxLayeredTexture>();

		//		for (int idxLayerTex = 0; idxLayerTex < layerTexCount; idxLayerTex++)
		//		{

		//		}
		//	}
		//}

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

		FbxGeometryElementUV* uvArray = mesh->GetElementUV();
		bool hasPerPolygonVertexUV = (uvArray->GetMappingMode() == FbxGeometryElement::eByPolygonVertex);

		FbxStringList uvSetNames;
		mesh->GetUVSetNames(uvSetNames);
		const char* uvSetName = uvSetNames.GetCount() > 0 ? uvSetNames[0] : nullptr;

		if (!hasPerPolygonVertexUV)
		{
			switch (uvArray->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
				for (int i = 0; i < controlPointCount; i++)
				{
					FbxVector2 uv = uvArray->GetDirectArray().GetAt(i);

					vertData[i].uv.x = (float)uv[0];
					vertData[i].uv.y = 1.0f - (float)uv[1];
				}
				break;

			case FbxGeometryElement::eIndexToDirect:
				for (int i = 0; i < controlPointCount; i++)
				{
					int index = uvArray->GetIndexArray().GetAt(i);
					FbxVector2 uv = uvArray->GetDirectArray().GetAt(index);

					vertData[i].uv.x = (float)uv[0];
					vertData[i].uv.y = 1.0f - (float)uv[1];
				}
				break;
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
					//int idxNormal = idxPoly * 3 + idxVert;
					//if (uvArray->GetReferenceMode() != FbxGeometryElement::eDirect)
					//{
					//	idxNormal = normalArray->GetIndexArray().GetAt(idxPoly * 3 + idxVert);
					//}

					//FbxVector4 normal = normalArray->GetDirectArray().GetAt(idxNormal);

					FbxVector4 normal;
					mesh->GetPolygonVertexNormal(idxPoly, idxVert, normal);

					vertex.normal.x = (float)normal[0];
					vertex.normal.y = (float)normal[1];
					vertex.normal.z = (float)normal[2];

					//sprintf_s(msg_buf, "Get normal [%f, %f, %f]\n", vertex.normal.x, vertex.normal.y, vertex.normal.z);
					//OutputDebugStringA(msg_buf);
				}

				if (hasPerPolygonVertexUV)
				{
					int idxUV = idxPoly * 3 + idxVert;
					if (uvArray->GetReferenceMode() != FbxGeometryElement::eDirect)
					{
						idxUV = uvArray->GetIndexArray().GetAt(idxPoly * 3 + idxVert);
					}

					FbxVector2 uv = uvArray->GetDirectArray().GetAt(idxUV);

					vertex.uv.x = (float)uv[0];
					vertex.uv.y = 1.0f - (float)uv[1];
				}

				flatVertData.push_back(vertex);
			}

			// Change triangle clockwise if necessary
			indexData.push_back(triangle[0]);
			indexData.push_back(triangle[1]);
			indexData.push_back(triangle[2]);
		}

		// Optimize mesh
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
		m_MeshElements.push_back(meshElem);

		sprintf_s(msg_buf, "Mesh loaded with %d vertices and %d triangles (unoptimized: vert %d, triangle %d).\n",
			optimizedVertData.size(), optimizedIndexData.size() / 3, flatVertData.size(), indexData.size() / 3);
		OutputDebugStringA(msg_buf);
	}

	lFbxScene->Destroy();
	lFbxSdkManager->Destroy();
}

int RSMeshObject::GetSubmeshCount() const
{
	return (int)m_MeshElements.size();
}

void RSMeshObject::SetMaterial(RMaterial* materials, int materialNum)
{
	m_Materials.clear();

	for (int i = 0; i < materialNum; i++)
	{
		m_Materials.push_back(materials[i]);
	}
}

RMaterial RSMeshObject::GetMaterial(int index) const
{
	return m_Materials[index];
}

void RSMeshObject::Draw()
{
	RRenderer.D3DImmediateContext()->IASetInputLayout(m_InputLayout);

	for (UINT32 i = 0; i < m_MeshElements.size(); i++)
	{
		if (m_Materials[i].Shader)
		{
			m_Materials[i].Shader->Bind();
			for (int t = 0; t < m_Materials[i].TextureNum; t++)
			{
				RRenderer.D3DImmediateContext()->PSSetShaderResources(t, 1, &m_Materials[i].Textures[t]);
			}
		}

		m_MeshElements[i].Draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
}
