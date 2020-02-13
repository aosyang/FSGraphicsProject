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

	bool operator()(const RVertexType::Particle &a, const RVertexType::Particle &b)
	{
		//return dot(a.pos - CamPos, CamDir) > dot(b.pos - CamPos, CamDir);
		return sqrDist(a.pos, CamPos) > sqrDist(b.pos, CamPos);
	}

	// Helper functions
	static float dot(const RVertexType::Vec4Data& a, const RVertexType::Vec4Data& b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	static float sqrDist(const RVertexType::Vec4Data& a, const RVertexType::Vec4Data b)
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
		return (b - a).SquaredMagitude();
	}
};


FSGraphicsProjectApp::FSGraphicsProjectApp()
	: m_ColorPrimitiveIL(nullptr),
	  m_ColorShader(nullptr),
	  m_RenderTargetView(nullptr),
	  m_Camera(nullptr)
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

	m_cbInstance[0].Release();
	m_cbInstance[1].Release();
	m_cbInstance[2].Release();

	m_BumpCubeMesh.Release();

	m_StarMesh.Release();

	m_Skybox.Release();
	m_Scene.Release();
}

bool FSGraphicsProjectApp::Initialize()
{
	CreateSceneRenderTargetView();

	m_ColorShader = RShaderManager::Instance().GetShaderResource("Color");
	m_BumpLightingShader = RShaderManager::Instance().GetShaderResource("BumpLighting");
	m_DepthShader = RShaderManager::Instance().GetShaderResource("Depth");
	m_ParticleShader = RShaderManager::Instance().GetShaderResource("Particle");
	m_RefractionShader = RShaderManager::Instance().GetShaderResource("Refraction");

	m_PostProcessingEffects[PPE_GammaCorrection] = GPostProcessorManager.CreateEffectFromFile("GammaCorrection", "PostProcessor_GammaCorrection.hlsl");
	m_PostProcessingEffects[PPE_ColorEdgeDetection] = GPostProcessorManager.CreateEffectFromFile("ColorEdgeDetection", "PostProcessor_ColorEdgeDetection.hlsl");

	m_Camera = m_Scene.CreateSceneObjectOfType<RCamera>();
	m_FbxMeshObj = m_Scene.CreateSceneObjectOfType<RSMeshObject>();
	m_TachikomaObj = m_Scene.CreateSceneObjectOfType<RSMeshObject>();
	m_CharacterObj = m_Scene.CreateSceneObjectOfType<RSMeshObject>();
	m_AOSceneObj = m_Scene.CreateSceneObjectOfType<RSMeshObject>();
	m_IslandMeshObj = m_Scene.CreateSceneObjectOfType<RSMeshObject>();
	m_TransparentMesh = m_Scene.CreateSceneObjectOfType<RSMeshObject>();

	// Create buffer for star mesh
	RVertexType::PositionColor starVertex[12];

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

	m_ColorPrimitiveIL = RVertexDeclaration::Instance().GetInputLayout<RVertexType::PositionColor>();
	m_StarMesh.CreateVertexBuffer(starVertex, sizeof(RVertexType::PositionColor), 12, m_ColorPrimitiveIL);
	m_StarMesh.CreateIndexBuffer(starIndex, sizeof(UINT32), sizeof(starIndex) / sizeof(UINT32));

	// Create buffer for bump cube
	RVertexType::Mesh boxVertex[] = 
	{
		{ RVertexType::Vec3Data(-1.0f, -1.0f, -1.0f), RVertexType::Vec3Data(0.0f, 0.0f, -1.0f), RVertexType::Vec3Data(1.0f, 0.0f, 0.0f), RVertexType::Vec2Data(0.0f, 0.0f) },
		{ RVertexType::Vec3Data(-1.0f,  1.0f, -1.0f), RVertexType::Vec3Data(0.0f, 0.0f, -1.0f), RVertexType::Vec3Data(1.0f, 0.0f, 0.0f), RVertexType::Vec2Data(0.0f, 1.0f) },
		{ RVertexType::Vec3Data( 1.0f,  1.0f, -1.0f), RVertexType::Vec3Data(0.0f, 0.0f, -1.0f), RVertexType::Vec3Data(1.0f, 0.0f, 0.0f), RVertexType::Vec2Data(1.0f, 1.0f) },
		{ RVertexType::Vec3Data( 1.0f, -1.0f, -1.0f), RVertexType::Vec3Data(0.0f, 0.0f, -1.0f), RVertexType::Vec3Data(1.0f, 0.0f, 0.0f), RVertexType::Vec2Data(1.0f, 0.0f) },

		{ RVertexType::Vec3Data( 1.0f, -1.0f, -1.0f), RVertexType::Vec3Data(1.0f, 0.0f, 0.0f), RVertexType::Vec3Data(0.0f, 0.0f, 1.0f), RVertexType::Vec2Data(0.0f, 0.0f) },
		{ RVertexType::Vec3Data( 1.0f,  1.0f, -1.0f), RVertexType::Vec3Data(1.0f, 0.0f, 0.0f), RVertexType::Vec3Data(0.0f, 0.0f, 1.0f), RVertexType::Vec2Data(0.0f, 1.0f) },
		{ RVertexType::Vec3Data( 1.0f,  1.0f,  1.0f), RVertexType::Vec3Data(1.0f, 0.0f, 0.0f), RVertexType::Vec3Data(0.0f, 0.0f, 1.0f), RVertexType::Vec2Data(1.0f, 1.0f) },
		{ RVertexType::Vec3Data( 1.0f, -1.0f,  1.0f), RVertexType::Vec3Data(1.0f, 0.0f, 0.0f), RVertexType::Vec3Data(0.0f, 0.0f, 1.0f), RVertexType::Vec2Data(1.0f, 0.0f) },

		{ RVertexType::Vec3Data( 1.0f, -1.0f,  1.0f), RVertexType::Vec3Data(0.0f, 0.0f, 1.0f), RVertexType::Vec3Data(-1.0f, 0.0f, 0.0f), RVertexType::Vec2Data(0.0f, 0.0f) },
		{ RVertexType::Vec3Data( 1.0f,  1.0f,  1.0f), RVertexType::Vec3Data(0.0f, 0.0f, 1.0f), RVertexType::Vec3Data(-1.0f, 0.0f, 0.0f), RVertexType::Vec2Data(0.0f, 1.0f) },
		{ RVertexType::Vec3Data(-1.0f,  1.0f,  1.0f), RVertexType::Vec3Data(0.0f, 0.0f, 1.0f), RVertexType::Vec3Data(-1.0f, 0.0f, 0.0f), RVertexType::Vec2Data(1.0f, 1.0f) },
		{ RVertexType::Vec3Data(-1.0f, -1.0f,  1.0f), RVertexType::Vec3Data(0.0f, 0.0f, 1.0f), RVertexType::Vec3Data(-1.0f, 0.0f, 0.0f), RVertexType::Vec2Data(1.0f, 0.0f) },

		{ RVertexType::Vec3Data(-1.0f, -1.0f,  1.0f), RVertexType::Vec3Data(-1.0f, 0.0f, 0.0f), RVertexType::Vec3Data(0.0f, 0.0f, -1.0f), RVertexType::Vec2Data(0.0f, 0.0f) },
		{ RVertexType::Vec3Data(-1.0f,  1.0f,  1.0f), RVertexType::Vec3Data(-1.0f, 0.0f, 0.0f), RVertexType::Vec3Data(0.0f, 0.0f, -1.0f), RVertexType::Vec2Data(0.0f, 1.0f) },
		{ RVertexType::Vec3Data(-1.0f,  1.0f, -1.0f), RVertexType::Vec3Data(-1.0f, 0.0f, 0.0f), RVertexType::Vec3Data(0.0f, 0.0f, -1.0f), RVertexType::Vec2Data(1.0f, 1.0f) },
		{ RVertexType::Vec3Data(-1.0f, -1.0f, -1.0f), RVertexType::Vec3Data(-1.0f, 0.0f, 0.0f), RVertexType::Vec3Data(0.0f, 0.0f, -1.0f), RVertexType::Vec2Data(1.0f, 0.0f) },

		{ RVertexType::Vec3Data(-1.0f,  1.0f, -1.0f), RVertexType::Vec3Data(0.0f, 1.0f, 0.0f), RVertexType::Vec3Data(1.0f, 0.0f, 0.0f), RVertexType::Vec2Data(0.0f, 0.0f) },
		{ RVertexType::Vec3Data(-1.0f,  1.0f,  1.0f), RVertexType::Vec3Data(0.0f, 1.0f, 0.0f), RVertexType::Vec3Data(1.0f, 0.0f, 0.0f), RVertexType::Vec2Data(0.0f, 1.0f) },
		{ RVertexType::Vec3Data( 1.0f,  1.0f,  1.0f), RVertexType::Vec3Data(0.0f, 1.0f, 0.0f), RVertexType::Vec3Data(1.0f, 0.0f, 0.0f), RVertexType::Vec2Data(1.0f, 1.0f) },
		{ RVertexType::Vec3Data( 1.0f,  1.0f, -1.0f), RVertexType::Vec3Data(0.0f, 1.0f, 0.0f), RVertexType::Vec3Data(1.0f, 0.0f, 0.0f), RVertexType::Vec2Data(1.0f, 0.0f) },

		{ RVertexType::Vec3Data(-1.0f, -1.0f,  1.0f), RVertexType::Vec3Data(0.0f, -1.0f, 0.0f), RVertexType::Vec3Data(1.0f, 0.0f, 0.0f), RVertexType::Vec2Data(0.0f, 0.0f) },
		{ RVertexType::Vec3Data(-1.0f, -1.0f, -1.0f), RVertexType::Vec3Data(0.0f, -1.0f, 0.0f), RVertexType::Vec3Data(1.0f, 0.0f, 0.0f), RVertexType::Vec2Data(0.0f, 1.0f) },
		{ RVertexType::Vec3Data( 1.0f, -1.0f, -1.0f), RVertexType::Vec3Data(0.0f, -1.0f, 0.0f), RVertexType::Vec3Data(1.0f, 0.0f, 0.0f), RVertexType::Vec2Data(1.0f, 1.0f) },
		{ RVertexType::Vec3Data( 1.0f, -1.0f,  1.0f), RVertexType::Vec3Data(0.0f, -1.0f, 0.0f), RVertexType::Vec3Data(1.0f, 0.0f, 0.0f), RVertexType::Vec2Data(1.0f, 0.0f) },
	};

	for (UINT32 i = 0; i < sizeof(boxVertex) / sizeof(RVertexType::Mesh); i++)
	{
		boxVertex[i].pos.x *= 100.0f;
		boxVertex[i].pos.y *= 100.0f;
		boxVertex[i].pos.z *= 100.0f;
	}

	UINT32 boxIndex[] =
	{
		0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7, 8, 9, 10, 8, 10, 11,
		12, 13, 14, 12, 14, 15, 16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23,
	};

	m_BumpLightingIL = RVertexDeclaration::Instance().GetInputLayout<RVertexType::Mesh>();
	m_BumpCubeMesh.CreateVertexBuffer(boxVertex, sizeof(RVertexType::Mesh), 24, m_BumpLightingIL);
	m_BumpCubeMesh.CreateIndexBuffer(boxIndex, sizeof(UINT32), 36);

	m_cbInstance[0].Initialize();
	m_cbInstance[1].Initialize();
	m_cbInstance[2].Initialize();

	m_BumpBaseTexture = RResourceManager::Instance().LoadResource<RTexture>("/DiamondPlate.dds");
	m_BumpNormalTexture = RResourceManager::Instance().LoadResource<RTexture>("/DiamondPlateNormal.dds");
	RResourceManager::Instance().LoadResource<RTexture>("/powderpeak.dds");

	m_SceneMeshCity = RResourceManager::Instance().LoadResource<RMesh>("/city.fbx");
	m_FbxMeshObj->SetMesh(m_SceneMeshCity);

	m_MeshTachikoma = RResourceManager::Instance().LoadResource<RMesh>("/tachikoma.fbx");
	m_TachikomaObj->SetMesh(m_MeshTachikoma);
	m_TachikomaObj->SetPosition(RVec3(0.0f, 39.0f, 0.0f));

	RMaterial* RefractionMaterial = RResourceManager::Instance().CreateNewResource<RMaterial>("Refraction");
	RefractionMaterial->SetShader(m_RefractionShader);
	RefractionMaterial->SetTextureSlot(0, RResourceManager::Instance().WrapShaderResourceViewInTexture(m_RenderTargetSRV));
	std::vector<RMaterial*> TachikomaMaterials;
	TachikomaMaterials.push_back(RefractionMaterial);

	m_TachikomaObj->SetMaterials(TachikomaMaterials);


	m_AOSceneMesh = RResourceManager::Instance().LoadResource<RMesh>("/AO_Scene.fbx");
	m_AOSceneObj->SetMesh(m_AOSceneMesh);
	m_AOSceneObj->SetPosition(RVec3(-500.0f, 0.0f, 500.0f));

	const char* animationNames[] =
	{
		"/unitychan/FUCM02_0004_CH01_AS_MAWAK.fbx",
		"/unitychan/FUCM02_0025_MYA_TF_DOWN.fbx",
		"/unitychan/FUCM02_0029_Cha01_STL01_ScrewK01.fbx",
		"/unitychan/FUCM03_0005_Landing.fbx",
		"/unitychan/FUCM03_0019_HeadSpring.fbx",
		"/unitychan/FUCM05_0000_Idle.fbx",
		"/unitychan/FUCM05_0001_M_CMN_LJAB.fbx",
		"/unitychan/FUCM05_0022_M_RISING_P.fbx",
		"/unitychan/FUCM_0012b_EH_RUN_LP_NoZ.fbx",
		"/unitychan/FUCM_04_0001_RHiKick.fbx",
		"/unitychan/FUCM_04_0010_MC2_SAMK.fbx",
	};

	m_CurrentAnim = 0;

	for (int i = 0; i < sizeof(animationNames) / sizeof(const char*); i++)
	{
		m_CharacterAnimations.push_back(RResourceManager::Instance().LoadResource<RMesh>(animationNames[i]));
	}

	m_CharacterAnimation = m_CharacterAnimations[0];
	m_CharacterObj->SetMesh(RResourceManager::Instance().LoadResource<RMesh>("/unitychan/unitychan.fbx"));
	//m_CharacterAnimations[0] = m_CharacterAnimation = RResourceManager::Instance().LoadResource<RMesh>("/spin_combo.fbx");
	//m_CharacterObj->SetMesh(m_CharacterAnimation);
	m_CharacterObj->SetTransform(RMatrix4::CreateYAxisRotation(90) * RMatrix4::CreateTranslation(-1100.0f, 50.0f, 0.0f));

	RMesh* sphereMesh = RResourceManager::Instance().LoadResource<RMesh>("/Sphere.fbx");

	m_TransparentMesh->SetMesh(sphereMesh);

	// Generate random position for floating islands
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

	m_SceneMeshIsland = RResourceManager::Instance().LoadResource<RMesh>("/Island.fbx");
	m_IslandMeshObj->SetMesh(m_SceneMeshIsland);
	m_IslandMeshObj->SetPosition(RVec3(0.0f, 0.0f, 500.0f));

	m_Skybox.CreateSkybox("/powderpeak.dds");

	m_Camera->SetPosition(RVec3(407.023712f, 339.007507f, 876.396484f));
	m_Camera->SetupView(65.0f, GRenderer.AspectRatio(), 1.0f, 10000.0f);
	m_CamPitch = 0.0900001600f;
	m_CamYaw = 3.88659930f;

	for (int i = 0; i < 3; i++)
	{
		m_ShadowMap[i].Initialize(1024, 1024);
	}

	m_ParticleIL = RVertexDeclaration::Instance().GetInputLayout<RVertexType::Particle>();
	m_ParticleBuffer.CreateVertexBuffer(nullptr, sizeof(RVertexType::Particle), PARTICLE_COUNT, m_ParticleIL, true);

	for (int i = 0; i < PARTICLE_COUNT; i++)
	{
		float x = RMath::RandRangedF(-2000.0f, 1000.0f),
			  y = RMath::RandRangedF(1000.0f, 1200.0f),
			  z = RMath::RandRangedF(-2000.0f, 1000.0f),
			  w = RMath::RandRangedF(500.0f, 750.0f);					// Particle radius
		float ic = RMath::RandRangedF(0.5f, 1.0f);						// Particle color
		float offsetX = (rand() % 2 == 0) ? 0.0f : 0.5f;
		float offsetY = (rand() % 2 == 0) ? 0.0f : 0.5f;
		m_ParticleVert[i] = { RVec4(x, y, z, w), RVec4(ic, ic, ic, 1.0f), RMath::RandRangedF(0.0f, PI * 2), RVec4(0.5f, 0.5f, offsetX, offsetY) };
		m_ParticleAabb.ExpandBySphere(RVec3(x, y, z), w);
	}

	m_ParticleDiffuseTexture = RResourceManager::Instance().LoadResource<RTexture>("/smoke_diffuse.dds");
	m_ParticleNormalTexture = RResourceManager::Instance().LoadResource<RTexture>("/smoke_normal.dds");

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

	GRenderer.D3DDevice()->CreateDepthStencilState(&depthDesc, &m_DepthState[0]);

	depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	GRenderer.D3DDevice()->CreateDepthStencilState(&depthDesc, &m_DepthState[1]);

	m_EnabledPostProcessor = 0;
	m_CharacterRot = 0.0f;
	m_CharacterYVel = 0.0f;
	m_RenderCollisionWireframe = false;

	m_SunVec = RVec3(sinf(1.0f) * 0.5f, 0.25f, cosf(1.0f) * 0.5f).GetNormalized() * 1000.0f;
	m_MaterialSpecular = RVec4(1.0f, 1.0f, 1.0f, 16.0f);

	m_MeshInstanceCount = 1;

	return true;
}

