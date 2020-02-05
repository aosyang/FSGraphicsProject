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
	const std::string MapPath("/TestArena.rmap");

	DefaultScene->Initialize();
	DefaultScene->LoadFromFile(MapPath);

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

	bool bNavMeshLoaded = false;
	std::string NavMeshDataPath = RFileUtil::CombinePath(RResourceManager::GetAssetsBasePath(), MapPath);
	NavMeshDataPath = RFileUtil::ReplaceExtension(NavMeshDataPath, "navmesh");

	// Load navmesh data from file
	RSerializer NavMeshSerializer;
	NavMeshSerializer.Open(NavMeshDataPath, ESerializeMode::Read);
	if (NavMeshSerializer.IsOpen())
	{
		bNavMeshLoaded = GNavigationSystem.SerializeNavMesh(NavMeshSerializer);
		NavMeshSerializer.Close();
	}
	
	if (!bNavMeshLoaded)
	{
		// Failed to load navmesh data from file. Rebuild the navmesh now
		GNavigationSystem.BuildNavMesh(DefaultScene, RPhysicsNavMeshCellDetector());

		// Save the built data to file
		NavMeshSerializer.Open(NavMeshDataPath, ESerializeMode::Write);
		if (NavMeshSerializer.IsOpen())
		{
			GNavigationSystem.SerializeNavMesh(NavMeshSerializer);
			NavMeshSerializer.Close();
		}
	}

	RSceneObject* GlobalLightInfo = DefaultScene->CreateSceneObjectOfType<RSceneObject>("DirectionalLight", CF_NoSerialization);
	RDirectionalLightComponent* DirLightComponent = GlobalLightInfo->AddNewComponent<RDirectionalLightComponent>();
	DirLightComponent->SetParameters({ RVec3(sinf(1.0f) * 0.5f, 0.25f, cosf(1.0) * 0.5f), RColor(1.0f, 1.0f, 0.8f, 1.0f) });

	m_Camera = DefaultScene->CreateSceneObjectOfType<RCamera>();
	m_Camera->SetTransform(RVec3(407.023712f, 339.007507f, 876.396484f), RQuat::Euler(0.09f, 3.88659930f, 0.0f));
	m_Camera->SetupView(45.0f, GRenderer.AspectRatio(), 1.0f, 10000.0f);
	m_FreeFlyCameraControl = m_Camera->AddNewComponent<RFreeFlyCameraControl>();
	m_FreeFlyCameraControl->SetEnabled(false);

	//RSceneObject* player = DefaultScene->FindObject("Player");
	//for (UINT i = 0; i < DefaultScene->GetSceneObjects().size(); i++)
	//{
	//	RSceneObject* obj = DefaultScene->GetSceneObjects()[i];
	//	obj->SetScript("UpdateObject");
	//}

	for (int i = 0; i < MaxNumPlayers; i++)
	{
		m_Player[i] = DefaultScene->CreateSceneObjectOfType<PlayerControllerBase>();
		if (m_Player[i])
		{
			FTGPlayerStateMachine& StateMachine = m_Player[i]->GetStateMachine();
			StateMachine.AllocateBehaviorInstance<FTGPlayerBehavior_Idle>("/HumanBody/Animations/Idle.fbx", AnimBitFlag_Loop);
			StateMachine.AllocateBehaviorInstance<FTGPlayerBehavior_Walk>("/HumanBody/Animations/Walking.fbx", AnimBitFlag_Loop);
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

		RVec3 playerPos = CurrentPlayer->GetPosition();
		float playerRot = CurrentPlayer->GetPlayerRotation();
		RAnimationBlender& blender = CurrentPlayer->GetAnimBlender();
		float SourcePlayback = blender.GetSourcePlaybackTime();

		std::stringstream DebugMsg;
		DebugMsg << "Player: ("
				 << playerPos.X() << ", "
				 << playerPos.Y() << ", "
				 << playerPos.Z() << "), rot: "
				 << playerRot << std::endl;

		DebugMsg << "Source animation : ";
		if (blender.GetSourceAnimation())
		{
			DebugMsg << blender.GetSourceAnimation()->GetName();
		}
		DebugMsg << std::endl;

		DebugMsg << "Source start time : " << blender.GetSourceAnimation()->GetStartTime() << std::endl;
		DebugMsg << "Source end time   : " << blender.GetSourceAnimation()->GetEndTime() << std::endl;
		DebugMsg << "Source playback time : " << SourcePlayback << std::endl;
		DebugMsg << "Current root motion : " << blender.GetCurrentRootOffset().ToString() << std::endl;

		DebugMsg << "Blend from : " << (blender.GetSourceAnimation() ? blender.GetSourceAnimation()->GetName() : "") << std::endl;
		DebugMsg << "Blend to   : " << (blender.GetTargetAnimation() ? blender.GetTargetAnimation()->GetName() : "") << std::endl;
		DebugMsg << "Blend time : " << blender.GetElapsedBlendTime() << std::endl;

		if (FTGPlayerController::DrawDebugHitShape)
		{
			DebugMsg << "Draw debug hit shape" << std::endl;
		}

		m_Text.SetText(DebugMsg.str().c_str(), RColor(0, 1, 0), RColor(0, 0, 0, 0.75f));
	}

	UpdateCameraPosition(timer.DeltaTime());

	//GNavigationSystem.DebugRender(NavMeshDebug_DrawNavMesh);
	if (MaxNumPlayers >= 2)
	{
		GNavigationSystem.DebugSetPathQueryPoints(m_Player[0]->GetPosition(), m_Player[1]->GetPosition());
	}
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
			bool bNavSlowly = RInput.IsKeyDown(VK_SHIFT);

			if (moveVec.SquaredMagitude() > 0.0f)
			{
				moveVec.Normalize();
				moveVec *= bNavSlowly ? 300.0f : 600.0f;
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

	//camVec = DefaultScene->TestMovingAabbWithScene(camAabb, camVec, FTGPlayerController::ActivePlayerControllers);

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
