//=============================================================================
// FSGraphicsProjectApp.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "FSGraphicsProjectApp.h"


struct ParticleDepthComparer
{
	RVec4 CamPos, CamDir;

	ParticleDepthComparer(const RVec4& camPos, const RVec4& camDir)
	{
		CamPos = camPos;
		CamDir = camDir;
	}

	bool operator()(const RVertex::PARTICLE_VERTEX &a, const RVertex::PARTICLE_VERTEX &b)
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

struct ObjectDepthComparer
{
	RVec3 CamPos;

	ObjectDepthComparer(const RVec4& camPos)
	{
		CamPos = camPos.ToVec3();
	}

	bool operator()(const RMatrix4 &a, const RMatrix4 &b)
	{
		return sqrDist(a.GetTranslation(), CamPos) > sqrDist(b.GetTranslation(), CamPos);
	}

	// Helper functions
	static float sqrDist(const RVec3& a, const RVec3 b)
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
	  m_RenderTargetView(nullptr)
{
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

	m_ParticleBuffer.Release();

	m_cbScreen.Release();
	m_cbMaterial.Release();
	m_cbLight.Release();
	m_cbPerObject.Release();
	m_cbScene.Release();
	m_cbInstance[0].Release();
	m_cbInstance[1].Release();
	m_cbBoneMatrices.Release();

	m_BumpCubeMesh.Release();

	m_StarMesh.Release();

	m_Skybox.Release();
	m_PostProcessor.Release();

	RShaderManager::Instance().UnloadAllShaders();
	RResourceManager::Instance().Destroy();
	m_DebugRenderer.Release();
}

bool FSGraphicsProjectApp::Initialize()
{
	RResourceManager::Instance().Initialize();
	CreateSceneRenderTargetView();

	// Initialize shaders
	RShaderManager::Instance().LoadShaders("../Shaders");
	
	m_ColorShader = RShaderManager::Instance().GetShaderResource("Color");
	m_BumpLightingShader = RShaderManager::Instance().GetShaderResource("BumpLighting");
	m_InstancedLightingShader = RShaderManager::Instance().GetShaderResource("InstancedLighting");
	m_DepthShader = RShaderManager::Instance().GetShaderResource("Depth");
	m_InstancedDepthShader = RShaderManager::Instance().GetShaderResource("InstancedDepth");
	m_ParticleShader = RShaderManager::Instance().GetShaderResource("Particle");
	m_RefractionShader = RShaderManager::Instance().GetShaderResource("Refraction");

	m_PostProcessor.Initialize();
	m_DebugRenderer.Initialize();

	// Create buffer for star mesh
	RVertex::PRIMITIVE_VERTEX starVertex[12];

	for (int i = 0; i < 10; i++)
	{
		float r = (i % 2 == 0) ? 100.0f : 50.0f;
		starVertex[i] = { RVec4(sinf(DEG_TO_RAD(i * 36)) * r, cosf(DEG_TO_RAD(i * 36)) * r, 0.0f, 1.0f),
						  RColor(1.0f, 0.0f, 0.0f, 1.0f) };
	}

	starVertex[10] = { RVec4(0.0f, 0.0f, -20.0f, 1.0f), RColor(1.0f, 1.0f, 0.0f, 1.0f) };
	starVertex[11] = { RVec4(0.0f, 0.0f, 20.0f, 1.0f), RColor(1.0f, 1.0f, 0.0f, 1.0f) };

	UINT32 starIndex[] = {
		0, 1, 10, 1, 2, 10, 2, 3, 10, 3, 4, 10, 4, 5, 10, 5, 6, 10, 6, 7, 10, 7, 8, 10, 8, 9, 10, 9, 0, 10,
		1, 0, 11, 2, 1, 11, 3, 2, 11, 4, 3, 11, 5, 4, 11, 6, 5, 11, 7, 6, 11, 8, 7, 11, 9, 8, 11, 0, 9, 11, };

	m_ColorPrimitiveIL = RVertexDeclaration::Instance().GetInputLayout(RVertex::PRIMITIVE_VERTEX::GetTypeName());
	m_StarMesh.CreateVertexBuffer(starVertex, sizeof(RVertex::PRIMITIVE_VERTEX), 12, m_ColorPrimitiveIL);
	m_StarMesh.CreateIndexBuffer(starIndex, sizeof(UINT32), sizeof(starIndex) / sizeof(UINT32));

	// Create buffer for bump cube
	RVertex::MESH_VERTEX boxVertex[] = 
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

	for (UINT32 i = 0; i < sizeof(boxVertex) / sizeof(RVertex::MESH_VERTEX); i++)
	{
		boxVertex[i].pos *= 100.0f;
	}

	UINT32 boxIndex[] =
	{
		0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7, 8, 9, 10, 8, 10, 11,
		12, 13, 14, 12, 14, 15, 16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23,
	};

	m_BumpLightingIL = RVertexDeclaration::Instance().GetInputLayout(RVertex::MESH_VERTEX::GetTypeName());
	m_BumpCubeMesh.CreateVertexBuffer(boxVertex, sizeof(RVertex::MESH_VERTEX), 24, m_BumpLightingIL);
	m_BumpCubeMesh.CreateIndexBuffer(boxIndex, sizeof(UINT32), 36);

	m_cbPerObject.Initialize();
	m_cbScene.Initialize();
	m_cbLight.Initialize();
	m_cbMaterial.Initialize();
	m_cbInstance[0].Initialize();
	m_cbInstance[1].Initialize();
	m_cbBoneMatrices.Initialize();
	m_cbScreen.Initialize();

	m_BumpBaseTexture = RResourceManager::Instance().LoadDDSTexture("../Assets/DiamondPlate.dds");
	m_BumpNormalTexture = RResourceManager::Instance().LoadDDSTexture("../Assets/DiamondPlateNormal.dds");
	RResourceManager::Instance().LoadDDSTexture("../Assets/powderpeak.dds");

	m_SceneMeshCity = RResourceManager::Instance().LoadFbxMesh("../Assets/city.fbx");
	m_FbxMeshObj.SetMesh(m_SceneMeshCity);

	m_MeshTachikoma = RResourceManager::Instance().LoadFbxMesh("../Assets/tachikoma.fbx");
	m_TachikomaObj.SetMesh(m_MeshTachikoma);
	m_TachikomaObj.SetPosition(RVec3(0.0f, 39.0f, 0.0f));

	RMaterial tachikomaMaterials[] =
	{
		{ m_RefractionShader, 1, RResourceManager::Instance().WrapSRV(m_RenderTargetSRV) },
	};

	m_TachikomaObj.SetMaterial(tachikomaMaterials, 1);


	m_AOSceneMesh = RResourceManager::Instance().LoadFbxMesh("../Assets/AO_Scene.fbx");
	m_AOSceneObj.SetMesh(m_AOSceneMesh);
	m_AOSceneObj.SetPosition(RVec3(-500.0f, 0.0f, 500.0f));

	const char* animationNames[] =
	{
		"../Assets/unitychan/FUCM02_0004_CH01_AS_MAWAK.fbx",
		"../Assets/unitychan/FUCM02_0025_MYA_TF_DOWN.fbx",
		"../Assets/unitychan/FUCM02_0029_Cha01_STL01_ScrewK01.fbx",
		"../Assets/unitychan/FUCM03_0005_Landing.fbx",
		"../Assets/unitychan/FUCM03_0019_HeadSpring.fbx",
		"../Assets/unitychan/FUCM05_0000_Idle.fbx",
		"../Assets/unitychan/FUCM05_0001_M_CMN_LJAB.fbx",
		"../Assets/unitychan/FUCM05_0022_M_RISING_P.fbx",
		"../Assets/unitychan/FUCM_0012b_EH_RUN_LP_NoZ.fbx",
		"../Assets/unitychan/FUCM_04_0001_RHiKick.fbx",
		"../Assets/unitychan/FUCM_04_0010_MC2_SAMK.fbx",
	};

	m_CurrentAnim = 0;

	for (int i = 0; i < sizeof(animationNames) / sizeof(const char*); i++)
	{
		m_CharacterAnimations.push_back(RResourceManager::Instance().LoadFbxMesh(animationNames[i]));
	}

	m_CharacterAnimation = m_CharacterAnimations[0];
	m_CharacterObj.SetMesh(RResourceManager::Instance().LoadFbxMesh("../Assets/unitychan/unitychan.fbx"));
	//m_CharacterAnimations[0] = m_CharacterAnimation = RResourceManager::Instance().LoadFbxMesh("../Assets/spin_combo.fbx");
	//m_CharacterObj.SetMesh(m_CharacterAnimation);
	m_CharacterObj.SetTransform(RMatrix4::CreateYAxisRotation(90) * RMatrix4::CreateTranslation(-1100.0f, 50.0f, 0.0f));

	RMesh* sphereMesh = RResourceManager::Instance().LoadFbxMesh("../Assets/Sphere.fbx");

	m_TransparentMesh.SetMesh(sphereMesh);
	m_TransparentMesh.SetOverridingShader(m_InstancedLightingShader);

	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			for (int k = 0; k < 5; k++)
			{
				cbInstance[1].instancedWorldMatrix[(i * 5 + j) * 5 + k] = RMatrix4::CreateTranslation(-1100.0f + i * 100, 500.0f + j * 100, 100.0f + k * 100);
			}
		}
	}