void FSGraphicsProjectApp::UpdateScene(const RTimer& timer)
{
	if (RInput.GetBufferedKeyState(VK_RBUTTON) == EBufferedKeyState::Pressed)
	{
		RInput.HideCursor();
		RInput.LockCursor();
	}

	if (RInput.GetBufferedKeyState(VK_RBUTTON) == EBufferedKeyState::Released)
	{
		RInput.ShowCursor();
		RInput.UnlockCursor();
	}

	if (RInput.IsKeyDown(VK_RBUTTON))
	{
		int dx, dy;
		RInput.GetRelativeCursorPosition(dx, dy);
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
	if (RInput.GetBufferedKeyState('F') == EBufferedKeyState::Pressed)
		m_EnableLights[2] = !m_EnableLights[2];

	if (RInput.GetBufferedKeyState('1') == EBufferedKeyState::Pressed)
		m_EnableLights[0] = !m_EnableLights[0];

	if (RInput.GetBufferedKeyState('2') == EBufferedKeyState::Pressed)
		m_EnableLights[1] = !m_EnableLights[1];

	if (RInput.GetBufferedKeyState('4') == EBufferedKeyState::Pressed)
		m_EnabledPostProcessor = 1;

	if (RInput.GetBufferedKeyState('5') == EBufferedKeyState::Pressed)
		m_EnabledPostProcessor = 2;

	if (RInput.GetBufferedKeyState('6') == EBufferedKeyState::Pressed)
		m_EnabledPostProcessor = 0;

	if (RInput.GetBufferedKeyState('P') == EBufferedKeyState::Pressed)
		m_RenderCollisionWireframe = !m_RenderCollisionWireframe;

	if (RInput.GetBufferedKeyState('X') == EBufferedKeyState::Pressed)
	{
		m_CurrentAnim++;
		m_CurrentAnim %= m_CharacterAnimations.size();
		m_CharacterAnimation = m_CharacterAnimations[m_CurrentAnim];
	}

	if (RInput.GetBufferedKeyState('Z') == EBufferedKeyState::Pressed)
	{
		m_CurrentAnim--;
		if (m_CurrentAnim < 0)
			m_CurrentAnim = (int)m_CharacterAnimations.size() - 1;
		m_CharacterAnimation = m_CharacterAnimations[m_CurrentAnim];
	}

	if (RInput.GetBufferedKeyState(VK_SPACE) == EBufferedKeyState::Pressed)
		m_CharacterYVel = 4.0f;

	if (RInput.GetBufferedKeyState('R') == EBufferedKeyState::Pressed)
		m_CharacterObj->SetPosition(RVec3(-1100.0f, 50.0f, 0.0f));

	if (RInput.IsKeyDown(VK_LEFT))
		m_CharacterRot -= timer.DeltaTime() * 100.0f;

	if (RInput.IsKeyDown(VK_RIGHT))
		m_CharacterRot += timer.DeltaTime() * 100.0f;


	RVec3 camPos = m_Camera->GetPosition();
	RMatrix4 cameraMatrix = RMatrix4::CreateXAxisRotation(m_CamPitch * 180 / PI) * RMatrix4::CreateYAxisRotation(m_CamYaw * 180 / PI);

	RVec3 worldMoveVec = (RVec4(moveVec, 0.0f) * cameraMatrix).ToVec3();

	RAabb camAabb;
	camAabb.Expand(camPos + RVec3(5.0f, 5.0f, 5.0f));
	camAabb.Expand(camPos - RVec3(5.0f, 5.0f, 5.0f));

	const std::vector<RMeshElement>& sceneMeshElements = m_SceneMeshCity->GetMeshElements();
	for (UINT i = 0; i < sceneMeshElements.size(); i++)
	{
		worldMoveVec = camAabb.TestDynamicCollisionWithAabb(worldMoveVec, sceneMeshElements[i].GetAabb());

		if (m_RenderCollisionWireframe)
			GDebugRenderer.DrawAabb(sceneMeshElements[i].GetAabb());
	}
	m_Camera->Translate(worldMoveVec, ETransformSpace::World);
	m_Camera->SetRotation(RQuat::Euler(m_CamPitch, m_CamYaw, 0.0f));

	RMatrix4 viewMatrix = m_Camera->GetViewMatrix();
	RMatrix4 projMatrix = m_Camera->GetProjectionMatrix();

	// Update scene constant buffer
	auto& cbScene = RConstantBuffers::cbScene.Data;
	cbScene.viewMatrix = viewMatrix;
	cbScene.projMatrix = projMatrix;
	cbScene.viewProjMatrix = viewMatrix * projMatrix;
	cbScene.cameraPos = m_Camera->GetPosition();

	static bool toggleMovingSun = false;

	if (RInput.GetBufferedKeyState('U') == EBufferedKeyState::Pressed)
	{
		toggleMovingSun = !toggleMovingSun;
	}

	static float ct = timer.TotalTime() * 0.2f;

	if (toggleMovingSun)
	{
		ct += timer.DeltaTime() * 0.2f;
		m_SunVec = RVec3(sinf(ct) * 0.5f, cosf(ct) * 0.5f, 0.25f).GetNormalized() * 1000.0f;
	}

	if (RInput.GetBufferedKeyState('I') == EBufferedKeyState::Pressed)
	{
		m_MaterialSpecular = RVec4(1.0f, 1.0f, 1.0f, RMath::RandRangedF(1.0f, 512.0f));
	}

	static RFrustum frustum = m_Camera->GetFrustum();
	static bool freezeFrustum = false;

	if (RInput.GetBufferedKeyState('O') == EBufferedKeyState::Pressed)
	{
		freezeFrustum = !freezeFrustum;
	}

	if (!freezeFrustum)
	{
		frustum = m_Camera->GetFrustum();
	}

	//GDebugRenderer.DrawFrustum(frustum);

	float shadowSplitPoints[4] = { 0.0f, 0.05f, 0.4f, 1.0f };
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
			GDebugRenderer.DrawSphere(s0.center, s0.radius, frustumColor[i]);

		lightDistance[i] = max(lightDistance[i], s0.radius);

		RVec3 shadowTarget = s0.center;
		RVec3 shadowEyePos = shadowTarget + m_SunVec.GetNormalized() * lightDistance[i];

		RVec3 viewForward = (shadowTarget - shadowEyePos).GetNormalized();
		RVec3 viewRight = RVec3::Cross(RVec3(0, 1, 0), viewForward).GetNormalized();
		RVec3 viewUp = RVec3::Cross(viewForward, viewRight).GetNormalized();

		// Calculate texel offset in world space
		float texel_unit = s0.radius * 2.0f / 1024.0f;
		float texel_depth_unit = (s0.radius + lightDistance[i]) / 1024.0f;

		float dx = RVec3::Dot(shadowEyePos, viewRight);
		float dy = RVec3::Dot(shadowEyePos, viewUp);
		float dz = RVec3::Dot(shadowEyePos, viewForward);
		float offset_x = dx - floorf(dx / texel_unit) * texel_unit;
		float offset_y = dy - floorf(dy / texel_unit) * texel_unit;
		float offset_z = dz - floorf(dz / texel_depth_unit) * texel_depth_unit;

		shadowTarget -= viewRight * offset_x + viewUp * offset_y/* + viewForward * offset_z*/;
		shadowEyePos -= viewRight * offset_x + viewUp * offset_y/* + viewForward * offset_z*/;

		RMatrix4 shadowViewMatrix = RMatrix4::CreateLookAtViewLH(shadowEyePos, shadowTarget, RVec3(0.0f, 1.0f, 0.0f));

		RFrustum shadowVolume = m_ShadowMap[i].GetFrustum();

		if (freezeFrustum)
			GDebugRenderer.DrawFrustum(shadowVolume, frustumColor[i]);

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

		//GDebugRenderer.DrawFrustum(shadowVolume);
	}

	// Update light constant buffer
	auto& cbLight = RConstantBuffers::cbLight.Data;
	RConstantBuffers::cbLight.ClearData();

	// Setup ambient color
	cbLight.HighHemisphereAmbientColor = RVec4(0.1f, 0.1f, 0.1f, 1.0f);
	cbLight.LowHemisphereAmbientColor = RVec4(0.2f, 0.2f, 0.2f, 1.0f);

	if (m_EnableLights[0])
	{
		RVec4 dirLightVec = RVec4(RVec3(0.25f, 1.0f, 0.5f).GetNormalized(), 1.0f);

		cbLight.DirectionalLightCount = 1;
		cbLight.DirectionalLight[0].Color = RVec4(1.0f, 1.0f, 0.8f, 1.0f);
		cbLight.DirectionalLight[0].Direction = RVec4(m_SunVec.GetNormalized(), 1.0f);
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
		RVec4 spotlightPos = RVec4(m_Camera->GetPosition(), 2000.0f);
		RVec4 spotlightDir = RVec4(m_Camera->GetForwardVector(), 1.0f);
		RVec4 spotlightRatio = RVec4(0.97f, 0.9f, 0.0f, 0.0f);
		cbLight.Spotlight[0].PosAndRadius = spotlightPos;
		cbLight.Spotlight[0].Direction = spotlightDir;
		cbLight.Spotlight[0].ConeRatio = spotlightRatio;
		cbLight.Spotlight[0].Color = RVec4(0.85f, 0.85f, 1.0f, 1.0f);
	}

	cbLight.CameraPos = m_Camera->GetPosition();
	cbLight.CascadedShadowCount = 3;

	float camNear = m_Camera->GetNearPlane(),
		  camFar = m_Camera->GetFarPlane();

	RVec4 sPoints[4] =
	{
		RVec4(0, 0, RMath::Lerp(camNear, camFar, shadowSplitPoints[0]), 1),
		RVec4(0, 0, RMath::Lerp(camNear, camFar, shadowSplitPoints[1]), 1),
		RVec4(0, 0, RMath::Lerp(camNear, camFar, shadowSplitPoints[2]), 1),
		RVec4(0, 0, RMath::Lerp(camNear, camFar, shadowSplitPoints[3]), 1),
	};

	for (int i = 1; i < 4; i++)
	{
		sPoints[i] = sPoints[i] * projMatrix;
		sPoints[i] /= sPoints[i].w;
		cbLight.CascadedShadowDepth[i - 1] = sPoints[i].z;
		//GDebugRenderer.DrawSphere(sPoints[i].ToVec3(), 50.0f);
	}

	RConstantBuffers::cbLight.UpdateBufferData();

	// Update instance buffer
	ZeroMemory(&cbInstance[0], sizeof(cbInstance[0]));

	for (int i = 0; i < MAX_INSTANCE_COUNT; i++)
	{
		float d = sinf((float)i / MAX_INSTANCE_COUNT * 2 * PI) + 2.0f;
		float x = sinf((float)i / MAX_INSTANCE_COUNT * 2 * PI) * 1000.0f * d;
		float y = sinf((float)i / MAX_INSTANCE_COUNT * 2 * PI * 8) * 1000.0f * d;
		float z = cosf((float)i / MAX_INSTANCE_COUNT * 2 * PI) * 1000.0f * d;
		RMatrix4 instanceMatrix = RMatrix4::CreateTranslation(x, y, z) * RMatrix4::CreateYAxisRotation((x + timer.TotalTime() * 0.1f * sinf(d)) * 180 / PI);

		m_InstanceMatrices[i] = instanceMatrix;
	}

	// Update screen information
	auto& cbScreen = RConstantBuffers::cbGlobal.Data;
	RConstantBuffers::cbGlobal.ClearData();

	cbScreen.ScreenSize = RVec4((float)GRenderer.GetClientWidth(), (float)GRenderer.GetClientHeight(),
								1.0f / (float)GRenderer.GetClientWidth(), 1.0f / (float)GRenderer.GetClientHeight());
	cbScreen.UseGammaCorrection = GRenderer.UsingGammaCorrection() ? 1 : (m_EnabledPostProcessor == 1);

	RConstantBuffers::cbGlobal.UpdateBufferData();

	// Update particle vertices
	ParticleDepthComparer cmp(m_Camera->GetPosition(), m_Camera->GetForwardVector());
	std::sort(m_ParticleVert, m_ParticleVert + PARTICLE_COUNT, cmp);
	m_ParticleBuffer.UpdateDynamicVertexBuffer(&m_ParticleVert, sizeof(RVertexType::Particle), PARTICLE_COUNT);

	auto& cbParticleInstance = m_cbInstance[1].Data;
	ObjectDepthComparer objCmp(m_Camera->GetPosition());
	std::sort(cbParticleInstance.instancedWorldMatrix, cbParticleInstance.instancedWorldMatrix + 125, objCmp);

	m_cbInstance[1].UpdateBufferData();

	RVec3 charPos = m_CharacterObj->GetTransformMatrix().GetTranslation();
	RMatrix4 charMatrix = RMatrix4::CreateYAxisRotation(m_CharacterRot);
	charMatrix.SetTranslation(charPos);
	m_CharacterObj->SetTransform(charMatrix);

	// Update animation
	RAnimation* animation = m_CharacterAnimation->IsLoaded() ? m_CharacterAnimation->GetAnimation() : nullptr;
	if (animation)
	{
		m_CharacterObj->GetMesh()->CacheAnimation(animation);

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

		RVec3 nodePos = m_CharacterObj->GetTransformMatrix().GetTranslation();
		RVec3 worldOffset = (RVec4(offset, 0.0f) * m_CharacterObj->GetTransformMatrix()).ToVec3();
		worldOffset.SetY(m_CharacterYVel);

		if (RInput.IsKeyDown(VK_UP))
			worldOffset += m_CharacterObj->GetForwardVector() * timer.DeltaTime() * 500.0f;

		if (RInput.IsKeyDown(VK_DOWN))
			worldOffset -= m_CharacterObj->GetForwardVector() * timer.DeltaTime() * 500.0f;

		RAabb aabb;
		aabb.Expand(RVec3(-50.0f, 0.0f, -50.0f));
		aabb.Expand(RVec3(50.0f, 200.0f, 50.0f));
		aabb.pMin += nodePos;
		aabb.pMax += nodePos;

		GDebugRenderer.DrawAabb(aabb);

		const std::vector<RMeshElement>& sceneMeshElements = m_SceneMeshCity->GetMeshElements();
		for (UINT i = 0; i < sceneMeshElements.size(); i++)
		{
			worldOffset = aabb.TestDynamicCollisionWithAabb(worldOffset, sceneMeshElements[i].GetAabb());
		}

		if (fabs(worldOffset.Y()) < fabs(m_CharacterYVel))
			m_CharacterYVel = 0.0f;

		m_CharacterObj->Translate(worldOffset, ETransformSpace::World);
		RVec3 invOffset = -animation->GetRootPosition(currTime);
		RMatrix4 rootInversedTranslation = RMatrix4::CreateTranslation(invOffset);

		auto cbSkinned = RConstantBuffers::cbBoneMatrices.Data;
		if (m_CharacterObj->GetMesh()->IsLoaded())
		{
			for (int i = 0; i < m_CharacterObj->GetMesh()->GetBoneCount(); i++)
			{
				RMatrix4 matrix;

				int boneId = m_CharacterObj->GetMesh()->GetCachedAnimationNodeId(animation, i);
				animation->GetNodePose(boneId, currTime, &matrix);

				cbSkinned.boneMatrix[i] = m_CharacterObj->GetMesh()->GetBoneInitInvMatrices(i) * matrix * rootInversedTranslation * m_CharacterObj->GetTransformMatrix();
			}
		}
		else
		{
			ZeroMemory(&cbSkinned, sizeof(cbSkinned));
		}

		GDebugRenderer.DrawLine(nodePos, nodePos + worldOffset * 10, RColor(1.0f, 0.0f, 0.0f));

		RConstantBuffers::cbBoneMatrices.UpdateBufferData();
		RConstantBuffers::cbBoneMatrices.BindBuffer();
	}

	RVec3 pos = m_TachikomaObj->GetTransformMatrix().GetTranslation();
	static float t = 0.0f;
	t += timer.DeltaTime() * 50.0f;
	m_TachikomaObj->SetTransform(RMatrix4::CreateYAxisRotation(t) * RMatrix4::CreateTranslation(pos));
	//GDebugRenderer.DrawAabb(m_TachikomaObj->GetAabb());


	if (RInput.GetBufferedKeyState(VK_OEM_PLUS) == EBufferedKeyState::Pressed)
	{
		m_MeshInstanceCount += 2;
	}

	if (RInput.GetBufferedKeyState(VK_OEM_MINUS) == EBufferedKeyState::Pressed)
	{
		m_MeshInstanceCount -= 2;
		if (m_MeshInstanceCount <= 0)
			m_MeshInstanceCount = 1;
	}
}

