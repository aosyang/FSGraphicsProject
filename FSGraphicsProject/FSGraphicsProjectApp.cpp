//=============================================================================
// FSGraphicsProjectApp.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "FSGraphicsProjectApp.h"

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
#include "Particle_PS.csh"
#include "Particle_VS.csh"
#include "Particle_GS.csh"
#include "Refraction_PS.csh"
#include "Refraction_VS.csh"
#include "AmbientOcclusion_PS.csh"
#include "AmbientOcclusion_VS.csh"

struct COLOR_VERTEX
{
	RVec4 pos;
	RVec4 color;
};

struct BUMP_MESH_VERTEX
{
	RVec3 pos;
	RVec2 uv;
	RVec3 normal;
	RVec3 tangent;
};

struct ParticleDepthComparer
{
	RVec4 CamPos, CamDir;

	ParticleDepthComparer(const RVec4& camPos, const RVec4& camDir)
	{
		CamPos = camPos;
		CamDir = camDir;
	}

	bool operator()(const PARTICLE_VERTEX &a, const PARTICLE_VERTEX &b)
	{
		//return dot(a.pos - CamPos, CamDir) > dot(b.pos - CamPos, CamDir);
		return sqrDist(a.pos, CamPos) > sqrDist(b.pos, CamPos);
	}