	m_SceneMeshIsland = RResourceManager::Instance().LoadFbxMesh("../Assets/Island.fbx");
	m_IslandTexture = RResourceManager::Instance().LoadDDSTexture("../Assets/TR_FloatingIsland02.dds");
	m_IslandMeshObj.SetMesh(m_SceneMeshIsland);
	m_IslandMeshObj.SetPosition(RVec3(0.0f, 0.0f, 500.0f));

	RMaterial islandMaterials[] =
	{
		{ m_InstancedLightingShader, 1, m_IslandTexture },
	};
	m_IslandMeshObj.SetMaterial(islandMaterials, 1);

	m_Skybox.CreateSkybox("../Assets/powderpeak.dds");

	m_Camera.SetPosition(RVec3(407.023712f, 339.007507f, 876.396484f));
	m_Camera.SetupView(65.0f, RRenderer.AspectRatio(), 1.0f, 10000.0f);
	m_CamPitch = 0.0900001600f;
	m_CamYaw = 3.88659930f;

	for (int i = 0; i < 3; i++)
	{
		m_ShadowMap[i].Initialize(1024, 1024);
	}

	m_ParticleIL = RVertexDeclaration::Instance().GetInputLayout(RVertex::PARTICLE_VERTEX::GetTypeName());
	m_ParticleBuffer.CreateVertexBuffer(nullptr, sizeof(RVertex::PARTICLE_VERTEX), PARTICLE_COUNT, m_ParticleIL, true);

	for (int i = 0; i < PARTICLE_COUNT; i++)
	{
		float x = Math::RandF(-2000.0f, 1000.0f),
			  y = Math::RandF(1000.0f, 1200.0f),
			  z = Math::RandF(-2000.0f, 1000.0f),
			  w = Math::RandF(500.0f, 750.0f);
		float ic = Math::RandF(0.5f, 1.0f);
		float offsetX = (rand() % 2 == 0) ? 0.0f : 0.5f;
		float offsetY = (rand() % 2 == 0) ? 0.0f : 0.5f;
		m_ParticleVert[i] = { RVec4(x, y, z, w), RVec4(ic, ic, ic, 1.0f), Math::RandF(0.0f, PI * 2), RVec4(0.5f, 0.5f, offsetX, offsetY) };
	}

