//=============================================================================
// FightingGameApp.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "FightingGameApp.h"

#include "AIBehavior_Fighter.h"
#include "AIBehavior_Roamer.h"

#include "PlayerAssets.h"

const static int MaxNumAIs = 10;
const static int MaxNumPlayers = 1;

FightingGameApp::FightingGameApp()
	: CurrentPlayerIndex(0)
	, m_Camera(nullptr)
	, m_FreeFlyMode(false)
	, m_FreeFlyCameraControl(nullptr)
{
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

#if 0
	{
		// Create a new animation graph resource
		RAnimGraph* AnimGraph = RResourceManager::Instance().CreateNewResource<RAnimGraph>("/Maid/Maid_Navigation.ranimgraph");

		// Bind an animation player node as root node
		RAnimGraphNode* AnimPlayerNode = AnimGraph->AddInputAnimNode("BlendPlayer");

		// Set animation resource for the animation player node
		//AnimPlayerNode->Attributes["Animation"] = "/Maid/Maid_Walk.fbx";
		AnimPlayerNode->Attributes["Loop"] = "True";

		AnimNodeAttributeMap::ChildEntry Entry;
		Entry.EntryName = std::string("BlendEntry");
		Entry["Animation"] = "/Maid/Maid_Walk.fbx";
		Entry["SampleValue"] = "1.0";
		AnimPlayerNode->Attributes.ChildEntries.push_back(std::move(Entry));

		// Save the animation graph to file
		AnimGraph->SaveToDisk();
	}
#endif	// if 0

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

	m_Camera = DefaultScene->CreateSceneObjectOfType<RCamera>();
	m_Camera->SetTransform(RVec3(407.023712f, 339.007507f, 876.396484f), RQuat::Euler(0.09f, 3.88659930f, 0.0f));
	m_Camera->SetupView(45.0f, GRenderer.AspectRatio(), 1.0f, 10000.0f);
	m_FreeFlyCameraControl = m_Camera->AddNewComponent<RFreeFlyCameraControl>();
	m_FreeFlyCameraControl->SetEnabled(false);

	m_Players.resize(MaxNumPlayers, nullptr);
	for (int i = 0; i < MaxNumPlayers; i++)
	{
		m_Players[i] = DefaultScene->CreateSceneObjectOfType<PlayerControllerBase>();
		if (m_Players[i])
		{
			// Better responsiveness in control
			m_Players[i]->SetMotorAcceleration(1000.0f);

			FTGPlayerStateMachine& StateMachine = m_Players[i]->GetStateMachine();
			InitializePlayerAsset_Maid(m_Players[i]);

			// Hacked start points for players
			{
				if (i == 0)
				{
					m_Players[i]->SetPosition(RVec3(-1795, 200, -200));
				}

				if (i == 1)
				{
					m_Players[i]->SetPosition(RVec3(1835, 40, -853));
				}
			}
		}
	}

	m_AIPlayers.resize(MaxNumAIs, nullptr);
	for (int i = 0; i < MaxNumAIs; i++)
	{
		m_AIPlayers[i] = DefaultScene->CreateSceneObjectOfType<FTGPlayerController>();
		if (m_AIPlayers[i])
		{
			m_AIPlayers[i]->SetPosition(RVec3(RMath::RandRangedF(-800, 800), 50, RMath::RandRangedF(-800, 800)));
			InitializePlayerAsset_Maid(m_AIPlayers[i]);

			// Make each AI play animation at a slightly different speed
			//m_AIPlayers[i]->SetAnimationDeviation(RMath::RandRangedF(0.8f, 1.2f));
			//m_AIPlayers[i]->SetAnimationDeviation(0.8f);

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
			if (m_Players[i])
			{
				m_Players[i]->SetPosition(RVec3(0, 100.0f, 0));
				m_Players[i]->Reset();
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

		std::stringstream DebugMsg;
		DebugMsg << "Player: ("
				 << playerPos.X() << ", "
				 << playerPos.Y() << ", "
				 << playerPos.Z() << "), rot: "
				 << playerRot << std::endl;

		DebugMsg << CurrentPlayer->GetStateMachine().GetDebugString();

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
		GNavigationSystem.DebugSetPathQueryPoints(m_Players[0]->GetPosition(), m_Players[1]->GetPosition());
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
				moveVec *= bNavSlowly ? 100.f : 285.f;
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
	static const RVec3 CameraOffset = RVec3(0.0f, 3.0f, 3.0f) * 115.0f;
	RMatrix4 cameraTransform = RMatrix4::CreateTranslation(CameraOffset) * playerTranslation;

	// Player mesh has its origin at feet. We offset the look target up by half of player's height.
	RVec3 lookTarget = PlayerPosition + RVec3(0, 100, 0);

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
	return m_Players[CurrentPlayerIndex];
}