	// Helper functions
	static float dot(const RVec4& a, const RVec4& b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	static float sqrDist(const RVec4& a, const RVec4 b)
	{
		float dx = b.x - a.x,
			  dy = b.y - a.y,
			  dz = b.z - a.z;
		return dx * dx + dy * dy + dz * dz;
	}
};

FSGraphicsProjectApp::FSGraphicsProjectApp()
	: m_ColorPrimitiveIL(nullptr),
	  m_ColorShader(nullptr),
	  m_LightingMeshIL(nullptr),
	  m_LightingShader(nullptr),
	  m_SamplerState(nullptr)
{
	m_MeshTexture[0] = nullptr;
	m_MeshTexture[1] = nullptr;
	m_MeshTexture[2] = nullptr;

	m_EnableLights[0] = true;
	m_EnableLights[1] = true;
	m_EnableLights[2] = true;
}


FSGraphicsProjectApp::~FSGraphicsProjectApp()
{
	SAFE_RELEASE(m_RenderTargetDepthBuffer);
	SAFE_RELEASE(m_RenderTargetDepthView);
	SAFE_RELEASE(m_RenderTargetBuffer);
	SAFE_RELEASE(m_RenderTargetView);
	SAFE_RELEASE(m_RenderTargetSRV);

	SAFE_RELEASE(m_DepthState[0]);
	SAFE_RELEASE(m_DepthState[1]);

	SAFE_RELEASE(m_BlendState[0]);
	SAFE_RELEASE(m_BlendState[1]);
	SAFE_RELEASE(m_BlendState[2]);

	SAFE_RELEASE(m_ParticleIL);
	m_ParticleBuffer.Release();

	SAFE_RELEASE(m_SamplerComparisonState);
	SAFE_RELEASE(m_SamplerState);

	SAFE_RELEASE(m_cbScreen);
	SAFE_RELEASE(m_cbMaterial);
	SAFE_RELEASE(m_cbLight);
	SAFE_RELEASE(m_cbPerObject);
	SAFE_RELEASE(m_cbScene);
	SAFE_RELEASE(m_cbInstance);

	SAFE_RELEASE(m_BumpLightingIL);
	SAFE_RELEASE(m_LightingMeshIL);

	m_BumpCubeMesh.Release();

	m_StarMesh.Release();
	SAFE_RELEASE(m_ColorPrimitiveIL);

	m_Skybox.Release();

	RShaderManager::Instance().UnloadAllShaders();
	RResourceManager::Instance().Destroy();
}

bool FSGraphicsProjectApp::Initialize()
{
	RResourceManager::Instance().Initialize();
	CreateSceneRenderTargetView();

	// Initialize shaders
	RShaderManager::Instance().AddShader("Color", Color_PS, sizeof(Color_PS), Color_VS, sizeof(Color_VS));
	RShaderManager::Instance().AddShader("Lighting", Lighting_PS, sizeof(Lighting_PS), Lighting_VS, sizeof(Lighting_VS));
	RShaderManager::Instance().AddShader("Skybox", Skybox_PS, sizeof(Skybox_PS), Skybox_VS, sizeof(Skybox_VS));
	RShaderManager::Instance().AddShader("BumpLighting", BumpLighting_PS, sizeof(BumpLighting_PS), BumpLighting_VS, sizeof(BumpLighting_VS));
	RShaderManager::Instance().AddShader("InstancedLighting", InstancedLighting_PS, sizeof(InstancedLighting_PS), InstancedLighting_VS, sizeof(InstancedLighting_VS));
	RShaderManager::Instance().AddShader("Depth", Depth_PS, sizeof(Depth_PS), Depth_VS, sizeof(Depth_VS));
	RShaderManager::Instance().AddShader("InstancedDepth", InstancedDepth_PS, sizeof(InstancedDepth_PS), InstancedDepth_VS, sizeof(InstancedDepth_VS));
	RShaderManager::Instance().AddShader("Particle", Particle_PS, sizeof(Particle_PS), Particle_VS, sizeof(Particle_VS), Particle_GS, sizeof(Particle_GS));
	RShaderManager::Instance().AddShader("Refraction", Refraction_PS, sizeof(Refraction_PS), Refraction_VS, sizeof(Refraction_VS));
	RShaderManager::Instance().AddShader("AmbientOcclusion", AmbientOcclusion_PS, sizeof(AmbientOcclusion_PS), AmbientOcclusion_VS, sizeof(AmbientOcclusion_VS));
	
	m_ColorShader = RShaderManager::Instance().GetShaderResource("Color");
	m_LightingShader = RShaderManager::Instance().GetShaderResource("Lighting");
	m_BumpLightingShader = RShaderManager::Instance().GetShaderResource("BumpLighting");
	m_InstancedLightingShader = RShaderManager::Instance().GetShaderResource("InstancedLighting");
	m_DepthShader = RShaderManager::Instance().GetShaderResource("Depth");
	m_InstancedDepthShader = RShaderManager::Instance().GetShaderResource("InstancedDepth");
	m_ParticleShader = RShaderManager::Instance().GetShaderResource("Particle");
	m_RefractionShader = RShaderManager::Instance().GetShaderResource("Refraction");
	m_AOShader = RShaderManager::Instance().GetShaderResource("AmbientOcclusion");

	// Create buffer for star mesh
	COLOR_VERTEX starVertex[12];

	for (int i = 0; i < 10; i++)
	{
		float r = (i % 2 == 0) ? 100.0f : 50.0f;
		starVertex[i] = { RVec4(sinf(DEG_TO_RAD(i * 36)) * r, cosf(DEG_TO_RAD(i * 36)) * r, 0.0f, 1.0f),
			RVec4(1.0f, 0.0f, 0.0f, 1.0f) };
	}

	starVertex[10] = { RVec4(0.0f, 0.0f, -20.0f, 1.0f), RVec4(1.0f, 1.0f, 0.0f, 1.0f) };
	starVertex[11] = { RVec4(0.0f, 0.0f, 20.0f, 1.0f), RVec4(1.0f, 1.0f, 0.0f, 1.0f) };

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
		{ RVec3(-1.0f, -1.0f, -1.0f), RVec2(0.0f, 1.0f), RVec3(0.0f, 0.0f, -1.0f), RVec3(1.0f, 0.0f, 0.0f) },
		{ RVec3(-1.0f,  1.0f, -1.0f), RVec2(0.0f, 0.0f), RVec3(0.0f, 0.0f, -1.0f), RVec3(1.0f, 0.0f, 0.0f) },
		{ RVec3( 1.0f,  1.0f, -1.0f), RVec2(1.0f, 0.0f), RVec3(0.0f, 0.0f, -1.0f), RVec3(1.0f, 0.0f, 0.0f) },
		{ RVec3( 1.0f, -1.0f, -1.0f), RVec2(1.0f, 1.0f), RVec3(0.0f, 0.0f, -1.0f), RVec3(1.0f, 0.0f, 0.0f) },

		{ RVec3( 1.0f, -1.0f, -1.0f), RVec2(0.0f, 1.0f), RVec3(1.0f, 0.0f, 0.0f), RVec3(0.0f, 0.0f, 1.0f) },
		{ RVec3( 1.0f,  1.0f, -1.0f), RVec2(0.0f, 0.0f), RVec3(1.0f, 0.0f, 0.0f), RVec3(0.0f, 0.0f, 1.0f) },
		{ RVec3( 1.0f,  1.0f,  1.0f), RVec2(1.0f, 0.0f), RVec3(1.0f, 0.0f, 0.0f), RVec3(0.0f, 0.0f, 1.0f) },
		{ RVec3( 1.0f, -1.0f,  1.0f), RVec2(1.0f, 1.0f), RVec3(1.0f, 0.0f, 0.0f), RVec3(0.0f, 0.0f, 1.0f) },

		{ RVec3( 1.0f, -1.0f,  1.0f), RVec2(0.0f, 1.0f), RVec3(0.0f, 0.0f, 1.0f), RVec3(-1.0f, 0.0f, 0.0f) },
		{ RVec3( 1.0f,  1.0f,  1.0f), RVec2(0.0f, 0.0f), RVec3(0.0f, 0.0f, 1.0f), RVec3(-1.0f, 0.0f, 0.0f) },
		{ RVec3(-1.0f,  1.0f,  1.0f), RVec2(1.0f, 0.0f), RVec3(0.0f, 0.0f, 1.0f), RVec3(-1.0f, 0.0f, 0.0f) },
		{ RVec3(-1.0f, -1.0f,  1.0f), RVec2(1.0f, 1.0f), RVec3(0.0f, 0.0f, 1.0f), RVec3(-1.0f, 0.0f, 0.0f) },

		{ RVec3(-1.0f, -1.0f,  1.0f), RVec2(0.0f, 1.0f), RVec3(-1.0f, 0.0f, 0.0f), RVec3(1.0f, 0.0f, -1.0f) },
		{ RVec3(-1.0f,  1.0f,  1.0f), RVec2(0.0f, 0.0f), RVec3(-1.0f, 0.0f, 0.0f), RVec3(1.0f, 0.0f, -1.0f) },
		{ RVec3(-1.0f,  1.0f, -1.0f), RVec2(1.0f, 0.0f), RVec3(-1.0f, 0.0f, 0.0f), RVec3(1.0f, 0.0f, -1.0f) },
		{ RVec3(-1.0f, -1.0f, -1.0f), RVec2(1.0f, 1.0f), RVec3(-1.0f, 0.0f, 0.0f), RVec3(1.0f, 0.0f, -1.0f) },

		{ RVec3(-1.0f,  1.0f, -1.0f), RVec2(0.0f, 1.0f), RVec3(0.0f, 1.0f, 0.0f), RVec3(1.0f, 0.0f, 0.0f) },
		{ RVec3(-1.0f,  1.0f,  1.0f), RVec2(0.0f, 0.0f), RVec3(0.0f, 1.0f, 0.0f), RVec3(1.0f, 0.0f, 0.0f) },
		{ RVec3( 1.0f,  1.0f,  1.0f), RVec2(1.0f, 0.0f), RVec3(0.0f, 1.0f, 0.0f), RVec3(1.0f, 0.0f, 0.0f) },
		{ RVec3( 1.0f,  1.0f, -1.0f), RVec2(1.0f, 1.0f), RVec3(0.0f, 1.0f, 0.0f), RVec3(1.0f, 0.0f, 0.0f) },

		{ RVec3(-1.0f, -1.0f,  1.0f), RVec2(0.0f, 1.0f), RVec3(0.0f, -1.0f, 0.0f), RVec3(1.0f, 0.0f, 0.0f) },
		{ RVec3(-1.0f, -1.0f, -1.0f), RVec2(0.0f, 0.0f), RVec3(0.0f, -1.0f, 0.0f), RVec3(1.0f, 0.0f, 0.0f) },
		{ RVec3( 1.0f, -1.0f, -1.0f), RVec2(1.0f, 0.0f), RVec3(0.0f, -1.0f, 0.0f), RVec3(1.0f, 0.0f, 0.0f) },
		{ RVec3( 1.0f, -1.0f,  1.0f), RVec2(1.0f, 1.0f), RVec3(0.0f, -1.0f, 0.0f), RVec3(1.0f, 0.0f, 0.0f) },
	};