void FSGraphicsProjectApp::RenderScene()
{
	float width = static_cast<float>(GRenderer.GetClientWidth());
	float height = static_cast<float>(GRenderer.GetClientHeight());
	D3D11_VIEWPORT vp[2] =
	{
		{ 0.0f, 0.0f, width, height, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 300, 300, 0.0f, 1.0f },
	};

	RConstantBuffers::cbPerObject.BindBuffer();
	RConstantBuffers::cbLight.BindBuffer();
	RConstantBuffers::cbMaterial.BindBuffer();
	RConstantBuffers::cbGlobal.BindBuffer();
	GRenderer.SetSamplerState(0, SamplerState_Texture);
	GRenderer.SetSamplerState(2, SamplerState_ShadowDepthComparison);

	//=========================== Shadow Pass ===========================
	for (int i = 0; i < 3; i++)
	{
		auto& cbScene = RConstantBuffers::cbScene.Data;
		cbScene.cascadedShadowIndex = i;
		RConstantBuffers::cbScene.UpdateBufferData();
		RConstantBuffers::cbScene.BindBuffer();

		m_ShadowMap[i].SetupRenderTarget();
		GRenderer.Clear();
		RenderSinglePass(ShadowPass);
	}

	//=========================== Scene Buffer Pass =====================
	GRenderer.SetRenderTargets(1, &m_RenderTargetView, m_RenderTargetDepthView);

	GRenderer.D3DImmediateContext()->RSSetViewports(1, &vp[0]);
	GRenderer.Clear();
	RenderSinglePass(RefractionScenePass);

	//=========================== Normal Pass ===========================

	// 0 : Screen pass
	// 1 : Viewport pass
	for (int i = 0; i < 1; i++)
	{
		if (i == 0)
		{
			if (m_EnabledPostProcessor)
			{
				GPostProcessorManager.SetupRenderTarget();
			}
			else
			{
				GRenderer.SetRenderTargets();
			}
		}
		else
		{
			static int shadowIndex = 0;
			if (RInput.GetBufferedKeyState(VK_TAB) == EBufferedKeyState::Pressed)
			{
				shadowIndex++;
				shadowIndex %= 3;
			}

			RMatrix4 viewMatrix = m_ShadowMap[shadowIndex].GetViewMatrix();
			RMatrix4 projMatrix = m_ShadowMap[shadowIndex].GetProjectionMatrix();

			auto& cbScene = RConstantBuffers::cbScene.Data;
			cbScene.viewMatrix = viewMatrix;
			cbScene.projMatrix = projMatrix;
			cbScene.viewProjMatrix = viewMatrix * projMatrix;
			cbScene.cameraPos = m_SunVec;

			RConstantBuffers::cbScene.UpdateBufferData();
	
			auto& cbLight = RConstantBuffers::cbLight.Data;
			cbLight.CameraPos = m_SunVec;

			RConstantBuffers::cbLight.UpdateBufferData();

			RConstantBuffers::cbScene.BindBuffer();
			RConstantBuffers::cbLight.BindBuffer();

			ParticleDepthComparer cmp(m_SunVec, -m_SunVec);
			std::sort(m_ParticleVert, m_ParticleVert + PARTICLE_COUNT, cmp);
			m_ParticleBuffer.UpdateDynamicVertexBuffer(&m_ParticleVert, sizeof(RVertexType::Particle), PARTICLE_COUNT);
	
			GRenderer.SetRenderTargets();
		}

		GRenderer.D3DImmediateContext()->RSSetViewports(1, &vp[i]);

		if (i == 0)
			GRenderer.Clear();
		else
			GRenderer.Clear(false, RColor(0.0f, 0.0f, 0.0f));

		RenderSinglePass(NormalPass);

		if (i == 0 && m_EnabledPostProcessor)
		{
			GRenderer.SetRenderTargets();
			GRenderer.Clear();

			// Set back buffer as the input sampler of post processor
			ID3D11ShaderResourceView* RenderTargetSRV = GPostProcessorManager.GetRenderTargetSRV();
			GRenderer.D3DImmediateContext()->PSSetShaderResources(0, 1, &RenderTargetSRV);

			int EffectIndex = m_EnabledPostProcessor == 1 ? PPE_GammaCorrection : PPE_ColorEdgeDetection;
			GPostProcessorManager.Draw(m_PostProcessingEffects[EffectIndex]);
		}
	}

	GDebugRenderer.Reset();
}

