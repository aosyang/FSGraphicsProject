//=============================================================================
// FightingGameApp.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "FightingGameApp.h"

#include "AIBehavior_Fighter.h"
#include "AIBehavior_Roamer.h"
#include "Navigation/RNavigationSystem.h"

FightingGameApp::FightingGameApp()
	: CurrentPlayerIndex(0)
	, m_Camera(nullptr)
	, m_FreeFlyMode(false)
	, m_FreeFlyCameraControl(nullptr)
{
	m_Player[0] = nullptr;
	m_Player[1] = nullptr;
}

FightingGameApp::~FightingGameApp()
{
}

bool FightingGameApp::Initialize()
{
	srand((unsigned int)time(nullptr));

	//RResourceManager::Instance().LoadAllResources();

	RScene* DefaultScene = GSceneManager.DefaultScene();

	DefaultScene->Initialize();
	DefaultScene->LoadFromFile("/TestArena.rmap");
	
	GNavigationSystem.BuildNavMesh(DefaultScene);

	m_ShadowMap.Initialize(1024, 1024);
	m_Camera = DefaultScene->CreateSceneObjectOfType<RCamera>();
	m_Camera->SetTransform(RVec3(407.023712f, 339.007507f, 876.396484f), RQuat::Euler(0.09f, 3.88659930f, 0.0f));
	m_Camera->SetupView(45.0f, GRenderer.AspectRatio(), 1.0f, 10000.0f);
	m_FreeFlyCameraControl = m_Camera->AddNewComponent<RFreeFlyCameraControl>();

	//RSceneObject* player = DefaultScene->FindObject("Player");
	//for (UINT i = 0; i < DefaultScene->GetSceneObjects().size(); i++)
	//{
	//	RSceneObject* obj = DefaultScene->GetSceneObjects()[i];
	//	obj->SetScript("UpdateObject");
	//}

	// Add static colliders to scene objects
	auto SceneObjects = DefaultScene->EnumerateSceneObjects();
	for (auto& SceneObject : SceneObjects)
	{
		if (SceneObject->CanCastTo<RSMeshObject>())
		{
			RRigidBodyComponent* RigidBody = SceneObject->AddNewComponent<RRigidBodyComponent>();
			RigidBody->SetMovable(false);
		}
	}

	for (int i = 0; i < MaxNumPlayers; i++)
	{
		m_Player[i] = DefaultScene->CreateSceneObjectOfType<PlayerControllerBase>();
		if (m_Player[i])
		{
			FTGPlayerStateMachine& StateMachine = m_Player[i]->GetStateMachine();
			StateMachine.AllocateBehaviorInstance<FTGPlayerBehavior_Idle>("/HumanBody/Animations/Idle.fbx", AnimBitFlag_Loop);
			StateMachine.AllocateBehaviorInstance<FTGPlayerBehavior_Run>("/HumanBody/Animations/Running.fbx", AnimBitFlag_Loop);

			m_Player[i]->InitAssets("/HumanBody/HumanBody_DefaultPose.fbx");

			// Hacked start points for players
			{
				if (i == 0)
				{
					m_Player[i]->SetPosition(RVec3(-1795, 200, -200));
				}

				if (i == 1)
				{
					m_Player[i]->SetPosition(RVec3(1835, 40, -853));
				}
			}
		}
	}

	m_AIPlayers.resize(MaxNumAIs);
	for (int i = 0; i < MaxNumAIs; i++)
	{
		m_AIPlayers[i] = DefaultScene->CreateSceneObjectOfType<FTGPlayerController>();
		if (m_AIPlayers[i])
		{
			m_AIPlayers[i]->SetPosition(RVec3(RMath::RandRangedF(-800, 800), 50, RMath::RandRangedF(-800, 800)));

			FTGPlayerStateMachine& StateMachine = m_AIPlayers[i]->GetStateMachine();
			StateMachine.AllocateBehaviorInstance<FTGPlayerBehavior_Idle>("/unitychan/FUCM05_0000_Idle.fbx", AnimBitFlag_Loop);
			StateMachine.AllocateBehaviorInstance<FTGPlayerBehavior_Run>("/unitychan/FUCM_0012b_EH_RUN_LP_NoZ.fbx", AnimBitFlag_Loop);
			StateMachine.AllocateBehaviorInstance<FTGPlayerBehavior_Punch>("/unitychan/FUCM05_0001_M_CMN_LJAB.fbx", AnimBitFlag_HasRootMotion);
			StateMachine.AllocateBehaviorInstance<FTGPlayerBehavior_Kick>("/unitychan/FUCM_04_0001_RHiKick.fbx", AnimBitFlag_HasRootMotion);
			StateMachine.AllocateBehaviorInstance<FTGPlayerBehavior_BackKick>("/unitychan/FUCM02_0004_CH01_AS_MAWAK.fbx", AnimBitFlag_HasRootMotion);
			StateMachine.AllocateBehaviorInstance<FTGPlayerBehavior_SpinAttack>("/unitychan/FUCM02_0029_Cha01_STL01_ScrewK01.fbx", AnimBitFlag_HasRootMotion);
			StateMachine.AllocateBehaviorInstance<FTGPlayerBehavior_Hit>("/unitychan/unitychan_DAMAGED00.fbx", AnimBitFlag_HasRootMotion);
			StateMachine.AllocateBehaviorInstance<FTGPlayerBehavior_KnockedDown>("/unitychan/FUCM02_0025_MYA_TF_DOWN.fbx", AnimBitFlag_HasRootMotion);
			StateMachine.AllocateBehaviorInstance<FTGPlayerBehavior_GetUp>("/unitychan/FUCM03_0019_HeadSpring.fbx", AnimBitFlag_HasRootMotion);

			m_AIPlayers[i]->InitAssets("/unitychan/unitychan.fbx");

			// Make each AI play animation at a slightly different speed
			//m_AIPlayers[i]->SetAnimationDeviation(RMath::RandRangedF(0.8f, 1.2f));
			m_AIPlayers[i]->SetAnimationDeviation(0.8f);

			// Create AI combat logic component
			m_AIPlayers[i]->AddNewComponent<AIBehavior_Roamer>();
		}
	}

	m_Text.Initialize(RResourceManager::Instance().LoadResource<RTexture>("/Fonts/Fixedsys_9c.DDS", EResourceLoadMode::Immediate), 16, 16);
	return true;
}

