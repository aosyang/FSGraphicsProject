//=============================================================================
// FSGraphicsProjectApp.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "FSGraphicsProjectApp.h"

#include <fbxsdk.h>

#include "Color_PS.csh"
#include "Color_VS.csh"
#include "Lighting_PS.csh"
#include "Lighting_VS.csh"

struct COLOR_VERTEX
{
	XMFLOAT4 pos;
	XMFLOAT4 color;
};

struct MESH_VERTEX
{
	XMFLOAT3 pos;
	XMFLOAT2 uv;
	XMFLOAT3 normal;
};

FSGraphicsProjectApp::FSGraphicsProjectApp()
	: m_ColorPrimitiveIL(nullptr),
	  m_ColorPixelShader(nullptr), m_ColorVertexShader(nullptr),
	  m_LightingMeshIL(nullptr),
	  m_LightingPixelShader(nullptr), m_LightingVertexShader(nullptr)
{
}


FSGraphicsProjectApp::~FSGraphicsProjectApp()
{
	for (vector<RMeshElement>::iterator iter = m_FbxMeshes.begin(); iter != m_FbxMeshes.end(); iter++)
	{
		iter->Release();
	}
	m_FbxMeshes.clear();

	SAFE_RELEASE(m_cbPerObject);
	SAFE_RELEASE(m_cbScene);

	SAFE_RELEASE(m_ColorPixelShader);
	SAFE_RELEASE(m_ColorVertexShader);
	SAFE_RELEASE(m_LightingMeshIL);

	SAFE_RELEASE(m_LightingPixelShader);
	SAFE_RELEASE(m_LightingVertexShader);

	m_StarMesh.Release();
	SAFE_RELEASE(m_ColorPrimitiveIL);
}

bool FSGraphicsProjectApp::Initialize()
{
	// Create buffer for star mesh
	COLOR_VERTEX starVertex[12];

	for (int i = 0; i < 10; i++)
	{
		float r = (i % 2 == 0) ? 1.0f : 0.5f;
		starVertex[i] = { XMFLOAT4(sinf(DEG_TO_RAD(i * 36)) * r, cosf(DEG_TO_RAD(i * 36)) * r, 0.0f, 1.0f),
			XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) };
	}

	starVertex[10] = { XMFLOAT4(0.0f, 0.0f, -0.2f, 1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) };
	starVertex[11] = { XMFLOAT4(0.0f, 0.0f, 0.2f, 1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) };

	UINT32 starIndex[] = {
		0, 1, 10, 1, 2, 10, 2, 3, 10, 3, 4, 10, 4, 5, 10, 5, 6, 10, 6, 7, 10, 7, 8, 10, 8, 9, 10, 9, 0, 10,
		1, 0, 11, 2, 1, 11, 3, 2, 11, 4, 3, 11, 5, 4, 11, 6, 5, 11, 7, 6, 11, 8, 7, 11, 9, 8, 11, 0, 9, 11, };

	m_StarMesh.CreateVertexBuffer(starVertex, sizeof(COLOR_VERTEX), 12);
	m_StarMesh.CreateIndexBuffer(starIndex, sizeof(UINT32), sizeof(starIndex) / sizeof(UINT32));

	D3D11_INPUT_ELEMENT_DESC colorVertDesc[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	RRenderer.D3DDevice()->CreateInputLayout(colorVertDesc, 2, Color_VS, sizeof(Color_VS), &m_ColorPrimitiveIL);

	RRenderer.D3DDevice()->CreateVertexShader(Color_VS, sizeof(Color_VS), NULL, &m_ColorVertexShader);
	RRenderer.D3DDevice()->CreatePixelShader(Color_PS, sizeof(Color_PS), NULL, &m_ColorPixelShader);

	D3D11_BUFFER_DESC cbPerObjectDesc;
	ZeroMemory(&cbPerObjectDesc, sizeof(cbPerObjectDesc));
	cbPerObjectDesc.ByteWidth = sizeof(XMFLOAT4X4);
	cbPerObjectDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbPerObjectDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbPerObjectDesc.Usage = D3D11_USAGE_DYNAMIC;

	RRenderer.D3DDevice()->CreateBuffer(&cbPerObjectDesc, NULL, &m_cbPerObject);

	D3D11_BUFFER_DESC cbSceneDesc;
	ZeroMemory(&cbSceneDesc, sizeof(cbSceneDesc));
	cbSceneDesc.ByteWidth = sizeof(XMFLOAT4X4);
	cbSceneDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbSceneDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbSceneDesc.Usage = D3D11_USAGE_DYNAMIC;

	RRenderer.D3DDevice()->CreateBuffer(&cbSceneDesc, NULL, &m_cbScene);

	LoadFbxMesh("../Assets/city.fbx");

	return true;
}