void FSGraphicsProjectApp::OnResize(int width, int height)
{
	if (m_Camera)
	{
		m_Camera->SetAspectRatio((float)width / (float)height);
	}

	if (m_RenderTargetView)
	{
		SAFE_RELEASE(m_RenderTargetBuffer);
		SAFE_RELEASE(m_RenderTargetView);
		SAFE_RELEASE(m_RenderTargetSRV);
		SAFE_RELEASE(m_RenderTargetDepthBuffer);
		SAFE_RELEASE(m_RenderTargetDepthView);

		RResourceManager::Instance().UnloadSRVWrappers();

		CreateSceneRenderTargetView();

		// TODO: Fix the memory leak here
		RMaterial* Material = m_TachikomaObj->GetMaterial(0);
		Material->SetTextureSlot(0, RResourceManager::Instance().WrapShaderResourceViewInTexture(m_RenderTargetSRV));
	}
}

void FSGraphicsProjectApp::CreateSceneRenderTargetView()
{
	D3D11_TEXTURE2D_DESC renderTargetTextureDesc;
	renderTargetTextureDesc.Width = GRenderer.GetClientWidth();
	renderTargetTextureDesc.Height = GRenderer.GetClientHeight();
	renderTargetTextureDesc.MipLevels = 1;
	renderTargetTextureDesc.ArraySize = 1;
	renderTargetTextureDesc.Format = GRenderer.UsingGammaCorrection() ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
	renderTargetTextureDesc.SampleDesc.Count = 1;
	renderTargetTextureDesc.SampleDesc.Quality = 0;
	renderTargetTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	renderTargetTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	renderTargetTextureDesc.CPUAccessFlags = 0;
	renderTargetTextureDesc.MiscFlags = 0;

	GRenderer.D3DDevice()->CreateTexture2D(&renderTargetTextureDesc, 0, &m_RenderTargetBuffer);
	GRenderer.D3DDevice()->CreateRenderTargetView(m_RenderTargetBuffer, 0, &m_RenderTargetView);

	D3D11_SHADER_RESOURCE_VIEW_DESC rtsrvDesc;
	rtsrvDesc.Format = renderTargetTextureDesc.Format;
	rtsrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	rtsrvDesc.Texture2D.MostDetailedMip = 0;
	rtsrvDesc.Texture2D.MipLevels = 1;

	GRenderer.D3DDevice()->CreateShaderResourceView(m_RenderTargetBuffer, &rtsrvDesc, &m_RenderTargetSRV);

	renderTargetTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	renderTargetTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	GRenderer.D3DDevice()->CreateTexture2D(&renderTargetTextureDesc, 0, &m_RenderTargetDepthBuffer);
	GRenderer.D3DDevice()->CreateDepthStencilView(m_RenderTargetDepthBuffer, 0, &m_RenderTargetDepthView);
}