	m_ParticleDiffuseTexture = RResourceManager::Instance().LoadDDSTexture("../Assets/smoke_diffuse.dds");
	m_ParticleNormalTexture = RResourceManager::Instance().LoadDDSTexture("../Assets/smoke_normal.dds");

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

	m_EnabledPostProcessor = 0;
	m_CharacterRot = 0.0f;
	m_CharacterYVel = 0.0f;
	m_RenderCollisionWireframe = false;

	m_SunVec = RVec3(sinf(1.0f) * 0.5f, 0.25f, cosf(1.0f) * 0.5f).GetNormalizedVec3() * 1000.0f;
	m_MaterialSpecular = RVec4(1.0f, 1.0f, 1.0f, 16.0f);

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
			m_CamPitch = max(-PI / 2, min(PI / 2, m_CamPitch));
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

	if (RInput.GetBufferedKeyState('4') == BKS_Pressed)
		m_EnabledPostProcessor = 1;

	if (RInput.GetBufferedKeyState('5') == BKS_Pressed)
		m_EnabledPostProcessor = 2;

	if (RInput.GetBufferedKeyState('6') == BKS_Pressed)
		m_EnabledPostProcessor = 0;

	if (RInput.GetBufferedKeyState('P') == BKS_Pressed)
		m_RenderCollisionWireframe = !m_RenderCollisionWireframe;

	if (RInput.GetBufferedKeyState('X') == BKS_Pressed)
	{
		m_CurrentAnim++;
		m_CurrentAnim %= m_CharacterAnimations.size();
		m_CharacterAnimation = m_CharacterAnimations[m_CurrentAnim];
	}

	if (RInput.GetBufferedKeyState('Z') == BKS_Pressed)
	{
		m_CurrentAnim--;
		if (m_CurrentAnim < 0)
			m_CurrentAnim = (int)m_CharacterAnimations.size() - 1;
		m_CharacterAnimation = m_CharacterAnimations[m_CurrentAnim];
	}

	if (RInput.GetBufferedKeyState(VK_SPACE) == BKS_Pressed)
		m_CharacterYVel = 4.0f;

	if (RInput.GetBufferedKeyState('R') == BKS_Pressed)
		m_CharacterObj.SetPosition(RVec3(-1100.0f, 50.0f, 0.0f));

	if (RInput.IsKeyDown(VK_LEFT))
		m_CharacterRot -= timer.DeltaTime() * 100.0f;

	if (RInput.IsKeyDown(VK_RIGHT))
		m_CharacterRot += timer.DeltaTime() * 100.0f;


	RVec3 camPos = m_Camera.GetPosition();
	RMatrix4 cameraMatrix = RMatrix4::CreateXAxisRotation(m_CamPitch * 180 / PI) * RMatrix4::CreateYAxisRotation(m_CamYaw * 180 / PI);

	RVec3 worldMoveVec = (RVec4(moveVec, 0.0f) * cameraMatrix).ToVec3();

	RAabb camAabb;
	camAabb.Expand(camPos + RVec3(5.0f, 5.0f, 5.0f));
	camAabb.Expand(camPos - RVec3(5.0f, 5.0f, 5.0f));

	const vector<RMeshElement>& sceneMeshElements = m_SceneMeshCity->GetMeshElements();
	for (UINT i = 0; i < sceneMeshElements.size(); i++)
	{
		worldMoveVec = camAabb.TestDynamicCollisionWithAabb(worldMoveVec, sceneMeshElements[i].GetAabb());

		if (m_RenderCollisionWireframe)
			m_DebugRenderer.DrawAabb(sceneMeshElements[i].GetAabb());
	}
	cameraMatrix.SetTranslation(camPos + worldMoveVec);
	m_Camera.SetTransform(cameraMatrix);

	RMatrix4 viewMatrix = m_Camera.GetViewMatrix();
	RMatrix4 projMatrix = m_Camera.GetProjectionMatrix();

	// Update scene constant buffer
	cbScene.viewMatrix = viewMatrix;
	cbScene.projMatrix = projMatrix;
	cbScene.viewProjMatrix = viewMatrix * projMatrix;
	cbScene.cameraPos = m_Camera.GetPosition();

	static bool toggleMovingSun = false;

	if (RInput.GetBufferedKeyState('U') == BKS_Pressed)
	{
		toggleMovingSun = !toggleMovingSun;
	}

	static float ct = timer.TotalTime() * 0.2f;

	if (toggleMovingSun)
	{
		ct = timer.TotalTime() * 0.2f;
		m_SunVec = RVec3(sinf(ct) * 0.5f, 0.25f, cosf(ct) * 0.5f).GetNormalizedVec3() * 1000.0f;
	}

	if (RInput.GetBufferedKeyState('I') == BKS_Pressed)
	{
		m_MaterialSpecular = RVec4(1.0f, 1.0f, 1.0f, Math::RandF(1.0f, 512.0f));
	}

	static RFrustum frustum = m_Camera.GetFrustum();
	static bool freezeFrustum = false;

	if (RInput.GetBufferedKeyState('O') == BKS_Pressed)
	{
		freezeFrustum = !freezeFrustum;
	}

	if (!freezeFrustum)
	{
		frustum = m_Camera.GetFrustum();
	}

	//m_DebugRenderer.DrawFrustum(frustum);

	float shadowSplitPoints[4] = { 0.0f, 0.05f, 0.2f, 0.5f };
	float lightDistance[3] = { 1000.0f, 2000.0f, 2000.0f };
	RColor frustumColor[3] = { RColor(1, 0, 0), RColor(0, 1, 0), RColor(0, 0, 1) };