void FightingGameApp::UpdateScene(const RTimer& timer)
{
	if (RInput.GetBufferedKeyState('P') == EBufferedKeyState::Pressed)
	{
		FTGPlayerController::DrawDebugHitShape = !FTGPlayerController::DrawDebugHitShape;
	}

	if (RInput.GetBufferedKeyState('R') == EBufferedKeyState::Pressed)
	{
		// Reset all characters' position
		for (int i = 0; i < MaxNumPlayers; i++)
		{
			if (m_Player[i])
			{
				m_Player[i]->SetPosition(RVec3(0, 100.0f, 0));
			}
		}

		for (int i = 0; i < MaxNumAIs; i++)
		{
			if (m_AIPlayers[i])
			{
				ResetPlayerPosition(m_AIPlayers[i]);
			}
		}
	}

	// Select current controlled player with number key '1' and '2'
	{
		if (RInput.GetBufferedKeyState('1') == EBufferedKeyState::Pressed && MaxNumPlayers >= 1)
		{
			CurrentPlayerIndex = 0;
		}

		if (RInput.GetBufferedKeyState('2') == EBufferedKeyState::Pressed && MaxNumPlayers >= 2)
		{
			CurrentPlayerIndex = 1;
		}
	}

	if (RInput.GetBufferedKeyState('X') == EBufferedKeyState::Pressed)
	{
		CreatePhysicsBoxes();
	}

	UpdateUserInput();

	// Update all player controllers
	for (auto SceneObject : FTGPlayerController::ActivePlayerControllers)
	{
		FTGPlayerController* PlayerController = static_cast<FTGPlayerController*>(SceneObject);
		PlayerController->UpdateController(timer.DeltaTime());

		// Kill plane
		if (PlayerController->GetWorldPosition().Y() < -2000.0f)
		{
			ResetPlayerPosition(PlayerController);
		}
	}

	if (auto* CurrentPlayer = GetCurrentPlayer())
	{
		//playerAabb.pMin = RVec3(-50.0f, 0.0f, -50.0f) + m_Player->GetPosition();
		//playerAabb.pMax = RVec3(50.0f, 150.0f, 50.0f) + m_Player->GetPosition();
		//GDebugRenderer.DrawAabb(playerAabb);

		//GDebugRenderer.DrawFrustum(m_Camera->GetFrustum());

		//RSphere s = { RVec3(0, 100, 0), 50.0f };
		//RCapsule cap = m_Player->GetCollisionShape();
		//RColor color = RColor(0, 1, 1);
		//if (RCollision::TestSphereWithCapsule(s, cap))
		//{
		//	color = RColor(1, 0, 0);
		//}
		//GDebugRenderer.DrawSphere(s.center, s.radius, color);
		//GDebugRenderer.DrawSphere(cap.start, cap.radius, color);
		//GDebugRenderer.DrawSphere(cap.end, cap.radius, color);

		char msg_buf[1024];
		RVec3 playerPos = CurrentPlayer->GetPosition();
		float playerRot = CurrentPlayer->GetPlayerRotation();
		RAnimationBlender& blender = CurrentPlayer->GetAnimBlender();
		sprintf_s(msg_buf, 1024, "Player: (%f, %f, %f), rot: %f\n"
			"Blend From : %s\n"
			"Blend To   : %s\n"
			"Blend time : %f\n"
			"%s",
			playerPos.X(), playerPos.Y(), playerPos.Z(), playerRot,
			blender.GetSourceAnimation() ? blender.GetSourceAnimation()->GetName().c_str() : "",
			blender.GetTargetAnimation() ? blender.GetTargetAnimation()->GetName().c_str() : "",
			blender.GetElapsedBlendTime(),
			FTGPlayerController::DrawDebugHitShape ? "Draw debug hit shape" : "");
		m_Text.SetText(msg_buf, RColor(0, 1, 0));
	}

	UpdateCameraPosition(timer.DeltaTime());

	//GNavigationSystem.DebugRender();
	if (MaxNumPlayers >= 2)
	{
		GNavigationSystem.DebugSetPathQueryPoints(m_Player[0]->GetPosition(), m_Player[1]->GetPosition());
	}
}