void FSGraphicsProjectApp::SetPerObjectConstBuffer(const RMatrix4& world)
{
	auto& cbObject = RConstantBuffers::cbPerObject.Data;
	cbObject.worldMatrix = world;

	RConstantBuffers::cbPerObject.UpdateBufferData();
}

void FSGraphicsProjectApp::RenderSinglePass(RenderPass pass)
{
	ID3D11ShaderResourceView* shadowMapSRV[] =
	{
		m_ShadowMap[0].GetRenderTargetDepthSRV(),
		m_ShadowMap[1].GetRenderTargetDepthSRV(),
		m_ShadowMap[2].GetRenderTargetDepthSRV(),
	};

	float timeNow = GEngine.GetTimer().TotalTime();
	float loadingFadeInTime = 1.0f;

	// Update material buffer
	auto& cbMaterial = RConstantBuffers::cbMaterial.Data;
	RConstantBuffers::cbMaterial.ClearData();

	cbMaterial.SpecularColorAndPower = m_MaterialSpecular;
	cbMaterial.GlobalOpacity = 1.0f;
	UpdateAndBindMaterialConstBuffer();

	GRenderer.SetBlendState(BlendState::Opaque);

	// Set up object world matrix
	SetPerObjectConstBuffer(RMatrix4::IDENTITY);

	if (pass != ShadowPass)
	{
		// Set shadow map to pixel shader
		GRenderer.D3DImmediateContext()->PSSetShaderResources(RShadowMap::ShaderResourceSlot(), 3, shadowMapSRV);

		// Draw skybox
		m_Skybox.Draw();

		// Clear depth buffer for skybox
		GRenderer.Clear(false, RColor(0, 0, 0));
	}

	auto& cbScene = RConstantBuffers::cbScene.Data;
	RFrustum cameraFrustum = (pass == ShadowPass) ? m_ShadowMap[cbScene.cascadedShadowIndex].GetFrustum() : m_Camera->GetFrustum();

#if 1
	// Draw star
	SetPerObjectConstBuffer(RMatrix4::CreateTranslation(0.0f, 500.0f, 0.0f));

	if (pass == ShadowPass)
		m_DepthShader->Bind();
	else
		m_ColorShader->Bind();

	m_StarMesh.Draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Draw meshes

	// Draw city
	const RAabb meshAabb = m_FbxMeshObj->GetMesh()->GetLocalSpaceAabb();

	int instanceCount = 0;
	auto& cbMeshInstance = m_cbInstance[2].Data;
	ZeroMemory(&cbMeshInstance, sizeof(cbMeshInstance));

	for (int x = -m_MeshInstanceCount / 2; x <= m_MeshInstanceCount / 2; x++)
	{
		for (int z = -m_MeshInstanceCount / 2; z <= m_MeshInstanceCount / 2; z++)
		{
			RMatrix4 mat = m_FbxMeshObj->GetTransformMatrix();
			mat.SetTranslation(RVec3(1700.0f * x, 0, 1700.0f * z));

			if (RCollision::TestAabbInsideFrustum(cameraFrustum, meshAabb.GetTransformedAabb(mat)))
			{
				cbMeshInstance.instancedWorldMatrix[instanceCount] = mat;
				instanceCount++;
			}

			if (instanceCount >= MAX_INSTANCE_COUNT ||
				((x == m_MeshInstanceCount / 2) && (z == m_MeshInstanceCount / 2)))
			{
				m_cbInstance[2].UpdateBufferData();
				m_cbInstance[2].BindBuffer();

				if (pass == ShadowPass)
					m_FbxMeshObj->DrawWithShader(m_DepthShader, true, instanceCount);
				else
				{
					float opacity = (timeNow - m_FbxMeshObj->GetResourceTimestamp()) / loadingFadeInTime;
					if (opacity >= 0.0f && opacity <= 1.0f)
					{
						cbMaterial.GlobalOpacity = opacity;
						UpdateAndBindMaterialConstBuffer();
						GRenderer.SetBlendState(BlendState::AlphaToCoverage);
					}
					else
					{
						GRenderer.SetBlendState(BlendState::Opaque);
					}

					m_FbxMeshObj->Draw(true, instanceCount);
				}

				instanceCount = 0;
			}
		}
	}

#endif

#if 1
	// Draw islands
	SetPerObjectConstBuffer(m_IslandMeshObj->GetTransformMatrix());

	auto& cbIslandInstance = m_cbInstance[0].Data;
	int islandInstanceCount = 0;

	if (m_IslandMeshObj->GetMesh()->IsLoaded())
	{
		RAabb islandAabb = m_IslandMeshObj->GetMesh()->GetLocalSpaceAabb();
		for (int i = 0; i < MAX_INSTANCE_COUNT; i++)
		{
			if (!RCollision::TestAabbInsideFrustum(cameraFrustum, islandAabb.GetTransformedAabb(m_InstanceMatrices[i])))
				continue;

			cbIslandInstance.instancedWorldMatrix[islandInstanceCount] = m_InstanceMatrices[i];
			islandInstanceCount++;
		}
		m_cbInstance[0].UpdateBufferData();
		m_cbInstance[0].BindBuffer();
	}

	if (pass == ShadowPass)
		m_IslandMeshObj->DrawWithShader(m_DepthShader, true, islandInstanceCount);
	else
	{
		float opacity = (timeNow - m_IslandMeshObj->GetResourceTimestamp()) / loadingFadeInTime;
		if (opacity >= 0.0f && opacity <= 1.0f)
		{
			cbMaterial.GlobalOpacity = opacity;
			UpdateAndBindMaterialConstBuffer();
			GRenderer.SetBlendState(BlendState::AlphaToCoverage);
		}
		else
		{
			GRenderer.SetBlendState(BlendState::Opaque);
		}
		m_IslandMeshObj->Draw(true, islandInstanceCount);
	}

	// Draw AO scene
	SetPerObjectConstBuffer(m_AOSceneObj->GetTransformMatrix());

	if (pass == ShadowPass)
		m_AOSceneObj->DrawWithShader(m_DepthShader);
	else
	{
		float opacity = (timeNow - m_AOSceneObj->GetResourceTimestamp()) / loadingFadeInTime;
		if (opacity >= 0.0f && opacity <= 1.0f)
		{
			cbMaterial.GlobalOpacity = opacity;
			UpdateAndBindMaterialConstBuffer();
			GRenderer.SetBlendState(BlendState::AlphaToCoverage);
			m_AOSceneObj->Draw();
		}
		else
		{
			GRenderer.SetBlendState(BlendState::Opaque);
			m_AOSceneObj->Draw();
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
		UpdateAndBindMaterialConstBuffer();
		GRenderer.SetBlendState(BlendState::AlphaToCoverage);
		m_BumpLightingShader->Bind();
		GRenderer.D3DImmediateContext()->PSSetShaderResources(0, 1, m_BumpBaseTexture->GetPtrSRV());
		GRenderer.D3DImmediateContext()->PSSetShaderResources(1, 1, m_BumpNormalTexture->GetPtrSRV());
		m_BumpCubeMesh.Draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
#endif

	// Draw character
	SetPerObjectConstBuffer(m_CharacterObj->GetTransformMatrix());
	if (RCollision::TestAabbInsideFrustum(cameraFrustum, m_CharacterObj->GetAabb()))
	{
		if (pass == ShadowPass)
			m_CharacterObj->DrawDepthPass();
		else
		{
			float opacity = (timeNow - m_CharacterObj->GetResourceTimestamp()) / loadingFadeInTime;
			if (opacity >= 0.0f && opacity <= 1.0f)
			{
				cbMaterial.GlobalOpacity = opacity;
				UpdateAndBindMaterialConstBuffer();
				GRenderer.SetBlendState(BlendState::AlphaToCoverage);
				m_CharacterObj->Draw();
			}
			else
			{
				cbMaterial.GlobalOpacity = 1.0f;
				UpdateAndBindMaterialConstBuffer();
				GRenderer.SetBlendState(BlendState::AlphaBlending);
				m_CharacterObj->Draw();
			}
		}
	}

#if 1
	// Draw transparent spheres
	if (pass != ShadowPass)
	{
		m_cbInstance[1].BindBuffer();

		SetPerObjectConstBuffer(m_TransparentMesh->GetTransformMatrix());

		float opacity = (timeNow - m_TransparentMesh->GetResourceTimestamp()) / loadingFadeInTime;
		if (opacity >= 0.0f && opacity <= 1.0f)
		{
			cbMaterial.GlobalOpacity = opacity * 0.25f;
			UpdateAndBindMaterialConstBuffer();
			GRenderer.SetBlendState(BlendState::AlphaToCoverage);
			m_TransparentMesh->Draw(true, 125);
		}
		else
		{
			cbMaterial.GlobalOpacity = 0.25f;
			UpdateAndBindMaterialConstBuffer();
			GRenderer.SetBlendState(BlendState::AlphaBlending);
			m_TransparentMesh->Draw(true, 125);
		}
	}

	// Draw tachikoma
	SetPerObjectConstBuffer(m_TachikomaObj->GetTransformMatrix());

	if (RCollision::TestAabbInsideFrustum(cameraFrustum, m_TachikomaObj->GetAabb()))
	{
		if (pass == ShadowPass)
			m_TachikomaObj->DrawWithShader(m_DepthShader);
		else if (pass == NormalPass)
		{
			float opacity = (timeNow - m_TachikomaObj->GetResourceTimestamp()) / loadingFadeInTime;
			if (opacity >= 0.0f && opacity <= 1.0f)
			{
				cbMaterial.GlobalOpacity = opacity;
				UpdateAndBindMaterialConstBuffer();
				GRenderer.SetBlendState(BlendState::AlphaToCoverage);
				m_TachikomaObj->Draw();
			}
			else
			{
				GRenderer.SetBlendState(BlendState::Opaque);
				m_TachikomaObj->Draw();
			}
		}
	}
#endif

	if (pass != ShadowPass)
	{
#if 1
		if (RCollision::TestAabbInsideFrustum(cameraFrustum, m_ParticleAabb))
		{
			// Draw particles
			GRenderer.SetBlendState(BlendState::AlphaBlending);
			GRenderer.D3DImmediateContext()->OMSetDepthStencilState(m_DepthState[1], 0);

			SetPerObjectConstBuffer(RMatrix4::CreateTranslation(0.0f, 150.0f, 150.0f));
			GRenderer.D3DImmediateContext()->PSSetShaderResources(0, 1, m_ParticleDiffuseTexture->GetPtrSRV());
			GRenderer.D3DImmediateContext()->PSSetShaderResources(1, 1, m_ParticleNormalTexture->GetPtrSRV());
			m_ParticleShader->Bind();
			m_ParticleBuffer.Draw(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

			// Restore depth writing
			GRenderer.D3DImmediateContext()->OMSetDepthStencilState(m_DepthState[0], 0);
		}
#endif
		// Draw debug lines
		GRenderer.SetBlendState(BlendState::Opaque);
		GDebugRenderer.Render();
	}

	// Unbind all shader resources so we can write into it
	ID3D11ShaderResourceView* nullSRV[8] = { nullptr };
	GRenderer.D3DImmediateContext()->PSSetShaderResources(0, 8, nullSRV);
}

void FSGraphicsProjectApp::UpdateAndBindMaterialConstBuffer()
{
	RConstantBuffers::cbMaterial.UpdateBufferData();
	RConstantBuffers::cbMaterial.BindBuffer();
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

	//GDebugRenderer.DrawSphere(center, 50.0f, RColor(1, 0, 0));

	//for (int i = 4; i < 8; i++)
	//{
	//	GDebugRenderer.DrawSphere(cornerPoints[i], 50.0f, RColor(1, 0, 0));
	//}

	return s;
}