	RMatrix4 shadowTransform(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	for (int i = 0; i < 3; i++)
	{
		RSphere s0 = CalculateFrustumBoundingSphere(frustum, shadowSplitPoints[i], shadowSplitPoints[i + 1]);

		if (freezeFrustum)
			m_DebugRenderer.DrawSphere(s0.center, s0.radius, frustumColor[i]);

		lightDistance[i] = max(lightDistance[i], s0.radius);

		RVec3 shadowTarget = s0.center;
		RVec3 shadowEyePos = shadowTarget + m_SunVec.GetNormalizedVec3() * lightDistance[i];

		RVec3 viewForward = (shadowTarget - shadowEyePos).GetNormalizedVec3();
		RVec3 viewRight = (RVec3(0, 1, 0).Cross(viewForward)).GetNormalizedVec3();
		RVec3 viewUp = (viewForward.Cross(viewRight)).GetNormalizedVec3();

		// Calculate texel offset in world space
		float texel_unit = s0.radius * 2.0f / 1024.0f;
		float texel_depth_unit = (s0.radius + lightDistance[i]) / 1024.0f;

		float dx = shadowEyePos.Dot(viewRight);
		float dy = shadowEyePos.Dot(viewUp);
		float dz = shadowEyePos.Dot(viewForward);
		float offset_x = dx - floorf(dx / texel_unit) * texel_unit;
		float offset_y = dy - floorf(dy / texel_unit) * texel_unit;
		float offset_z = dz - floorf(dz / texel_depth_unit) * texel_depth_unit;

		shadowTarget -= viewRight * offset_x + viewUp * offset_y/* + viewForward * offset_z*/;
		shadowEyePos -= viewRight * offset_x + viewUp * offset_y/* + viewForward * offset_z*/;

		RMatrix4 shadowViewMatrix = RMatrix4::CreateLookAtViewLH(shadowEyePos, shadowTarget, RVec3(0.0f, 1.0f, 0.0f));

		RFrustum shadowVolume = m_ShadowMap[i].GetFrustum();

		if (freezeFrustum)
			m_DebugRenderer.DrawFrustum(shadowVolume, frustumColor[i]);

		m_ShadowMap[i].SetViewMatrix(shadowViewMatrix);
		//m_ShadowMap.SetOrthogonalProjection(500.0f, 500.0f, 0.1f, 5000.0f);
		m_ShadowMap[i].SetOrthogonalProjection(s0.radius * 2.0f, s0.radius * 2.0f, 0.1f, s0.radius + lightDistance[i]);

		RMatrix4 shadowViewProjMatrix = m_ShadowMap[i].GetViewMatrix() * m_ShadowMap[i].GetProjectionMatrix();
		cbScene.shadowViewProjMatrix[i] = shadowViewProjMatrix;
		shadowViewProjMatrix *= shadowTransform;
		cbScene.shadowViewProjBiasedMatrix[i] = shadowViewProjMatrix;


		//RMatrix4 invShadowViewProj = shadowViewProjMatrix.Inverse();

		//for (int n = 0; n < 8; n++)
		//{
		//	RVec4 v = RVec4(shadowVolume.corners[n], 1);
		//	v = v * invShadowViewProj;
		//	v /= v.w;
		//	shadowVolume.corners[n] = v.ToVec3();
		//}

		//m_DebugRenderer.DrawFrustum(shadowVolume);
	}

	// Update light constant buffer
	ZeroMemory(&cbLight, sizeof(cbLight));

	// Setup ambient color
	cbLight.HighHemisphereAmbientColor = RVec4(0.1f, 0.1f, 0.1f, 1.0f);
	cbLight.LowHemisphereAmbientColor = RVec4(0.2f, 0.2f, 0.2f, 1.0f);

	if (m_EnableLights[0])
	{
		RVec4 dirLightVec = RVec4(RVec3(0.25f, 1.0f, 0.5f).GetNormalizedVec3(), 1.0f);

		cbLight.DirectionalLightCount = 1;
		cbLight.DirectionalLight[0].Color = RVec4(1.0f, 1.0f, 0.8f, 1.0f);
		cbLight.DirectionalLight[0].Direction = RVec4(m_SunVec.GetNormalizedVec3(), 1.0f);
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
		RVec4 spotlightPos = RVec4(m_Camera.GetPosition(), 2000.0f);
		RVec4 spotlightDir = RVec4(m_Camera.GetNodeTransform().GetForward(), 1.0f);
		RVec4 spotlightRatio = RVec4(0.97f, 0.9f, 0.0f, 0.0f);
		cbLight.Spotlight[0].PosAndRadius = spotlightPos;
		cbLight.Spotlight[0].Direction = spotlightDir;
		cbLight.Spotlight[0].ConeRatio = spotlightRatio;
		cbLight.Spotlight[0].Color = RVec4(0.85f, 0.85f, 1.0f, 1.0f);
	}

	cbLight.CameraPos = m_Camera.GetPosition();
	cbLight.CascadedShadowCount = 3;

	float camNear = m_Camera.GetNearPlane(),
		  camFar = m_Camera.GetFarPlane();

	RVec4 sPoints[4] =
	{
		RVec4(0, 0, Math::Lerp(camNear, camFar, shadowSplitPoints[0]), 1),
		RVec4(0, 0, Math::Lerp(camNear, camFar, shadowSplitPoints[1]), 1),
		RVec4(0, 0, Math::Lerp(camNear, camFar, shadowSplitPoints[2]), 1),
		RVec4(0, 0, Math::Lerp(camNear, camFar, shadowSplitPoints[3]), 1),
	};

	for (int i = 1; i < 4; i++)
	{
		sPoints[i] = sPoints[i] * projMatrix;
		sPoints[i] /= sPoints[i].w;
		cbLight.CascadedShadowDepth[i - 1] = sPoints[i].z;
		//m_DebugRenderer.DrawSphere(sPoints[i].ToVec3(), 50.0f);
	}

	m_cbLight.UpdateContent(&cbLight);

	// Update instance buffer
	ZeroMemory(&cbInstance[0], sizeof(cbInstance[0]));

	for (int i = 0; i < MAX_INSTANCE_COUNT; i++)
	{
		float d = sinf((float)i / MAX_INSTANCE_COUNT * 2 * PI) + 2.0f;
		float x = sinf((float)i / MAX_INSTANCE_COUNT * 2 * PI) * 1000.0f * d;
		float y = sinf((float)i / MAX_INSTANCE_COUNT * 2 * PI * 8) * 1000.0f * d;
		float z = cosf((float)i / MAX_INSTANCE_COUNT * 2 * PI) * 1000.0f * d;
		RMatrix4 instanceMatrix = RMatrix4::CreateTranslation(x, y, z) * RMatrix4::CreateYAxisRotation((x + timer.TotalTime() * 0.1f * sinf(d)) * 180 / PI);

		cbInstance[0].instancedWorldMatrix[i] = instanceMatrix;
	}

	m_cbInstance[0].UpdateContent(&cbInstance[0]);

	// Update screen information
	SHADER_GLOBAL_BUFFER cbScreen;
	ZeroMemory(&cbScreen, sizeof(cbScreen));

	cbScreen.ScreenSize = RVec2((float)RRenderer.GetClientWidth(), (float)RRenderer.GetClientHeight());
	cbScreen.UseGammaCorrection = RRenderer.UsingGammaCorrection() ? 1 : (m_EnabledPostProcessor == 1);

	m_cbScreen.UpdateContent(&cbScreen);

	// Update particle vertices
	ParticleDepthComparer cmp(m_Camera.GetPosition(), m_Camera.GetNodeTransform().GetForward());
	std::sort(m_ParticleVert, m_ParticleVert + PARTICLE_COUNT, cmp);
	m_ParticleBuffer.UpdateDynamicVertexBuffer(&m_ParticleVert, sizeof(RVertex::PARTICLE_VERTEX), PARTICLE_COUNT);

	ObjectDepthComparer objCmp(m_Camera.GetPosition());
	std::sort(cbInstance[1].instancedWorldMatrix, cbInstance[1].instancedWorldMatrix + 125, objCmp);

	m_cbInstance[1].UpdateContent(&cbInstance[1]);

	RVec3 charPos = m_CharacterObj.GetNodeTransform().GetTranslation();
	RMatrix4 charMatrix = RMatrix4::CreateYAxisRotation(m_CharacterRot);
	charMatrix.SetTranslation(charPos);
	m_CharacterObj.SetTransform(charMatrix);

	// Update animation
	RAnimation* animation = m_CharacterAnimation->GetAnimation();
	if (animation)
	{
		m_CharacterObj.GetMesh()->CacheAnimation(animation);

		static float currTime = animation->GetStartTime();

		// Changing time may cause start time greater than end time
		if  (currTime >= animation->GetEndTime() - 1)
		{
			currTime = animation->GetStartTime();
		}
		RVec3 start_offset = animation->GetRootPosition(currTime);

		currTime += timer.DeltaTime() * animation->GetFrameRate();
		bool startOver = false;

		while (currTime >= animation->GetEndTime() - 1)
		{
			currTime -= animation->GetEndTime() - animation->GetStartTime() - 1;
			startOver = true;
		}

		RVec3 offset = animation->GetRootPosition(currTime) - start_offset;
		if (startOver)
		{
			offset = animation->GetRootPosition(animation->GetEndTime() - 1) - start_offset +
					 animation->GetRootPosition(currTime) - animation->GetInitRootPosition();
		}

		m_CharacterYVel -= 10.0f * timer.DeltaTime();

		RVec3 nodePos = m_CharacterObj.GetNodeTransform().GetTranslation();
		RVec3 worldOffset = (RVec4(offset, 0.0f) * m_CharacterObj.GetNodeTransform()).ToVec3();
		worldOffset.y = m_CharacterYVel;

		if (RInput.IsKeyDown(VK_UP))
			worldOffset += m_CharacterObj.GetNodeTransform().GetForward() * timer.DeltaTime() * 500.0f;

		if (RInput.IsKeyDown(VK_DOWN))
			worldOffset -= m_CharacterObj.GetNodeTransform().GetForward() * timer.DeltaTime() * 500.0f;

		RAabb aabb;
		aabb.Expand(RVec3(-50.0f, 0.0f, -50.0f));
		aabb.Expand(RVec3(50.0f, 200.0f, 50.0f));
		aabb.pMin += nodePos;
		aabb.pMax += nodePos;

		m_DebugRenderer.DrawAabb(aabb);

		const vector<RMeshElement>& sceneMeshElements = m_SceneMeshCity->GetMeshElements();
		for (UINT i = 0; i < sceneMeshElements.size(); i++)
		{
			worldOffset = aabb.TestDynamicCollisionWithAabb(worldOffset, sceneMeshElements[i].GetAabb());
		}

		if (fabs(worldOffset.y) < fabs(m_CharacterYVel))
			m_CharacterYVel = 0.0f;

		m_CharacterObj.Translate(worldOffset);
		RVec3 invOffset = -animation->GetRootPosition(currTime);
		RMatrix4 rootInversedTranslation = RMatrix4::CreateTranslation(invOffset);

		SHADER_SKINNED_BUFFER cbSkinned;
		for (int i = 0; i < m_CharacterObj.GetMesh()->GetBoneCount(); i++)
		{
			RMatrix4 matrix;

			int boneId = m_CharacterObj.GetMesh()->GetCachedAnimationNodeId(animation, i);
			animation->GetNodePose(boneId, currTime, &matrix);

			cbSkinned.boneMatrix[i] = m_CharacterObj.GetMesh()->GetBoneInitInvMatrices(i) * matrix * rootInversedTranslation * m_CharacterObj.GetNodeTransform();
		}

		m_DebugRenderer.DrawLine(nodePos, nodePos + worldOffset * 10, RColor(1.0f, 0.0f, 0.0f), RColor(1.0f, 0.0f, 0.0f));

		m_cbBoneMatrices.UpdateContent(&cbSkinned);
		m_cbBoneMatrices.ApplyToShaders();
	}

	RVec3 pos = m_TachikomaObj.GetNodeTransform().GetTranslation();
	static float t = 0.0f;
	t += timer.DeltaTime() * 50.0f;
	m_TachikomaObj.SetTransform(RMatrix4::CreateYAxisRotation(t) * RMatrix4::CreateTranslation(pos));
	m_DebugRenderer.DrawAabb(m_TachikomaObj.GetAabb());
}

void FSGraphicsProjectApp::RenderScene()
{
	float width = static_cast<float>(RRenderer.GetClientWidth());
	float height = static_cast<float>(RRenderer.GetClientHeight());
	D3D11_VIEWPORT vp[2] =
	{
		{ 0.0f, 0.0f, width, height, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 300, 300, 0.0f, 1.0f },
	};

	m_cbPerObject.ApplyToShaders();
	m_cbLight.ApplyToShaders();
	m_cbMaterial.ApplyToShaders();
	m_cbScreen.ApplyToShaders();
	RRenderer.SetSamplerState(0, SamplerState_Texture);
	RRenderer.SetSamplerState(2, SamplerState_ShadowDepthComparison);

	//=========================== Shadow Pass ===========================
	for (int i = 0; i < 3; i++)
	{
		cbScene.cascadedShadowIndex = i;
		m_cbScene.UpdateContent(&cbScene);
		m_cbScene.ApplyToShaders();

		m_ShadowMap[i].SetupRenderTarget();
		RRenderer.Clear();
		RenderSinglePass(ShadowPass);
	}

	//=========================== Scene Buffer Pass =====================
	RRenderer.SetRenderTargets(1, &m_RenderTargetView, m_RenderTargetDepthView);

	RRenderer.D3DImmediateContext()->RSSetViewports(1, &vp[0]);
	RRenderer.Clear();
	RenderSinglePass(RefractionScenePass);

	//=========================== Normal Pass ===========================

	// 0 : Screen pass
	// 1 : Viewport pass
	for (int i = 0; i < 2; i++)
	{
		if (i == 0)
		{
			if (m_EnabledPostProcessor)
			{
				m_PostProcessor.SetupRenderTarget();
			}
			else
			{
				RRenderer.SetRenderTargets();
			}
		}
		else
		{
			static int shadowIndex = 0;
			if (RInput.GetBufferedKeyState(VK_TAB) == BKS_Pressed)
			{
				shadowIndex++;
				shadowIndex %= 3;
			}

			RMatrix4 viewMatrix = m_ShadowMap[shadowIndex].GetViewMatrix();
			RMatrix4 projMatrix = m_ShadowMap[shadowIndex].GetProjectionMatrix();

			cbScene.viewMatrix = viewMatrix;
			cbScene.projMatrix = projMatrix;
			cbScene.viewProjMatrix = viewMatrix * projMatrix;
			cbScene.cameraPos = m_SunVec;

			m_cbScene.UpdateContent(&cbScene);
	
			cbLight.CameraPos = m_SunVec;

			m_cbLight.UpdateContent(&cbLight);

			m_cbScene.ApplyToShaders();
			m_cbLight.ApplyToShaders();

			ParticleDepthComparer cmp(m_SunVec, -m_SunVec);
			std::sort(m_ParticleVert, m_ParticleVert + PARTICLE_COUNT, cmp);
			m_ParticleBuffer.UpdateDynamicVertexBuffer(&m_ParticleVert, sizeof(RVertex::PARTICLE_VERTEX), PARTICLE_COUNT);
	
			RRenderer.SetRenderTargets();
		}

		RRenderer.D3DImmediateContext()->RSSetViewports(1, &vp[i]);

		if (i == 0)
			RRenderer.Clear();
		else
			RRenderer.Clear(false, RColor(0.0f, 0.0f, 0.0f));

		RenderSinglePass(NormalPass);

		if (i == 0 && m_EnabledPostProcessor)
		{
			RRenderer.SetRenderTargets();
			RRenderer.Clear();

			m_PostProcessor.Draw(m_EnabledPostProcessor == 1 ? PPE_GammaCorrection : PPE_ColorEdgeDetection);
		}
	}

	RRenderer.Present();
	m_DebugRenderer.Reset();
}

void FSGraphicsProjectApp::OnResize(int width, int height)
{
	m_PostProcessor.RecreateLostResources();
	m_Camera.SetAspectRatio((float)width / (float)height);

	if (m_RenderTargetView)
	{
		SAFE_RELEASE(m_RenderTargetBuffer);
		SAFE_RELEASE(m_RenderTargetView);
		SAFE_RELEASE(m_RenderTargetSRV);
		SAFE_RELEASE(m_RenderTargetDepthBuffer);
		SAFE_RELEASE(m_RenderTargetDepthView);

		RResourceManager::Instance().UnloadSRVWrappers();

		CreateSceneRenderTargetView();

		RMaterial tachikomaMaterials[] =
		{
			{ m_RefractionShader, 1, RResourceManager::Instance().WrapSRV(m_RenderTargetSRV) },
		};

		m_TachikomaObj.SetMaterial(tachikomaMaterials, 1);
	}
}

void FSGraphicsProjectApp::CreateSceneRenderTargetView()
{
	D3D11_TEXTURE2D_DESC renderTargetTextureDesc;
	renderTargetTextureDesc.Width = RRenderer.GetClientWidth();
	renderTargetTextureDesc.Height = RRenderer.GetClientHeight();
	renderTargetTextureDesc.MipLevels = 1;
	renderTargetTextureDesc.ArraySize = 1;
	renderTargetTextureDesc.Format = RRenderer.UsingGammaCorrection() ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
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

void FSGraphicsProjectApp::SetPerObjectConstBuffer(const RMatrix4& world)
{
	SHADER_OBJECT_BUFFER cbObject;
	cbObject.worldMatrix = world;

	m_cbPerObject.UpdateContent(&cbObject);
}

void FSGraphicsProjectApp::RenderSinglePass(RenderPass pass)
{
	ID3D11ShaderResourceView* shadowMapSRV[] =
	{
		m_ShadowMap[0].GetRenderTargetDepthSRV(),
		m_ShadowMap[1].GetRenderTargetDepthSRV(),
		m_ShadowMap[2].GetRenderTargetDepthSRV(),
	};

	float timeNow = REngine::GetTimer().TotalTime();
	float loadingFadeInTime = 1.0f;

	// Update material buffer
	SHADER_MATERIAL_BUFFER cbMaterial;
	ZeroMemory(&cbMaterial, sizeof(cbMaterial));

	cbMaterial.SpecularColorAndPower = m_MaterialSpecular;
	cbMaterial.GlobalOpacity = 1.0f;
	SetMaterialConstBuffer(&cbMaterial);

	RRenderer.SetBlendState(Blend_Opaque);

	// Set up object world matrix
	SetPerObjectConstBuffer(RMatrix4::IDENTITY);

	if (pass != ShadowPass)
	{
		// Set shadow map to pixel shader
		RRenderer.D3DImmediateContext()->PSSetShaderResources(2, 3, shadowMapSRV);

		// Draw skybox
		m_Skybox.Draw();

		// Clear depth buffer for skybox
		RRenderer.Clear(false, RColor(0, 0, 0));
	}

	// Draw star
	SetPerObjectConstBuffer(RMatrix4::CreateTranslation(0.0f, 500.0f, 0.0f));

	if (pass == ShadowPass)
		m_DepthShader->Bind();
	else
		m_ColorShader->Bind();

	m_StarMesh.Draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Draw meshes

	// Draw city
	SetPerObjectConstBuffer(m_FbxMeshObj.GetNodeTransform());

	if (pass == ShadowPass)
		m_FbxMeshObj.DrawWithShader(m_DepthShader);
	else
	{
		float opacity = (timeNow - m_FbxMeshObj.GetResourceTimestamp()) / loadingFadeInTime;
		if (opacity >= 0.0f && opacity <= 1.0f)
		{
			cbMaterial.GlobalOpacity = opacity;
			SetMaterialConstBuffer(&cbMaterial);
			RRenderer.SetBlendState(Blend_AlphaToCoverage);
			m_FbxMeshObj.Draw();
		}
		else
		{
			RRenderer.SetBlendState(Blend_Opaque);
			m_FbxMeshObj.Draw();
		}
	}

	// Draw islands
	SetPerObjectConstBuffer(m_IslandMeshObj.GetNodeTransform());
	m_cbInstance[0].ApplyToShaders();

	if (pass == ShadowPass)
		m_IslandMeshObj.DrawWithShader(m_InstancedDepthShader, true, MAX_INSTANCE_COUNT);
	else
	{
		float opacity = (timeNow - m_IslandMeshObj.GetResourceTimestamp()) / loadingFadeInTime;
		if (opacity >= 0.0f && opacity <= 1.0f)
		{
			cbMaterial.GlobalOpacity = opacity;
			SetMaterialConstBuffer(&cbMaterial);
			RRenderer.SetBlendState(Blend_AlphaToCoverage);
			m_IslandMeshObj.Draw(true, MAX_INSTANCE_COUNT);
		}
		else
		{
			RRenderer.SetBlendState(Blend_Opaque);
			m_IslandMeshObj.Draw(true, MAX_INSTANCE_COUNT);
		}
	}

	// Draw AO scene
	SetPerObjectConstBuffer(m_AOSceneObj.GetNodeTransform());

	if (pass == ShadowPass)
		m_AOSceneObj.DrawWithShader(m_DepthShader);
	else
	{
		float opacity = (timeNow - m_AOSceneObj.GetResourceTimestamp()) / loadingFadeInTime;
		if (opacity >= 0.0f && opacity <= 1.0f)
		{
			cbMaterial.GlobalOpacity = opacity;
			SetMaterialConstBuffer(&cbMaterial);
			RRenderer.SetBlendState(Blend_AlphaToCoverage);
			m_AOSceneObj.Draw();
		}
		else
		{
			RRenderer.SetBlendState(Blend_Opaque);
			m_AOSceneObj.Draw();
		}
	}

	// Draw bumped cube
	SetPerObjectConstBuffer(RMatrix4::CreateTranslation(-1400.0f, 150.0f, 0.0f));

	if (pass == ShadowPass)
	{
		m_DepthShader->Bind();
		m_BumpCubeMesh.Draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
	else
	{
		cbMaterial.GlobalOpacity = 1.0f;
		SetMaterialConstBuffer(&cbMaterial);
		RRenderer.SetBlendState(Blend_AlphaToCoverage);
		m_BumpLightingShader->Bind();
		RRenderer.D3DImmediateContext()->PSSetShaderResources(0, 1, m_BumpBaseTexture->GetPtrSRV());
		RRenderer.D3DImmediateContext()->PSSetShaderResources(1, 1, m_BumpNormalTexture->GetPtrSRV());
		m_BumpCubeMesh.Draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	// Draw character
	SetPerObjectConstBuffer(m_CharacterObj.GetNodeTransform());
	
	if (pass == ShadowPass)
		m_CharacterObj.DrawDepthPass();
	else
	{
		float opacity = (timeNow - m_CharacterObj.GetResourceTimestamp()) / loadingFadeInTime;
		if (opacity >= 0.0f && opacity <= 1.0f)
		{
			cbMaterial.GlobalOpacity = opacity;
			SetMaterialConstBuffer(&cbMaterial);
			RRenderer.SetBlendState(Blend_AlphaToCoverage);
			m_CharacterObj.Draw();
		}
		else
		{
			cbMaterial.GlobalOpacity = 1.0f;
			SetMaterialConstBuffer(&cbMaterial);
			RRenderer.SetBlendState(Blend_AlphaBlending);
			m_CharacterObj.Draw();
		}
	}

	// Draw transparent spheres
	if (pass != ShadowPass)
	{
		m_cbInstance[1].ApplyToShaders();

		SetPerObjectConstBuffer(m_TransparentMesh.GetNodeTransform());

		float opacity = (timeNow - m_TransparentMesh.GetResourceTimestamp()) / loadingFadeInTime;
		if (opacity >= 0.0f && opacity <= 1.0f)
		{
			cbMaterial.GlobalOpacity = opacity * 0.25f;
			SetMaterialConstBuffer(&cbMaterial);
			RRenderer.SetBlendState(Blend_AlphaToCoverage);
			m_TransparentMesh.Draw(true, 125);
		}
		else
		{
			cbMaterial.GlobalOpacity = 0.25f;
			SetMaterialConstBuffer(&cbMaterial);
			RRenderer.SetBlendState(Blend_AlphaBlending);
			m_TransparentMesh.Draw(true, 125);
		}
	}

	// Draw tachikoma
	SetPerObjectConstBuffer(m_TachikomaObj.GetNodeTransform());

	if (pass == ShadowPass)
		m_TachikomaObj.DrawWithShader(m_DepthShader);
	else if (pass == NormalPass)
	{
		float opacity = (timeNow - m_TachikomaObj.GetResourceTimestamp()) / loadingFadeInTime;
		if (opacity >= 0.0f && opacity <= 1.0f)
		{
			cbMaterial.GlobalOpacity = opacity;
			SetMaterialConstBuffer(&cbMaterial);
			RRenderer.SetBlendState(Blend_AlphaToCoverage);
			m_TachikomaObj.Draw();
		}
		else
		{
			RRenderer.SetBlendState(Blend_Opaque);
			m_TachikomaObj.Draw();
		}
	}

	if (pass != ShadowPass)
	{
		// Draw particles
		RRenderer.SetBlendState(Blend_AlphaBlending);
		RRenderer.D3DImmediateContext()->OMSetDepthStencilState(m_DepthState[1], 0);

		SetPerObjectConstBuffer(RMatrix4::CreateTranslation(0.0f, 150.0f, 150.0f));
		RRenderer.D3DImmediateContext()->PSSetShaderResources(0, 1, m_ParticleDiffuseTexture->GetPtrSRV());
		RRenderer.D3DImmediateContext()->PSSetShaderResources(1, 1, m_ParticleNormalTexture->GetPtrSRV());
		m_ParticleShader->Bind();
		m_ParticleBuffer.Draw(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

		// Restore depth writing
		RRenderer.D3DImmediateContext()->OMSetDepthStencilState(m_DepthState[0], 0);

		// Draw debug lines
		RRenderer.SetBlendState(Blend_Opaque);
		SetPerObjectConstBuffer(RMatrix4::IDENTITY);
		m_DebugRenderer.Render();
	}

	// Unbind all shader resources so we can write into it
	ID3D11ShaderResourceView* nullSRV[8] = { nullptr };
	RRenderer.D3DImmediateContext()->PSSetShaderResources(0, 8, nullSRV);
}

void FSGraphicsProjectApp::SetMaterialConstBuffer(SHADER_MATERIAL_BUFFER* buffer)
{
	m_cbMaterial.UpdateContent(buffer);
	m_cbMaterial.ApplyToShaders();
}

RSphere FSGraphicsProjectApp::CalculateFrustumBoundingSphere(const RFrustum& frustum, float start, float end)
{
	RVec3 cornerPoints[8] = {
		RVec3::Lerp(frustum.corners[4], frustum.corners[0], start),
		RVec3::Lerp(frustum.corners[5], frustum.corners[1], start),
		RVec3::Lerp(frustum.corners[6], frustum.corners[2], start),
		RVec3::Lerp(frustum.corners[7], frustum.corners[3], start),
		RVec3::Lerp(frustum.corners[4], frustum.corners[0], end),
		RVec3::Lerp(frustum.corners[5], frustum.corners[1], end),
		RVec3::Lerp(frustum.corners[6], frustum.corners[2], end),
		RVec3::Lerp(frustum.corners[7], frustum.corners[3], end),
	};
	RVec3 nearMidPoint = (cornerPoints[0] + cornerPoints[1] + cornerPoints[2] + cornerPoints[3]) / 4.0f;
	RVec3 farMidPoint = (cornerPoints[4] + cornerPoints[5] + cornerPoints[6] + cornerPoints[7]) / 4.0f;
	RVec3 center = (farMidPoint + nearMidPoint) * 0.5f;
	RSphere s = { center, (cornerPoints[4] - center).Magnitude() };

	//m_DebugRenderer.DrawSphere(center, 50.0f, RColor(1, 0, 0));

	//for (int i = 4; i < 8; i++)
	//{
	//	m_DebugRenderer.DrawSphere(cornerPoints[i], 50.0f, RColor(1, 0, 0));
	//}

	return s;
}