void FightingGameApp::RenderScene()
{
	// Update light constant buffer
	auto& cbLight = RConstantBuffers::cbLight.Data;
	RConstantBuffers::cbLight.ClearData();

	// Setup ambient color
	cbLight.HighHemisphereAmbientColor = RVec4(1.0f, 1.0f, 1.0f, 0.4f);
	cbLight.LowHemisphereAmbientColor = RVec4(0.2f, 0.2f, 0.2f, 1.0f);

	RVec4 dirLightVec = RVec4(RVec3(0.25f, 1.0f, 0.5f).GetNormalized(), 1.0f);

	RVec3 sunVec = RVec3(sinf(1.0f) * 0.5f, 0.25f, cosf(1.0) * 0.5f).GetNormalized() * 2000.0f;
	RMatrix4 shadowViewMatrix = RMatrix4::CreateLookAtViewLH(sunVec, RVec3(0.0f, 0.0f, 0.0f), RVec3(0.0f, 1.0f, 0.0f));

	cbLight.DirectionalLightCount = 1;
	cbLight.DirectionalLight[0].Color = RVec4(1.0f, 1.0f, 0.8f, 2.0f);
	cbLight.DirectionalLight[0].Direction = RVec4(sunVec.GetNormalized(), 1.0f);

	cbLight.CascadedShadowCount = 1;


	// Update scene constant buffer
	auto& cbScene = RConstantBuffers::cbScene.Data;
	RConstantBuffers::cbScene.ClearData();

#if 1
	cbScene.viewMatrix = m_Camera->GetViewMatrix();
	cbScene.projMatrix = m_Camera->GetProjectionMatrix();
	cbScene.viewProjMatrix = cbScene.viewMatrix * cbScene.projMatrix;
	cbScene.cameraPos = m_Camera->GetPosition();

	cbLight.CameraPos = m_Camera->GetPosition();
#else
	RMatrix4 cameraMatrix = RMatrix4::CreateXAxisRotation(0.09f * 180 / PI) * RMatrix4::CreateYAxisRotation(3.88659930f * 180 / PI);
	cameraMatrix.SetTranslation(RVec3(407.023712f, 339.007507f, 876.396484f));

	cbScene.viewMatrix = RMatrix4::CreateLookAtViewLH(RVec3(407.023712f, 339.007507f, 876.396484f), m_Player->GetPosition(), RVec3(0, 1, 0));
	cbScene.projMatrix = RMatrix4::CreatePerspectiveProjectionLH(65.0f, GRenderer.AspectRatio(), 1.0f, 10000.0f);
	cbScene.viewProjMatrix = cbScene.viewMatrix * cbScene.projMatrix;
	cbScene.cameraPos = cameraMatrix.GetTranslation();

	cbLight.CameraPos = cameraMatrix.GetTranslation();
#endif

	m_ShadowMap.SetViewMatrix(shadowViewMatrix);
	m_ShadowMap.SetOrthogonalProjection(5000.0f, 5000.0f, 0.1f, 5000.0f);

	RMatrix4 shadowTransform(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	RMatrix4 shadowViewProjMatrix = m_ShadowMap.GetViewMatrix() * m_ShadowMap.GetProjectionMatrix();
	cbScene.shadowViewProjMatrix[0] = shadowViewProjMatrix;
	shadowViewProjMatrix *= shadowTransform;
	cbScene.shadowViewProjBiasedMatrix[0] = shadowViewProjMatrix;

	RConstantBuffers::cbScene.UpdateBufferData();
	RConstantBuffers::cbScene.BindBuffer();

	RConstantBuffers::cbLight.UpdateBufferData();
	RConstantBuffers::cbLight.BindBuffer();

	auto& cbMaterial = RConstantBuffers::cbMaterial.Data;
	RConstantBuffers::cbMaterial.ClearData();

	cbMaterial.SpecularColorAndPower = RVec4(1.0f, 1.0f, 1.0f, 512.0f);
	cbMaterial.GlobalOpacity = 1.0f;

	RConstantBuffers::cbMaterial.UpdateBufferData();
	RConstantBuffers::cbMaterial.BindBuffer();

	auto& cbScreen = RConstantBuffers::cbGlobal.Data;
	RConstantBuffers::cbGlobal.ClearData();

	cbScreen.ScreenSize = RVec4((float)GRenderer.GetClientWidth(), (float)GRenderer.GetClientHeight(),
		1.0f / (float)GRenderer.GetClientWidth(), 1.0f / (float)GRenderer.GetClientHeight());
	cbScreen.UseGammaCorrection = GRenderer.UsingGammaCorrection();

	RConstantBuffers::cbGlobal.UpdateBufferData();
	RConstantBuffers::cbGlobal.BindBuffer();


	GRenderer.SetSamplerState(0, SamplerState_Texture);
	GRenderer.SetSamplerState(2, SamplerState_ShadowDepthComparison);

	float width = static_cast<float>(GRenderer.GetClientWidth());
	float height = static_cast<float>(GRenderer.GetClientHeight());
	D3D11_VIEWPORT vp = { 0.0f, 0.0f, width, height, 0.0f, 1.0f };


	auto& cbObject = RConstantBuffers::cbPerObject.Data;
	RScene* DefaultScene = GSceneManager.DefaultScene();

	for (int pass = 0; pass < 2; pass++)
	{
		if (pass == 0)
		{
			ID3D11ShaderResourceView* nullSRV[] = { nullptr };
			GRenderer.D3DImmediateContext()->PSSetShaderResources(RShadowMap::ShaderResourceSlot(), 1, nullSRV);
			
			m_ShadowMap.SetupRenderTarget();
		}
		else
		{
			GRenderer.SetRenderTargets();
			GRenderer.D3DImmediateContext()->RSSetViewports(1, &vp);

			ID3D11ShaderResourceView* shadowMapSRV[] = { m_ShadowMap.GetRenderTargetDepthSRV() };
			GRenderer.D3DImmediateContext()->PSSetShaderResources(RShadowMap::ShaderResourceSlot(), 1, shadowMapSRV);
		}

		GRenderer.Clear();

		if (pass == 0)
		{
			RFrustum frustum = m_ShadowMap.GetFrustum();
			DefaultScene->RenderDepthPass(&frustum);
		}
		else
		{
			RFrustum frustum = m_Camera->GetFrustum();
			DefaultScene->Render(&frustum);
		}
	}

	GDebugRenderer.Render();
	GDebugRenderer.Reset();

	GRenderer.Clear(false);
	m_Text.Render();

	GRenderer.Present();
}

void FightingGameApp::OnResize(int width, int height)
{
	if (m_Camera)
	{
		m_Camera->SetAspectRatio((float)width / (float)height);
	}
}

void FightingGameApp::UpdateUserInput()
{
	if (RInput.GetBufferedKeyState(VK_TAB) == EBufferedKeyState::Pressed)
	{
		m_FreeFlyMode = !m_FreeFlyMode;
		if (m_FreeFlyCameraControl)
		{
			m_FreeFlyCameraControl->SetEnabled(m_FreeFlyMode);
		}
	}

	if (!m_FreeFlyMode || !m_FreeFlyCameraControl)
	{
		if (auto* CurrentPlayer = GetCurrentPlayer())
		{
			RVec3 moveVec(0, 0, 0);

			RVec3 charRight = m_Camera->GetRightVector();
			RVec3 charForward = RVec3::Cross(charRight, RVec3(0, 1, 0));

			if (RInput.IsKeyDown('W')) moveVec += charForward;
			if (RInput.IsKeyDown('S')) moveVec -= charForward;
			if (RInput.IsKeyDown('A')) moveVec -= charRight;
			if (RInput.IsKeyDown('D')) moveVec += charRight;

			if (moveVec.SquaredMagitude() > 0.0f)
			{
				moveVec.Normalize();
				moveVec *= 600.0f;
			}

			CurrentPlayer->SetMovementInput(moveVec);

			FTGPlayerController* FightingPlayer = CurrentPlayer->CastTo<FTGPlayerController>();
			if (FightingPlayer)
			{
				if (RInput.GetBufferedKeyState(VK_SPACE) == EBufferedKeyState::Pressed)
				{
					FightingPlayer->PerformSpinAttack();
				}

				if (RInput.GetBufferedKeyState('J') == EBufferedKeyState::Pressed)
				{
					FightingPlayer->PerformPunch();
				}

				if (RInput.GetBufferedKeyState('K') == EBufferedKeyState::Pressed)
				{
					FightingPlayer->PerformKick();
				}

				if (RInput.GetBufferedKeyState('C') == EBufferedKeyState::Pressed)
				{
					FightingPlayer->SetBehavior(BHV_KnockedDown);
				}
			}
		}
	}
}

void FightingGameApp::UpdateCameraPosition(float DeltaTime)
{
	if (m_FreeFlyMode)
	{
		return;
	}

	auto* CurrentPlayer = GetCurrentPlayer();
	RVec3 PlayerPosition = CurrentPlayer ? CurrentPlayer->GetPosition() : RVec3(0, 0, 0);

	RMatrix4 playerTranslation = RMatrix4::CreateTranslation(PlayerPosition);

	// Make camera's world transform from a local transform
	static const RVec3 CameraOffset = RVec3(0.0f, 5.0f, 3.0f) * 175.0f;
	RMatrix4 cameraTransform = RMatrix4::CreateTranslation(CameraOffset) * playerTranslation;

	// Player mesh has its origin at feet. We offset the look target up by half of player's height.
	RVec3 lookTarget = PlayerPosition + RVec3(0, 125, 0);

	// Camera bounding box used for level collision
	RAabb camAabb;
	camAabb.pMin = RVec3(-5, -5, -5) + lookTarget;
	camAabb.pMax = RVec3(5, 5, 5) + lookTarget;

	RVec3 camVec = cameraTransform.GetTranslation() - lookTarget;
	RScene* DefaultScene = GSceneManager.DefaultScene();

	camVec = DefaultScene->TestMovingAabbWithScene(camAabb, camVec, FTGPlayerController::ActivePlayerControllers);

	//m_Camera->SetTransform(cameraTransform);
	static RVec3 actualCamVec = m_Camera->GetWorldPosition();
	actualCamVec = RVec3::Lerp(actualCamVec, camVec, 5.0f * DeltaTime);
	m_Camera->SetPosition(actualCamVec + lookTarget);
	m_Camera->LookAt(lookTarget);
}

void FightingGameApp::CreatePhysicsBoxes()
{
	static std::vector<RSMeshObject*> BoxList;

	for (auto Iter : BoxList)
	{
		Iter->Destroy();
	}
	BoxList.clear();

	// Create physics boxes
	{
		RMesh* CubeMesh = RResourceManager::Instance().LoadResource<RMesh>("/cube.fbx", EResourceLoadMode::Immediate);
		RScene* DefaultScene = GSceneManager.DefaultScene();

		for (int x = -10; x <= 10; x += 2)
		{
			for (int z = -10; z <= 10; z += 2)
			{
				RSMeshObject* BoxObject = DefaultScene->CreateMeshObject(CubeMesh);
				BoxObject->SetPosition(RVec3(200.0f * x, 1000, 200.0f * z));

				float scale_x = RMath::RandRangedF(0.5f, 4.0f);
				float scale_y = RMath::RandRangedF(0.5f, 4.0f);
				float scale_z = RMath::RandRangedF(0.5f, 4.0f);

				BoxObject->SetScale(RVec3(scale_x, scale_y, scale_x));
				BoxObject->SetRotation(RQuat::Euler(0.0f, RMath::RadianToDegree(45.0f), 0.0f));
				BoxObject->AddNewComponent<RRigidBodyComponent>();

				BoxList.push_back(BoxObject);
			}
		}
	}
}

void FightingGameApp::ResetPlayerPosition(FTGPlayerController* PlayerController)
{
	PlayerController->SetPosition(RVec3(RMath::RandRangedF(-800, 800), 50, RMath::RandRangedF(-800, 800)));
	PlayerController->Reset();
}

PlayerControllerBase* FightingGameApp::GetCurrentPlayer() const
{
	assert(CurrentPlayerIndex >= 0 && CurrentPlayerIndex < MaxNumPlayers);
	return m_Player[CurrentPlayerIndex];
}
