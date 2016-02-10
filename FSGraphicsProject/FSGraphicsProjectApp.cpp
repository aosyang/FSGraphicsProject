//=============================================================================
// FSGraphicsProjectApp.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "FSGraphicsProjectApp.h"

#include "ConstBufferPS.h"
#include "ConstBufferVS.h"

#include "Color_PS.csh"
#include "Color_VS.csh"
#include "Lighting_PS.csh"
#include "Lighting_VS.csh"
#include "Skybox_PS.csh"
#include "Skybox_VS.csh"
#include "BumpLighting_PS.csh"
#include "BumpLighting_VS.csh"
#include "InstancedLighting_PS.csh"
#include "InstancedLighting_VS.csh"
#include "Depth_PS.csh"
#include "Depth_VS.csh"
#include "InstancedDepth_PS.csh"
#include "InstancedDepth_VS.csh"

struct COLOR_VERTEX
{
	XMFLOAT4 pos;
	XMFLOAT4 color;
};

struct BUMP_MESH_VERTEX
{
	XMFLOAT3 pos;
	XMFLOAT2 uv;
	XMFLOAT3 normal;
	XMFLOAT3 tangent;
};

FSGraphicsProjectApp::FSGraphicsProjectApp()
	: m_ColorPrimitiveIL(nullptr),
	  m_ColorShader(nullptr),
	  m_LightingMeshIL(nullptr),
	  m_LightingShader(nullptr),
	  m_SamplerState(nullptr)
{
	m_MeshTextureSRV[0] = nullptr;
	m_MeshTextureSRV[1] = nullptr;
	m_MeshTextureSRV[2] = nullptr;

	m_EnableLights[0] = true;
	m_EnableLights[1] = true;
	m_EnableLights[2] = true;
}


FSGraphicsProjectApp::~FSGraphicsProjectApp()
{
	SAFE_RELEASE(m_IslandTextureSRV);

	SAFE_RELEASE(m_SamplerComparisonState);
	SAFE_RELEASE(m_SamplerState);
	SAFE_RELEASE(m_MeshTextureSRV[0]);
	SAFE_RELEASE(m_MeshTextureSRV[1]);
	SAFE_RELEASE(m_MeshTextureSRV[2]);

	SAFE_RELEASE(m_cbMaterial);
	SAFE_RELEASE(m_cbLight);
	SAFE_RELEASE(m_cbPerObject);
	SAFE_RELEASE(m_cbScene);
	SAFE_RELEASE(m_cbInstance);

	SAFE_RELEASE(m_BumpBaseTextureSRV);
	SAFE_RELEASE(m_BumpNormalTextureSRV);
	SAFE_RELEASE(m_BumpLightingIL);
	SAFE_RELEASE(m_LightingMeshIL);

	m_BumpCubeMesh.Release();

	m_StarMesh.Release();
	SAFE_RELEASE(m_ColorPrimitiveIL);

	m_Skybox.Release();

	RShaderManager::Instance().UnloadAllShaders();
	RResourceManager::Instance().UnloadAllMeshes();
}