	for (UINT32 i = 0; i < sizeof(boxVertex) / sizeof(BUMP_MESH_VERTEX); i++)
	{
		boxVertex[i].pos *= 100.0f;
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

	D3D11_BUFFER_DESC cbScreenDesc;
	ZeroMemory(&cbScreenDesc, sizeof(cbScreenDesc));
	cbScreenDesc.ByteWidth = sizeof(SHADER_SCREEN_BUFFER);
	cbScreenDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbScreenDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbScreenDesc.Usage = D3D11_USAGE_DYNAMIC;

	RRenderer.D3DDevice()->CreateBuffer(&cbScreenDesc, NULL, &m_cbScreen);


	// Create input layout
	D3D11_INPUT_ELEMENT_DESC objVertDesc[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",	0, DXGI_FORMAT_R32G32B32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	1, DXGI_FORMAT_R32G32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	RRenderer.D3DDevice()->CreateInputLayout(objVertDesc, 5, m_AOShader->VS_Bytecode, m_AOShader->VS_BytecodeSize, &m_LightingMeshIL);

	m_MeshTexture[0] = RResourceManager::Instance().LoadDDSTexture("../Assets/cty1.dds");
	m_MeshTexture[1] = RResourceManager::Instance().LoadDDSTexture("../Assets/ang1.dds");
	m_MeshTexture[2] = RResourceManager::Instance().LoadDDSTexture("../Assets/cty2x.dds");
	m_BumpBaseTexture = RResourceManager::Instance().LoadDDSTexture("../Assets/DiamondPlate.dds");
	m_BumpNormalTexture = RResourceManager::Instance().LoadDDSTexture("../Assets/DiamondPlateNormal.dds");

	m_SceneMeshCity = RResourceManager::Instance().LoadFbxMesh("../Assets/city.fbx", m_LightingMeshIL);
	m_FbxMeshObj.SetMesh(m_SceneMeshCity);

	m_MeshTachikoma = RResourceManager::Instance().LoadFbxMesh("../Assets/tachikoma.fbx", m_LightingMeshIL);
	m_TachikomaObj.SetMesh(m_MeshTachikoma);
	m_TachikomaObj.SetPosition(RVec3(0.0f, 40.0f, 0.0f));

	RTexture* meshAOTextureSRV[] =
	{
		RResourceManager::Instance().LoadDDSTexture("../Assets/lowBuildingsAO.dds"),
		RResourceManager::Instance().LoadDDSTexture("../Assets/groundAO.dds"),
		RResourceManager::Instance().LoadDDSTexture("../Assets/tallBuildingsAO.dds"),
		RResourceManager::Instance().LoadDDSTexture("../Assets/StreetAO.dds"),
	};

	RMaterial cityMaterials[] =
	{
		{ m_AOShader, 2, m_MeshTexture[0], meshAOTextureSRV[0] },
		{ m_AOShader, 2, m_MeshTexture[1], meshAOTextureSRV[1] },
		{ m_AOShader, 2, m_MeshTexture[2], meshAOTextureSRV[2] },
		{ m_AOShader, 2, m_MeshTexture[1], meshAOTextureSRV[3] },
	};

	m_FbxMeshObj.SetMaterial(cityMaterials, 4);

	RMaterial tachikomaMaterials[] =
	{
		{ m_RefractionShader, 1, RResourceManager::Instance().WrapSRV(m_RenderTargetSRV) },
	};

	m_TachikomaObj.SetMaterial(tachikomaMaterials, 1);


	m_AOSceneMesh = RResourceManager::Instance().LoadFbxMesh("../Assets/AO_Scene.fbx", m_LightingMeshIL);
	m_AOSceneObj.SetMesh(m_AOSceneMesh);
	m_AOTexture = RResourceManager::Instance().LoadDDSTexture("../Assets/AO_Scene.dds");
	RTexture* greyTexture = RResourceManager::Instance().LoadDDSTexture("../Assets/Grey.dds");

	RMaterial aoMat[] =
	{
		{ m_AOShader, 2, greyTexture, m_AOTexture },
	};

	m_AOSceneObj.SetMaterial(aoMat, 1);
	m_AOSceneObj.SetPosition(RVec3(-500.0f, 0.0f, 500.0f));

	m_CharacterObj.SetMesh(RResourceManager::Instance().LoadFbxMesh("../Assets/SpeedballPlayer.fbx", m_LightingMeshIL));
	m_CharacterObj.SetOverridingShader(m_BumpLightingShader);
	m_CharacterObj.SetTransform(RMatrix4::CreateXAxisRotation(-90.0f) * RMatrix4::CreateTranslation(-1100.0f, 40.0f, 0.0f));

	m_SceneMeshIsland = RResourceManager::Instance().LoadFbxMesh("../Assets/Island.fbx", m_LightingMeshIL);
	m_IslandTexture = RResourceManager::Instance().LoadDDSTexture("../Assets/TR_FloatingIsland02.dds");
	m_IslandMeshObj.SetMesh(m_SceneMeshIsland);
	m_IslandMeshObj.SetPosition(RVec3(0.0f, 0.0f, 500.0f));

	RMaterial islandMaterials[] =
	{
		{ m_InstancedLightingShader, 1, m_IslandTexture },
	};
	m_IslandMeshObj.SetMaterial(islandMaterials, 1);

	// Create texture sampler state
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
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

	m_CameraMatrix = RMatrix4::CreateTranslation(407.023712f, 339.007507f, 876.396484f);
	m_CamPitch = 0.0900001600f;
	m_CamYaw = 3.88659930f;

	m_ShadowMap.Initialize(1024, 1024);

	m_ParticleBuffer.CreateVertexBuffer(nullptr, sizeof(PARTICLE_VERTEX), PARTICLE_COUNT, true);

	D3D11_INPUT_ELEMENT_DESC particleVertDesc[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32_FLOAT,			0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	RRenderer.D3DDevice()->CreateInputLayout(particleVertDesc, 3, m_ParticleShader->VS_Bytecode, m_ParticleShader->VS_BytecodeSize, &m_ParticleIL);

	for (int i = 0; i < PARTICLE_COUNT; i++)
	{
		float x = MathHelper::RandF(-2000.0f, 1000.0f),
			  y = MathHelper::RandF(1000.0f, 1200.0f),
			  z = MathHelper::RandF(-2000.0f, 1000.0f),
			  w = MathHelper::RandF(500.0f, 750.0f);
		float ic = MathHelper::RandF(0.5f, 1.0f);
		m_ParticleVert[i] = { RVec4(x, y, z, w), RVec4(ic, ic, ic, 1.0f), MathHelper::RandF(0.0f, PI * 2) };
	}

	m_ParticleDiffuseTexture = RResourceManager::Instance().LoadDDSTexture("../Assets/smoke_diffuse.dds");
	m_ParticleNormalTexture = RResourceManager::Instance().LoadDDSTexture("../Assets/smoke_normal.dds");

	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	RRenderer.D3DDevice()->CreateBlendState(&blendDesc, &m_BlendState[0]);

	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	RRenderer.D3DDevice()->CreateBlendState(&blendDesc, &m_BlendState[1]);

	blendDesc.AlphaToCoverageEnable = true;
	RRenderer.D3DDevice()->CreateBlendState(&blendDesc, &m_BlendState[2]);

	D3D11_DEPTH_STENCIL_DESC depthDesc;

	// Depth test parameters
	depthDesc.DepthEnable = true;
	depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthDesc.DepthFunc = D3D11_COMPARISON_LESS;

	// Stencil test parameters
	depthDesc.StencilEnable = false;
	depthDesc.StencilReadMask = 0xFF;
	depthDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing
	depthDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	depthDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	RRenderer.D3DDevice()->CreateDepthStencilState(&depthDesc, &m_DepthState[0]);

	depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	RRenderer.D3DDevice()->CreateDepthStencilState(&depthDesc, &m_DepthState[1]);

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
	RVec3 moveVec(0.0f, 0.0f, 0.0f);
	if (RInput.IsKeyDown('W'))
		moveVec += RVec3(0.0f, 0.0f, 1.0f) * timer.DeltaTime() * camSpeed;
	if (RInput.IsKeyDown('S'))
		moveVec -= RVec3(0.0f, 0.0f, 1.0f) * timer.DeltaTime() * camSpeed;
	if (RInput.IsKeyDown('A'))
		moveVec -= RVec3(1.0f, 0.0f, 0.0f) * timer.DeltaTime() * camSpeed;
	if (RInput.IsKeyDown('D'))
		moveVec += RVec3(1.0f, 0.0f, 0.0f) * timer.DeltaTime() * camSpeed;

	// Toggle lights
	if (RInput.GetBufferedKeyState('F') == BKS_Pressed)
		m_EnableLights[2] = !m_EnableLights[2];

	if (RInput.GetBufferedKeyState('1') == BKS_Pressed)
		m_EnableLights[0] = !m_EnableLights[0];

	if (RInput.GetBufferedKeyState('2') == BKS_Pressed)
		m_EnableLights[1] = !m_EnableLights[1];

	RVec3 camPos = m_CameraMatrix.GetTranslation();
	m_CameraMatrix = RMatrix4::CreateXAxisRotation(m_CamPitch * 180 / PI) * RMatrix4::CreateYAxisRotation(m_CamYaw * 180 / PI);
	m_CameraMatrix.SetTranslation(camPos + (RVec4(moveVec, 1.0f) * m_CameraMatrix).ToVec3());

	RMatrix4 viewMatrix = m_CameraMatrix.GetViewMatrix();
	RMatrix4 projMatrix = RMatrix4::CreatePerspectiveProjectionLH(65.0f, RRenderer.AspectRatio(), 1.0f, 10000.0f);

	// Update scene constant buffer
	SHADER_SCENE_BUFFER cbScene;

	cbScene.viewMatrix = viewMatrix;
	cbScene.projMatrix = projMatrix;
	cbScene.viewProjMatrix = viewMatrix * projMatrix;
	cbScene.cameraPos = m_CameraMatrix.GetRow(3);

	float ct = timer.TotalTime() * 0.2f;
	RVec3 sunVec = RVec3(sinf(ct) * 0.5f, 0.25f, cosf(ct) * 0.5f).GetNormalizedVec3();
	RMatrix4 shadowViewMatrix = RMatrix4::CreateLookAtViewLH(sunVec * 2000.0f, RVec3(0.0f, 0.0f, 0.0f), RVec3(0.0f, 1.0f, 0.0f));

	m_ShadowMap.SetViewMatrix(shadowViewMatrix);
	m_ShadowMap.SetOrthogonalProjection(5000.0f, 5000.0f, 0.1f, 5000.0f);

	RMatrix4 shadowTransform(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	RMatrix4 shadowViewProjMatrix = m_ShadowMap.GetViewMatrix() * m_ShadowMap.GetProjectionMatrix();
	cbScene.shadowViewProjMatrix = shadowViewProjMatrix;
	shadowViewProjMatrix *= shadowTransform;
	cbScene.shadowViewProjBiasedMatrix = shadowViewProjMatrix;

	D3D11_MAPPED_SUBRESOURCE subres;
	RRenderer.D3DImmediateContext()->Map(m_cbScene, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
	memcpy(subres.pData, &cbScene, sizeof(SHADER_SCENE_BUFFER));
	RRenderer.D3DImmediateContext()->Unmap(m_cbScene, 0);

	// Update light constant buffer
	SHADER_LIGHT_BUFFER cbLight;
	ZeroMemory(&cbLight, sizeof(cbLight));

	// Setup ambient color
	cbLight.HighHemisphereAmbientColor = RVec4(0.75f, 0.75f, 0.75f, 1.0f);
	cbLight.LowHemisphereAmbientColor = RVec4(0.2f, 0.2f, 0.2f, 0.5f);

	if (m_EnableLights[0])
	{
		RVec4 dirLightVec = RVec4(RVec3(0.25f, 1.0f, 0.5f).GetNormalizedVec3(), 1.0f);

		cbLight.DirectionalLightCount = 1;
		cbLight.DirectionalLight[0].Color = RVec4(1.0f, 1.0f, 1.0f, 1.0f);
		cbLight.DirectionalLight[0].Direction = sunVec;
	}

	if (m_EnableLights[1])
	{
		cbLight.PointLightCount = 1;
		RVec4 pointLightPosAndRadius = RVec4(sinf(timer.TotalTime()) * 400.0f, 700.0f, cosf(timer.TotalTime()) * 400.0f, 1000.0f);
		cbLight.PointLight[0].PosAndRadius = pointLightPosAndRadius;
		cbLight.PointLight[0].Color = RVec4(1.0f, 0.75f, 0.75f, 5.0f);
	}

	if (m_EnableLights[2])
	{
		cbLight.SpotlightCount = 1;
		RVec4 spotlightPos = RVec4(m_CameraMatrix.GetTranslation(), 2000.0f);
		RVec4 spotlightDir = RVec4(m_CameraMatrix.GetRow(2).ToVec3(), 1.0f);
		RVec4 spotlightRatio = RVec4(0.97f, 0.9f, 0.0f, 0.0f);
		cbLight.Spotlight[0].PosAndRadius = spotlightPos;
		cbLight.Spotlight[0].Direction = spotlightDir;
		cbLight.Spotlight[0].ConeRatio = spotlightRatio;
		cbLight.Spotlight[0].Color = RVec4(0.85f, 0.85f, 1.0f, 1.0f);
	}

	cbLight.CameraPos = m_CameraMatrix.GetRow(3);

	RRenderer.D3DImmediateContext()->Map(m_cbLight, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
	memcpy(subres.pData, &cbLight, sizeof(SHADER_LIGHT_BUFFER));
	RRenderer.D3DImmediateContext()->Unmap(m_cbLight, 0);

	// Update instance buffer
	SHADER_INSTANCE_BUFFER cbInstance;
	ZeroMemory(&cbInstance, sizeof(cbInstance));

	for (int i = 0; i < MAX_INSTANCE_COUNT; i++)
	{
		float d = sinf((float)i / MAX_INSTANCE_COUNT * 2 * PI) + 2.0f;
		float x = sinf((float)i / MAX_INSTANCE_COUNT * 2 * PI) * 1000.0f * d;
		float y = sinf((float)i / MAX_INSTANCE_COUNT * 2 * PI * 8) * 1000.0f * d;
		float z = cosf((float)i / MAX_INSTANCE_COUNT * 2 * PI) * 1000.0f * d;
		RMatrix4 instanceMatrix = RMatrix4::CreateTranslation(x, y, z) * RMatrix4::CreateYAxisRotation((x + timer.TotalTime() * 0.1f * sinf(d)) * 180 / PI);

		cbInstance.instancedWorldMatrix[i] = instanceMatrix;
	}

	RRenderer.D3DImmediateContext()->Map(m_cbInstance, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
	memcpy(subres.pData, &cbInstance, sizeof(SHADER_INSTANCE_BUFFER));
	RRenderer.D3DImmediateContext()->Unmap(m_cbInstance, 0);

	// Update screen information
	SHADER_SCREEN_BUFFER cbScreen;
	ZeroMemory(&cbScreen, sizeof(cbScreen));

	cbScreen.ScreenSize = RVec2((float)RRenderer.GetClientWidth(), (float)RRenderer.GetClientHeight());

	RRenderer.D3DImmediateContext()->Map(m_cbScreen, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
	memcpy(subres.pData, &cbScreen, sizeof(SHADER_SCREEN_BUFFER));
	RRenderer.D3DImmediateContext()->Unmap(m_cbScreen, 0);

	// Update particle vertices
	ParticleDepthComparer cmp(m_CameraMatrix.GetRow(3), m_CameraMatrix.GetRow(3));
	std::sort(m_ParticleVert, m_ParticleVert + PARTICLE_COUNT, cmp);
	m_ParticleBuffer.UpdateDynamicVertexBuffer(&m_ParticleVert, sizeof(PARTICLE_VERTEX), PARTICLE_COUNT);
}

void FSGraphicsProjectApp::RenderScene()
{
	RRenderer.D3DImmediateContext()->VSSetConstantBuffers(0, 1, &m_cbPerObject);
	RRenderer.D3DImmediateContext()->VSSetConstantBuffers(1, 1, &m_cbScene);
	RRenderer.D3DImmediateContext()->VSSetConstantBuffers(2, 1, &m_cbInstance);
	RRenderer.D3DImmediateContext()->PSSetConstantBuffers(0, 1, &m_cbLight);
	RRenderer.D3DImmediateContext()->PSSetConstantBuffers(1, 1, &m_cbMaterial);
	RRenderer.D3DImmediateContext()->PSSetConstantBuffers(2, 1, &m_cbScreen);
	RRenderer.D3DImmediateContext()->PSSetSamplers(0, 1, &m_SamplerState);
	RRenderer.D3DImmediateContext()->PSSetSamplers(2, 1, &m_SamplerComparisonState);
	RRenderer.D3DImmediateContext()->GSSetConstantBuffers(1, 1, &m_cbScene);

	//=========================== Shadow Pass ===========================
	m_ShadowMap.SetupRenderTarget();
	RenderSinglePass(ShadowPass);

	//=========================== Scene Buffer Pass =====================
	RRenderer.SetRenderTarget(m_RenderTargetView, m_RenderTargetDepthView);

	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = static_cast<float>(RRenderer.GetClientWidth());
	vp.Height = static_cast<float>(RRenderer.GetClientHeight());
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	RRenderer.D3DImmediateContext()->RSSetViewports(1, &vp);

	RenderSinglePass(RefractionScenePass);

	//=========================== Normal Pass ===========================
	RRenderer.SetRenderTarget();
	RenderSinglePass(NormalPass);

	RRenderer.Present();
}

void FSGraphicsProjectApp::CreateSceneRenderTargetView()
{
	D3D11_TEXTURE2D_DESC renderTargetTextureDesc;
	renderTargetTextureDesc.Width = RRenderer.GetClientWidth();
	renderTargetTextureDesc.Height = RRenderer.GetClientHeight();
	renderTargetTextureDesc.MipLevels = 1;
	renderTargetTextureDesc.ArraySize = 1;
	renderTargetTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	renderTargetTextureDesc.SampleDesc.Count = 1;
	renderTargetTextureDesc.SampleDesc.Quality = 0;
	renderTargetTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	renderTargetTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	renderTargetTextureDesc.CPUAccessFlags = 0;
	renderTargetTextureDesc.MiscFlags = 0;

	RRenderer.D3DDevice()->CreateTexture2D(&renderTargetTextureDesc, 0, &m_RenderTargetBuffer);
	RRenderer.D3DDevice()->CreateRenderTargetView(m_RenderTargetBuffer, 0, &m_RenderTargetView);

	D3D11_SHADER_RESOURCE_VIEW_DESC rtsrvDesc;
	rtsrvDesc.Format = renderTargetTextureDesc.Format;
	rtsrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	rtsrvDesc.Texture2D.MostDetailedMip = 0;
	rtsrvDesc.Texture2D.MipLevels = 1;

	RRenderer.D3DDevice()->CreateShaderResourceView(m_RenderTargetBuffer, &rtsrvDesc, &m_RenderTargetSRV);

	renderTargetTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	renderTargetTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	RRenderer.D3DDevice()->CreateTexture2D(&renderTargetTextureDesc, 0, &m_RenderTargetDepthBuffer);
	RRenderer.D3DDevice()->CreateDepthStencilView(m_RenderTargetDepthBuffer, 0, &m_RenderTargetDepthView);
}

void FSGraphicsProjectApp::SetPerObjectConstBuffuer(const RMatrix4& world)
{
	SHADER_OBJECT_BUFFER cbObject;
	cbObject.worldMatrix = world;

	D3D11_MAPPED_SUBRESOURCE subres;
	RRenderer.D3DImmediateContext()->Map(m_cbPerObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
	memcpy(subres.pData, &cbObject, sizeof(cbObject));
	RRenderer.D3DImmediateContext()->Unmap(m_cbPerObject, 0);
}

void FSGraphicsProjectApp::RenderSinglePass(RenderPass pass)
{
	ID3D11ShaderResourceView* shadowMapSRV[] = { m_ShadowMap.GetRenderTargetDepthSRV() };
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	float timeNow = REngine::GetTimer().TotalTime();
	float loadingFadeInTime = 1.0f;

	// Update material buffer
	SHADER_MATERIAL_BUFFER cbMaterial;
	ZeroMemory(&cbMaterial, sizeof(cbMaterial));

	cbMaterial.SpecularColorAndPower = RVec4(1.0f, 1.0f, 1.0f, 512.0f);
	cbMaterial.GlobalOpacity = 1.0f;
	SetMaterialConstBuffer(&cbMaterial);

	RRenderer.D3DImmediateContext()->OMSetBlendState(m_BlendState[0], blendFactor, 0xFFFFFFFF);

	RRenderer.Clear();

	// Set up object world matrix
	SetPerObjectConstBuffuer(RMatrix4::IDENTITY);

	if (pass != ShadowPass)
	{
		// Set shadow map to pixel shader
		RRenderer.D3DImmediateContext()->PSSetShaderResources(2, 1, shadowMapSRV);

		// Draw skybox
		m_Skybox.Draw();

		// Clear depth buffer for skybox
		RRenderer.Clear(false, RColor(0, 0, 0));
	}

	// Draw star
	SetPerObjectConstBuffuer(RMatrix4::CreateTranslation(0.0f, 500.0f, 0.0f));

	if (pass == ShadowPass)
		m_DepthShader->Bind();
	else
		m_ColorShader->Bind();

	RRenderer.D3DImmediateContext()->IASetInputLayout(m_ColorPrimitiveIL);

	m_StarMesh.Draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Draw meshes

	// Draw city
	SetPerObjectConstBuffuer(m_FbxMeshObj.GetNodeTransform());

	if (pass == ShadowPass)
		m_FbxMeshObj.DrawWithShader(m_DepthShader);
	else
	{
		float opacity = (timeNow - m_FbxMeshObj.GetResourceTimestamp()) / loadingFadeInTime;
		if (opacity >= 0.0f && opacity <= 1.0f)
		{
			cbMaterial.GlobalOpacity = opacity;
			SetMaterialConstBuffer(&cbMaterial);
			RRenderer.D3DImmediateContext()->OMSetBlendState(m_BlendState[2], blendFactor, 0xFFFFFFFF);
			m_FbxMeshObj.Draw();
		}
		else
		{
			RRenderer.D3DImmediateContext()->OMSetBlendState(m_BlendState[0], blendFactor, 0xFFFFFFFF);
			m_FbxMeshObj.Draw();
		}
	}

	// Draw islands
	SetPerObjectConstBuffuer(m_IslandMeshObj.GetNodeTransform());

	if (pass == ShadowPass)
		m_IslandMeshObj.DrawWithShader(m_InstancedDepthShader, true, MAX_INSTANCE_COUNT);
	else
	{
		float opacity = (timeNow - m_IslandMeshObj.GetResourceTimestamp()) / loadingFadeInTime;
		if (opacity >= 0.0f && opacity <= 1.0f)
		{
			cbMaterial.GlobalOpacity = opacity;
			SetMaterialConstBuffer(&cbMaterial);
			RRenderer.D3DImmediateContext()->OMSetBlendState(m_BlendState[2], blendFactor, 0xFFFFFFFF);
			m_IslandMeshObj.Draw(true, MAX_INSTANCE_COUNT);
		}
		else
		{
			RRenderer.D3DImmediateContext()->OMSetBlendState(m_BlendState[0], blendFactor, 0xFFFFFFFF);
			m_IslandMeshObj.Draw(true, MAX_INSTANCE_COUNT);
		}
	}

	// Draw AO scene
	SetPerObjectConstBuffuer(m_AOSceneObj.GetNodeTransform());

	if (pass == ShadowPass)
		m_AOSceneObj.DrawWithShader(m_DepthShader);
	else
	{
		float opacity = (timeNow - m_AOSceneObj.GetResourceTimestamp()) / loadingFadeInTime;
		if (opacity >= 0.0f && opacity <= 1.0f)
		{
			cbMaterial.GlobalOpacity = opacity;
			SetMaterialConstBuffer(&cbMaterial);
			RRenderer.D3DImmediateContext()->OMSetBlendState(m_BlendState[2], blendFactor, 0xFFFFFFFF);
			m_AOSceneObj.Draw();
		}
		else
		{
			RRenderer.D3DImmediateContext()->OMSetBlendState(m_BlendState[0], blendFactor, 0xFFFFFFFF);
			m_AOSceneObj.Draw();
		}
	}

	// Draw bumped cube
	SetPerObjectConstBuffuer(RMatrix4::CreateTranslation(-1400.0f, 150.0f, 0.0f));

	if (pass == ShadowPass)
	{
		m_DepthShader->Bind();
		m_BumpCubeMesh.Draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
	else
	{
		RRenderer.D3DImmediateContext()->OMSetBlendState(m_BlendState[0], blendFactor, 0xFFFFFFFF);
		m_BumpLightingShader->Bind();
		RRenderer.D3DImmediateContext()->PSSetShaderResources(0, 1, m_BumpBaseTexture->GetPtrSRV());
		RRenderer.D3DImmediateContext()->PSSetShaderResources(1, 1, m_BumpNormalTexture->GetPtrSRV());
		RRenderer.D3DImmediateContext()->IASetInputLayout(m_BumpLightingIL);
		m_BumpCubeMesh.Draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	// Draw character
	SetPerObjectConstBuffuer(m_CharacterObj.GetNodeTransform());
	
	if (pass == ShadowPass)
		m_CharacterObj.DrawWithShader(m_DepthShader);
	else
	{
		float opacity = (timeNow - m_CharacterObj.GetResourceTimestamp()) / loadingFadeInTime;
		if (opacity >= 0.0f && opacity <= 1.0f)
		{
			cbMaterial.GlobalOpacity = opacity;
			SetMaterialConstBuffer(&cbMaterial);
			RRenderer.D3DImmediateContext()->OMSetBlendState(m_BlendState[2], blendFactor, 0xFFFFFFFF);
			m_CharacterObj.Draw();
		}
		else
		{
			RRenderer.D3DImmediateContext()->OMSetBlendState(m_BlendState[0], blendFactor, 0xFFFFFFFF);
			m_CharacterObj.Draw();
		}
	}

	// Draw tachikoma
	SetPerObjectConstBuffuer(m_TachikomaObj.GetNodeTransform());

	if (pass == ShadowPass)
		m_TachikomaObj.DrawWithShader(m_DepthShader);
	else if (pass == NormalPass)
	{
		float opacity = (timeNow - m_TachikomaObj.GetResourceTimestamp()) / loadingFadeInTime;
		if (opacity >= 0.0f && opacity <= 1.0f)
		{
			cbMaterial.GlobalOpacity = opacity;
			SetMaterialConstBuffer(&cbMaterial);
			RRenderer.D3DImmediateContext()->OMSetBlendState(m_BlendState[2], blendFactor, 0xFFFFFFFF);
			m_TachikomaObj.Draw();
		}
		else
		{
			RRenderer.D3DImmediateContext()->OMSetBlendState(m_BlendState[0], blendFactor, 0xFFFFFFFF);
			m_TachikomaObj.Draw();
		}
	}

	if (pass != ShadowPass)
	{
		// Draw particles
		RRenderer.D3DImmediateContext()->OMSetBlendState(m_BlendState[1], blendFactor, 0xFFFFFFFF);
		RRenderer.D3DImmediateContext()->OMSetDepthStencilState(m_DepthState[1], 0);

		SetPerObjectConstBuffuer(RMatrix4::CreateTranslation(0.0f, 150.0f, 150.0f));
		RRenderer.D3DImmediateContext()->PSSetShaderResources(0, 1, m_ParticleDiffuseTexture->GetPtrSRV());
		RRenderer.D3DImmediateContext()->PSSetShaderResources(1, 1, m_ParticleNormalTexture->GetPtrSRV());
		m_ParticleShader->Bind();
		RRenderer.D3DImmediateContext()->IASetInputLayout(m_ParticleIL);
		m_ParticleBuffer.Draw(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

		// Restore depth writing
		RRenderer.D3DImmediateContext()->OMSetDepthStencilState(m_DepthState[0], 0);
	}

	// Unbind all shader resources so we can write into it
	ID3D11ShaderResourceView* nullSRV[] = { nullptr };
	RRenderer.D3DImmediateContext()->PSSetShaderResources(0, 1, nullSRV);
	RRenderer.D3DImmediateContext()->PSSetShaderResources(1, 1, nullSRV);
	RRenderer.D3DImmediateContext()->PSSetShaderResources(2, 1, nullSRV);
}

void FSGraphicsProjectApp::SetMaterialConstBuffer(SHADER_MATERIAL_BUFFER* buffer)
{
	D3D11_MAPPED_SUBRESOURCE subres;
	RRenderer.D3DImmediateContext()->Map(m_cbMaterial, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
	memcpy(subres.pData, buffer, sizeof(SHADER_MATERIAL_BUFFER));
	RRenderer.D3DImmediateContext()->Unmap(m_cbMaterial, 0);
}