void FSGraphicsProjectApp::LoadFbxMesh(char* filename)
{
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
	lGeomConverter.SplitMeshesPerMaterial(lFbxScene, true);

	// Load meshes
	for (int idxNode = 0; idxNode < lFbxScene->GetNodeCount(); idxNode++)
	{
		FbxNode* node = lFbxScene->GetNode(idxNode);
		FbxMesh* mesh = node->GetMesh();

		if (!mesh)
			continue;

		mesh->SplitPoints();

		FbxVector4* controlPointArray;
		vector<MESH_VERTEX> vertData;
		vector<int> indexData;

		controlPointArray = mesh->GetControlPoints();
		int controlPointCount = mesh->GetControlPointsCount();

		vertData.resize(controlPointCount);

		FbxGeometryElementNormal* normalArray = mesh->GetElementNormal();
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

		// Fill vertex data
		for (int i = 0; i < controlPointCount; i++)
		{
			vertData[i].pos.x = (float)controlPointArray[i][0];
			vertData[i].pos.y = (float)controlPointArray[i][1];
			vertData[i].pos.z = (float)controlPointArray[i][2];
		}

		// Fill triangle data
		int polyCount = mesh->GetPolygonCount();

		for (int idxPoly = 0; idxPoly < polyCount; idxPoly++)
		{
			// Loop through index buffer
			int* indexArray = mesh->GetPolygonVertices();
			int indexSize = mesh->GetPolygonSize(idxPoly);

			// Note: Assume mesh has been triangulated
			int triangle[3];

			for (int idxVert = 0; idxVert < mesh->GetPolygonSize(idxPoly); idxVert++)
			{
				triangle[idxVert] = mesh->GetPolygonVertex(idxPoly, idxVert);

				if (hasPerPolygonVertexUV)
				{
					int idxUV = idxPoly * 3 + idxVert;
					if (uvArray->GetReferenceMode() != FbxGeometryElement::eDirect)
					{
						idxUV = uvArray->GetIndexArray().GetAt(idxPoly * 3 + idxVert);
					}

					FbxVector2 uv = uvArray->GetDirectArray().GetAt(idxUV);

					vertData[triangle[idxVert]].uv.x = (float)uv[0];
					vertData[triangle[idxVert]].uv.y = 1.0f - (float)uv[1];
				}
			}

			// Change triangle clockwise if necessary
			indexData.push_back(triangle[0]);
			indexData.push_back(triangle[1]);
			indexData.push_back(triangle[2]);
		}

		RMeshElement meshElem;
		meshElem.CreateVertexBuffer(vertData.data(), sizeof(MESH_VERTEX), vertData.size());
		meshElem.CreateIndexBuffer(indexData.data(), sizeof(UINT32), indexData.size());
		m_FbxMeshes.push_back(meshElem);
	}

	lFbxScene->Destroy();
	lFbxSdkManager->Destroy();

	// Create mesh shader
	RRenderer.D3DDevice()->CreatePixelShader(Lighting_PS, sizeof(Lighting_PS), NULL, &m_LightingPixelShader);
	RRenderer.D3DDevice()->CreateVertexShader(Lighting_VS, sizeof(Lighting_VS), NULL, &m_LightingVertexShader);

	// Create input layout
	D3D11_INPUT_ELEMENT_DESC objVertDesc[] =
	{
		{ "POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",		0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",			0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	RRenderer.D3DDevice()->CreateInputLayout(objVertDesc, 3, Lighting_VS, sizeof(Lighting_VS), &m_LightingMeshIL);
}

void FSGraphicsProjectApp::UpdateScene(const RTimer& timer)
{
	XMMATRIX viewMatrix = XMMatrixLookAtLH(XMVectorSet(-500.0f, 500.0f, -500.0f, 1.0f), XMVectorZero(), XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f));
	//XMMatrixIdentity();
	XMMATRIX projMatrix = XMMatrixPerspectiveFovLH(90.0f, RRenderer.AspectRatio(), 0.1f, 1000.0f);
	XMFLOAT4X4 viewProj;
	XMStoreFloat4x4(&viewProj, viewMatrix * projMatrix);

	D3D11_MAPPED_SUBRESOURCE subres;
	RRenderer.D3DImmediateContext()->Map(m_cbScene, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
	memcpy(subres.pData, &viewProj, sizeof(viewProj));
	RRenderer.D3DImmediateContext()->Unmap(m_cbScene, 0);
}

void FSGraphicsProjectApp::RenderScene()
{
	RRenderer.Clear();

	RRenderer.D3DImmediateContext()->VSSetConstantBuffers(1, 1, &m_cbScene);

	// Set up object world matrix
	XMMATRIX worldMatrix = XMMatrixTranslation(0.0f, 0.0f, 2.0f);
	XMFLOAT4X4 world;
	XMStoreFloat4x4(&world, worldMatrix);

	D3D11_MAPPED_SUBRESOURCE subres;
	RRenderer.D3DImmediateContext()->Map(m_cbPerObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
	memcpy(subres.pData, &world, sizeof(world));
	RRenderer.D3DImmediateContext()->Unmap(m_cbPerObject, 0);

	// Draw star
	RRenderer.D3DImmediateContext()->VSSetConstantBuffers(0, 1, &m_cbPerObject);
	RRenderer.D3DImmediateContext()->PSSetShader(m_ColorPixelShader, NULL, 0);
	RRenderer.D3DImmediateContext()->VSSetShader(m_ColorVertexShader, NULL, 0);
	RRenderer.D3DImmediateContext()->IASetInputLayout(m_ColorPrimitiveIL);

	m_StarMesh.Draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Draw meshes
	RRenderer.D3DImmediateContext()->PSSetShader(m_LightingPixelShader, NULL, 0);
	RRenderer.D3DImmediateContext()->VSSetShader(m_LightingVertexShader, NULL, 0);
	RRenderer.D3DImmediateContext()->IASetInputLayout(m_LightingMeshIL);

	for (vector<RMeshElement>::iterator iter = m_FbxMeshes.begin(); iter != m_FbxMeshes.end(); iter++)
	{
		iter->Draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	RRenderer.Present();
}