bool FSGraphicsProjectApp::Initialize()
{
	// Initialize shaders
	RShaderManager::Instance().AddShader("Color", Color_PS, sizeof(Color_PS), Color_VS, sizeof(Color_VS));
	RShaderManager::Instance().AddShader("Lighting", Lighting_PS, sizeof(Lighting_PS), Lighting_VS, sizeof(Lighting_VS));
	RShaderManager::Instance().AddShader("Skybox", Skybox_PS, sizeof(Skybox_PS), Skybox_VS, sizeof(Skybox_VS));
	RShaderManager::Instance().AddShader("BumpLighting", BumpLighting_PS, sizeof(BumpLighting_PS), BumpLighting_VS, sizeof(BumpLighting_VS));
	RShaderManager::Instance().AddShader("InstancedLighting", InstancedLighting_PS, sizeof(InstancedLighting_PS), InstancedLighting_VS, sizeof(InstancedLighting_VS));
	RShaderManager::Instance().AddShader("Depth", Depth_PS, sizeof(Depth_PS), Depth_VS, sizeof(Depth_VS));
	RShaderManager::Instance().AddShader("InstancedDepth", InstancedDepth_PS, sizeof(InstancedDepth_PS), InstancedDepth_VS, sizeof(InstancedDepth_VS));

	m_ColorShader = RShaderManager::Instance().GetShaderResource("Color");
	m_LightingShader = RShaderManager::Instance().GetShaderResource("Lighting");
	m_BumpLightingShader = RShaderManager::Instance().GetShaderResource("BumpLighting");
	m_InstancedLightingShader = RShaderManager::Instance().GetShaderResource("InstancedLighting");
	m_DepthShader = RShaderManager::Instance().GetShaderResource("Depth");
	m_InstancedDepthShader = RShaderManager::Instance().GetShaderResource("InstancedDepth");

	// Create buffer for star mesh
	COLOR_VERTEX starVertex[12];

	for (int i = 0; i < 10; i++)
	{
		float r = (i % 2 == 0) ? 100.0f : 50.0f;
		starVertex[i] = { XMFLOAT4(sinf(DEG_TO_RAD(i * 36)) * r, cosf(DEG_TO_RAD(i * 36)) * r, 0.0f, 1.0f),
			XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) };
	}

	starVertex[10] = { XMFLOAT4(0.0f, 0.0f, -20.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) };
	starVertex[11] = { XMFLOAT4(0.0f, 0.0f, 20.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) };

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

	RRenderer.D3DDevice()->CreateInputLayout(colorVertDesc, 2, m_ColorShader->VS_Bytecode, m_ColorShader->VS_BytecodeSize, &m_ColorPrimitiveIL);

	// Create buffer for bump cube
	BUMP_MESH_VERTEX boxVertex[] = 
	{
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3( 1.0f,  1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3( 1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },

		{ XMFLOAT3( 1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3( 1.0f,  1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3( 1.0f,  1.0f,  1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3( 1.0f, -1.0f,  1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },

		{ XMFLOAT3( 1.0f, -1.0f,  1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3( 1.0f,  1.0f,  1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },

		{ XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, -1.0f) },

		{ XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3( 1.0f,  1.0f,  1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3( 1.0f,  1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },

		{ XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3( 1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3( 1.0f, -1.0f,  1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
	};

	for (UINT32 i = 0; i < sizeof(boxVertex) / sizeof(BUMP_MESH_VERTEX); i++)
	{
		XMVECTOR pos = XMLoadFloat3(&boxVertex[i].pos);
		XMStoreFloat3(&boxVertex[i].pos, pos * 100.0f);
	}

	UINT32 boxIndex[] =
	{
		0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7, 8, 9, 10, 8, 10, 11,
		12, 13, 14, 12, 14, 15, 16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23,
	};

	m_BumpCubeMesh.CreateVertexBuffer(boxVertex, sizeof(BUMP_MESH_VERTEX), 24);
	m_BumpCubeMesh.CreateIndexBuffer(boxIndex, sizeof(UINT32), 36);

	D3D11_INPUT_ELEMENT_DESC bumpVertDesc[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",	0, DXGI_FORMAT_R32G32B32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	RRenderer.D3DDevice()->CreateInputLayout(bumpVertDesc, 4, m_BumpLightingShader->VS_Bytecode, m_BumpLightingShader->VS_BytecodeSize, &m_BumpLightingIL);

	D3D11_BUFFER_DESC cbPerObjectDesc;
	ZeroMemory(&cbPerObjectDesc, sizeof(cbPerObjectDesc));
	cbPerObjectDesc.ByteWidth = sizeof(SHADER_OBJECT_BUFFER);
	cbPerObjectDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbPerObjectDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbPerObjectDesc.Usage = D3D11_USAGE_DYNAMIC;

	RRenderer.D3DDevice()->CreateBuffer(&cbPerObjectDesc, NULL, &m_cbPerObject);

	D3D11_BUFFER_DESC cbSceneDesc;
	ZeroMemory(&cbSceneDesc, sizeof(cbSceneDesc));
	cbSceneDesc.ByteWidth = sizeof(SHADER_SCENE_BUFFER);
	cbSceneDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbSceneDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbSceneDesc.Usage = D3D11_USAGE_DYNAMIC;

	RRenderer.D3DDevice()->CreateBuffer(&cbSceneDesc, NULL, &m_cbScene);

	D3D11_BUFFER_DESC cbLightDesc;
	ZeroMemory(&cbLightDesc, sizeof(cbLightDesc));
	cbLightDesc.ByteWidth = sizeof(SHADER_LIGHT_BUFFER);
	cbLightDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbLightDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbLightDesc.Usage = D3D11_USAGE_DYNAMIC;

	RRenderer.D3DDevice()->CreateBuffer(&cbLightDesc, NULL, &m_cbLight);

	D3D11_BUFFER_DESC cbMaterialDesc;
	ZeroMemory(&cbMaterialDesc, sizeof(cbMaterialDesc));
	cbMaterialDesc.ByteWidth = sizeof(SHADER_MATERIAL_BUFFER);
	cbMaterialDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbMaterialDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbMaterialDesc.Usage = D3D11_USAGE_DYNAMIC;

	RRenderer.D3DDevice()->CreateBuffer(&cbMaterialDesc, NULL, &m_cbMaterial);

	D3D11_BUFFER_DESC cbInstanceDesc;
	ZeroMemory(&cbInstanceDesc, sizeof(cbInstanceDesc));
	cbInstanceDesc.ByteWidth = sizeof(SHADER_INSTANCE_BUFFER);
	cbInstanceDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbInstanceDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbInstanceDesc.Usage = D3D11_USAGE_DYNAMIC;

	RRenderer.D3DDevice()->CreateBuffer(&cbInstanceDesc, NULL, &m_cbInstance);

	// Create input layout
	D3D11_INPUT_ELEMENT_DESC objVertDesc[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	RRenderer.D3DDevice()->CreateInputLayout(objVertDesc, 3, m_LightingShader->VS_Bytecode, m_LightingShader->VS_BytecodeSize, &m_LightingMeshIL);

	m_SceneMeshCity = RResourceManager::Instance().LoadFbxMesh("../Assets/city.fbx", m_LightingMeshIL);
	m_FbxMeshObj.SetMesh(m_SceneMeshCity);

	CreateDDSTextureFromFile(RRenderer.D3DDevice(), L"../Assets/cty1.dds", NULL, &m_MeshTextureSRV[0]);
	CreateDDSTextureFromFile(RRenderer.D3DDevice(), L"../Assets/ang1.dds", NULL, &m_MeshTextureSRV[1]);
	CreateDDSTextureFromFile(RRenderer.D3DDevice(), L"../Assets/cty2x.dds", NULL, &m_MeshTextureSRV[2]);
	CreateDDSTextureFromFile(RRenderer.D3DDevice(), L"../Assets/DiamondPlate.dds", NULL, &m_BumpBaseTextureSRV);
	CreateDDSTextureFromFile(RRenderer.D3DDevice(), L"../Assets/DiamondPlateNormal.dds", NULL, &m_BumpNormalTextureSRV);

	RMaterial meshMaterials[] =
	{
		{ m_LightingShader, 1, m_MeshTextureSRV[0] },
		{ m_LightingShader, 1, m_MeshTextureSRV[1] },
		{ m_LightingShader, 1, m_MeshTextureSRV[2] },
		{ m_LightingShader, 1, m_MeshTextureSRV[1] },
	};

	m_FbxMeshObj.SetMaterial(meshMaterials, 4);

	m_SceneMeshIsland = RResourceManager::Instance().LoadFbxMesh("../Assets/Island.fbx", m_LightingMeshIL);
	CreateDDSTextureFromFile(RRenderer.D3DDevice(), L"../Assets/TR_FloatingIsland02.dds", NULL, &m_IslandTextureSRV);
	m_IslandMeshObj.SetMesh(m_SceneMeshIsland);
	m_IslandMeshObj.SetPosition(XMFLOAT3(0.0f, 0.0f, 500.0f));

	RMaterial islandMaterials[] =
	{
		{ m_InstancedLightingShader, 1, m_IslandTextureSRV },
	};
	m_IslandMeshObj.SetMaterial(islandMaterials, 1);

	// Create texture sampler state
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	RRenderer.D3DDevice()->CreateSamplerState(&samplerDesc, &m_SamplerState);

	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	RRenderer.D3DDevice()->CreateSamplerState(&samplerDesc, &m_SamplerComparisonState);

	m_Skybox.CreateSkybox(L"../Assets/powderpeak.dds");

	XMStoreFloat4x4(&m_CameraMatrix, XMMatrixIdentity());
	m_CamPitch = 0.0f;
	m_CamYaw = PI;

	m_ShadowMap.Initialize(1024, 1024);

	return true;
}

void FSGraphicsProjectApp::UpdateScene(const RTimer& timer)
{
	if (RInput.GetBufferedKeyState(VK_RBUTTON) == BKS_Pressed)
	{
		RInput.HideCursor();
		RInput.LockCursor();
	}

	if (RInput.GetBufferedKeyState(VK_RBUTTON) == BKS_Released)
	{
		RInput.ShowCursor();
		RInput.UnlockCursor();
	}

	if (RInput.IsKeyDown(VK_RBUTTON))
	{
		int dx, dy;
		RInput.GetCursorRelPos(dx, dy);
		if (dx || dy)
		{
			m_CamYaw += (float)dx / 200.0f;
			m_CamPitch += (float)dy / 200.0f;
			m_CamPitch = max(-PI/2, min(PI/2, m_CamPitch));
		}
	}

	float camSpeed = 100.0f;
	if (RInput.IsKeyDown(VK_LSHIFT))
		camSpeed *= 10.0f;
	XMVECTOR moveVec = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	if (RInput.IsKeyDown('W'))
		moveVec += XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f) * timer.DeltaTime() * camSpeed;
	if (RInput.IsKeyDown('S'))
		moveVec -= XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f) * timer.DeltaTime() * camSpeed;
	if (RInput.IsKeyDown('A'))
		moveVec -= XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f) * timer.DeltaTime() * camSpeed;
	if (RInput.IsKeyDown('D'))
		moveVec += XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f) * timer.DeltaTime() * camSpeed;

	// Toggle lights
	if (RInput.GetBufferedKeyState('F') == BKS_Pressed)
		m_EnableLights[2] = !m_EnableLights[2];

	if (RInput.GetBufferedKeyState('1') == BKS_Pressed)
		m_EnableLights[0] = !m_EnableLights[0];

	if (RInput.GetBufferedKeyState('2') == BKS_Pressed)
		m_EnableLights[1] = !m_EnableLights[1];

	XMMATRIX cameraMatrix = XMLoadFloat4x4(&m_CameraMatrix);
	XMVECTOR camPos = cameraMatrix.r[3];
	cameraMatrix = XMMatrixRotationX(m_CamPitch) * XMMatrixRotationY(m_CamYaw);
	cameraMatrix.r[3] = camPos + XMVector4Transform(moveVec, cameraMatrix);
	XMStoreFloat4x4(&m_CameraMatrix, cameraMatrix);

	XMMATRIX viewMatrix = XMMatrixInverse(NULL, cameraMatrix);
	XMMATRIX projMatrix = XMMatrixPerspectiveFovLH(45.0f, RRenderer.AspectRatio(), 1.0f, 5000.0f);

	// Update scene constant buffer
	SHADER_SCENE_BUFFER cbScene;

	XMStoreFloat4x4(&cbScene.viewMatrix, viewMatrix);
	XMStoreFloat4x4(&cbScene.projMatrix, projMatrix);
	XMStoreFloat4x4(&cbScene.viewProjMatrix, viewMatrix * projMatrix);
	XMStoreFloat4(&cbScene.cameraPos, cameraMatrix.r[3]);

	float ct = timer.TotalTime() * 0.2f;
	XMVECTOR sunVec = XMVector3Normalize(XMVectorSet(sinf(ct) * 0.5f, 0.25f, cosf(ct) * 0.5f, 1.0f));
	XMMATRIX shadowViewMatrix = XMMatrixLookAtLH(sunVec * 2000.0f, XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f));

	m_ShadowMap.SetViewMatrix(shadowViewMatrix);
	m_ShadowMap.SetOrthogonalProjection(4000.0f, 4000.0f, 0.1f, 4000.0f);

	XMMATRIX shadowTransform(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX shadowViewProjMatrix = m_ShadowMap.GetViewMatrix() * m_ShadowMap.GetProjectionMatrix();
	XMStoreFloat4x4(&cbScene.shadowViewProjMatrix, shadowViewProjMatrix);
	shadowViewProjMatrix *= shadowTransform;
	XMStoreFloat4x4(&cbScene.shadowViewProjBiasedMatrix, shadowViewProjMatrix);

	D3D11_MAPPED_SUBRESOURCE subres;
	RRenderer.D3DImmediateContext()->Map(m_cbScene, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
	memcpy(subres.pData, &cbScene, sizeof(SHADER_SCENE_BUFFER));
	RRenderer.D3DImmediateContext()->Unmap(m_cbScene, 0);

	// Update light constant buffer
	SHADER_LIGHT_BUFFER cbLight;
	ZeroMemory(&cbLight, sizeof(cbLight));

	// Setup ambient color
	XMStoreFloat4(&cbLight.HighHemisphereAmbientColor, XMVectorSet(0.3f, 0.3f, 0.5f, 0.5f));
	XMStoreFloat4(&cbLight.LowHemisphereAmbientColor, XMVectorSet(0.2f, 0.2f, 0.2f, 0.5f));

	if (m_EnableLights[0])
	{
		XMVECTOR dirLightVec = XMVector3Normalize(XMVectorSet(0.25f, 1.0f, 0.5f, 1.0f));

		cbLight.DirectionalLightCount = 1;
		XMStoreFloat4(&cbLight.DirectionalLight[0].Color, XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f));
		XMStoreFloat4(&cbLight.DirectionalLight[0].Direction, sunVec);
	}

	if (m_EnableLights[1])
	{
		cbLight.PointLightCount = 1;
		XMVECTOR pointLightPosAndRadius = XMVectorSet(sinf(timer.TotalTime()) * 400.0f, 50.0f, cosf(timer.TotalTime()) * 400.0f, 1000.0f);
		XMStoreFloat4(&cbLight.PointLight[0].PosAndRadius, pointLightPosAndRadius);
		XMStoreFloat4(&cbLight.PointLight[0].Color, XMVectorSet(1.0f, 0.75f, 0.75f, 5.0f));
	}

	if (m_EnableLights[2])
	{
		cbLight.SpotlightCount = 1;
		XMVECTOR spotlightPos = XMVectorSet(cameraMatrix.r[3].m128_f32[0], cameraMatrix.r[3].m128_f32[1], cameraMatrix.r[3].m128_f32[2], 0.97f);
		XMVECTOR spotlightCone = XMVectorSet(cameraMatrix.r[2].m128_f32[0], cameraMatrix.r[2].m128_f32[1], cameraMatrix.r[2].m128_f32[2], 0.9f);
		XMStoreFloat4(&cbLight.Spotlight[0].PosAndInnerConeRatio, spotlightPos);
		XMStoreFloat4(&cbLight.Spotlight[0].ConeDirAndOuterConeRatio, spotlightCone);
		XMStoreFloat4(&cbLight.Spotlight[0].Color, XMVectorSet(0.85f, 0.85f, 1.0f, 1.0f));
	}

	XMStoreFloat4(&cbLight.CameraPos, cameraMatrix.r[3]);

	RRenderer.D3DImmediateContext()->Map(m_cbLight, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
	memcpy(subres.pData, &cbLight, sizeof(SHADER_LIGHT_BUFFER));
	RRenderer.D3DImmediateContext()->Unmap(m_cbLight, 0);

	// Update material buffer
	SHADER_MATERIAL_BUFFER cbMaterial;
	ZeroMemory(&cbMaterial, sizeof(cbMaterial));

	XMStoreFloat4(&cbMaterial.SpecularColorAndPower, XMVectorSet(1.0f, 1.0f, 1.0f, 512.0f));

	RRenderer.D3DImmediateContext()->Map(m_cbMaterial, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
	memcpy(subres.pData, &cbMaterial, sizeof(SHADER_MATERIAL_BUFFER));
	RRenderer.D3DImmediateContext()->Unmap(m_cbMaterial, 0);

	// Update instance buffer
	SHADER_INSTANCE_BUFFER cbInstance;
	ZeroMemory(&cbInstance, sizeof(cbInstance));

	for (int i = 0; i < MAX_INSTANCE_COUNT; i++)
	{
		float d = sinf((float)i / MAX_INSTANCE_COUNT * 2 * PI) + 2.0f;
		float x = sinf((float)i / MAX_INSTANCE_COUNT * 2 * PI) * 1000.0f * d;
		float y = sinf((float)i / MAX_INSTANCE_COUNT * 2 * PI * 8) * 1000.0f * d;
		float z = cosf((float)i / MAX_INSTANCE_COUNT * 2 * PI) * 1000.0f * d;
		XMMATRIX instanceMatrix = XMMatrixTranslation(x, y, z) * XMMatrixRotationY(x + timer.TotalTime() * 0.1f * sinf(d));

		XMStoreFloat4x4(&cbInstance.instancedWorldMatrix[i], instanceMatrix);
	}

	RRenderer.D3DImmediateContext()->Map(m_cbInstance, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
	memcpy(subres.pData, &cbInstance, sizeof(SHADER_INSTANCE_BUFFER));
	RRenderer.D3DImmediateContext()->Unmap(m_cbInstance, 0);
}

void FSGraphicsProjectApp::RenderScene()
{
	//=========================== Shadow Pass ===========================
	m_ShadowMap.SetupRenderTarget();

	RRenderer.Clear();

	RRenderer.D3DImmediateContext()->VSSetConstantBuffers(0, 1, &m_cbPerObject);
	RRenderer.D3DImmediateContext()->VSSetConstantBuffers(1, 1, &m_cbScene);
	RRenderer.D3DImmediateContext()->VSSetConstantBuffers(2, 1, &m_cbInstance);
	RRenderer.D3DImmediateContext()->PSSetConstantBuffers(0, 1, &m_cbLight);
	RRenderer.D3DImmediateContext()->PSSetConstantBuffers(1, 1, &m_cbMaterial);
	RRenderer.D3DImmediateContext()->PSSetSamplers(0, 1, &m_SamplerState);
	RRenderer.D3DImmediateContext()->PSSetSamplers(2, 1, &m_SamplerComparisonState);

	// Draw star
	SetPerObjectConstBuffuer(XMMatrixTranslation(0.0f, 500.0f, 0.0f));

	m_DepthShader->Bind();
	RRenderer.D3DImmediateContext()->IASetInputLayout(m_ColorPrimitiveIL);

	m_StarMesh.Draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Draw meshes

	// Draw city
	SetPerObjectConstBuffuer(m_FbxMeshObj.GetNodeTransform());
	m_FbxMeshObj.DrawWithShader(m_DepthShader);

	// Draw island
	SetPerObjectConstBuffuer(m_IslandMeshObj.GetNodeTransform());
	m_IslandMeshObj.DrawWithShader(m_InstancedDepthShader, true, MAX_INSTANCE_COUNT);

	// Draw bumped cube
	SetPerObjectConstBuffuer(XMMatrixTranslation(0.0f, 150.0f, 0.0f));

	m_DepthShader->Bind();
	RRenderer.D3DImmediateContext()->IASetInputLayout(m_BumpLightingIL);
	m_BumpCubeMesh.Draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	//=========================== Normal Pass ===========================
	RRenderer.SetRenderTarget();

	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = static_cast<float>(RRenderer.GetClientWidth());
	vp.Height = static_cast<float>(RRenderer.GetClientHeight());
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	RRenderer.D3DImmediateContext()->RSSetViewports(1, &vp);

	// Set shadow map to pixel shader
	ID3D11ShaderResourceView* shadowMapSRV[] = { m_ShadowMap.GetRenderTargetSRV() };
	RRenderer.D3DImmediateContext()->PSSetShaderResources(2, 1, shadowMapSRV);

	RRenderer.Clear();

	// Set up object world matrix
	SetPerObjectConstBuffuer(XMMatrixTranslation(0.0f, 0.0f, 0.0f));

	// Draw skybox
	m_Skybox.Draw();

	// Clear depth buffer for skybox
	RRenderer.Clear(false, Colors::Black);

	// Draw star
	SetPerObjectConstBuffuer(XMMatrixTranslation(0.0f, 500.0f, 0.0f));

	m_ColorShader->Bind();
	RRenderer.D3DImmediateContext()->IASetInputLayout(m_ColorPrimitiveIL);

	m_StarMesh.Draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Draw meshes

	// Draw city
	SetPerObjectConstBuffuer(m_FbxMeshObj.GetNodeTransform());
	m_FbxMeshObj.Draw();

	// Draw island
	SetPerObjectConstBuffuer(m_IslandMeshObj.GetNodeTransform());
	m_IslandMeshObj.Draw(true, MAX_INSTANCE_COUNT);

	// Draw bumped cube
	SetPerObjectConstBuffuer(XMMatrixTranslation(0.0f, 150.0f, 0.0f));

	m_BumpLightingShader->Bind();
	RRenderer.D3DImmediateContext()->PSSetShaderResources(0, 1, &m_BumpBaseTextureSRV);
	RRenderer.D3DImmediateContext()->PSSetShaderResources(1, 1, &m_BumpNormalTextureSRV);
	RRenderer.D3DImmediateContext()->IASetInputLayout(m_BumpLightingIL);
	m_BumpCubeMesh.Draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D11ShaderResourceView* nullSRV[] = { nullptr };
	RRenderer.D3DImmediateContext()->PSSetShaderResources(0, 1, nullSRV);
	RRenderer.D3DImmediateContext()->PSSetShaderResources(2, 1, nullSRV);

	RRenderer.Present();
}

void FSGraphicsProjectApp::SetPerObjectConstBuffuer(const XMMATRIX& world)
{
	SHADER_OBJECT_BUFFER cbObject;
	XMStoreFloat4x4(&cbObject.worldMatrix, world);

	D3D11_MAPPED_SUBRESOURCE subres;
	RRenderer.D3DImmediateContext()->Map(m_cbPerObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
	memcpy(subres.pData, &cbObject, sizeof(cbObject));
	RRenderer.D3DImmediateContext()->Unmap(m_cbPerObject, 0);
}
