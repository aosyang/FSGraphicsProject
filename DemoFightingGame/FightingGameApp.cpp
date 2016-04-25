//=============================================================================
// FightingGameApp.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "FightingGameApp.h"

FightingGameApp::FightingGameApp()
	: m_Player(nullptr), m_PlayerRot(0.0f)
{

}

FightingGameApp::~FightingGameApp()
{
	SAFE_DELETE(m_Player);
	m_Scene.Release();
	m_DebugRenderer.Release();
	RShaderManager::Instance().UnloadAllShaders();
	RResourceManager::Instance().Destroy();
}

bool FightingGameApp::Initialize()
{
	RResourceManager::Instance().Initialize();
	//RResourceManager::Instance().LoadAllResources();
	RShaderManager::Instance().LoadShaders("../Shaders");

	m_DebugRenderer.Initialize();

	m_Scene.Initialize();
	m_Scene.LoadFromFile("../Assets/TestMap.rmap");

	RMatrix4 cameraMatrix = RMatrix4::CreateXAxisRotation(0.09f * 180 / PI) * RMatrix4::CreateYAxisRotation(3.88659930f * 180 / PI);
	cameraMatrix.SetTranslation(RVec3(407.023712f, 339.007507f, 876.396484f));
	m_Camera.SetTransform(cameraMatrix);
	m_Camera.SetupView(65.0f, RRenderer.AspectRatio(), 1.0f, 10000.0f);

	//m_Player = (RSMeshObject*)m_Scene.FindObject("Player");
	//if (m_Player)
	//{
	//	m_Scene.RemoveObjectFromScene(m_Player);
	//}

	m_Player = new PlayerController();
	m_Player->SetScene(&m_Scene);
	m_Player->SetPosition(RVec3(0, 100.0f, 0));
	m_Player->Cache();

	return true;
}

float LerpDegreeAngle(float from, float to, float t)
{
	while (from - to > 180.0f)
	{
		from -= 360.0f;
	}

	while (from - to < -180.0f)
	{
		from += 360.0f;
	}

	if (t < 0) t = 0;
	if (t > 1) t = 1;

	return from + (to - from) * t;
}

void FightingGameApp::UpdateScene(const RTimer& timer)
{
	if (m_Player)
	{
		if (RInput.GetBufferedKeyState('R') == BKS_Pressed)
			m_Player->SetPosition(RVec3(0, 100.0f, 0));

		m_Player->PreUpdate(timer);
		
		RVec3 moveVec = RVec3(0, 0, 0);
		RAabb playerAabb;
		playerAabb.pMin = RVec3(-50.0f, 0.0f, -50.0f) + m_Player->GetPosition();
		playerAabb.pMax = RVec3(50.0f, 150.0f, 50.0f) + m_Player->GetPosition();

		RVec3 charRight = m_Camera.GetNodeTransform().GetRight();
		RVec3 charForward = charRight.Cross(RVec3(0, 1, 0));

		if (m_Player->GetBehavior() == BHV_Running ||
			m_Player->GetBehavior() == BHV_Idle)
		{
			if (RInput.IsKeyDown('W')) moveVec += charForward;
			if (RInput.IsKeyDown('S')) moveVec -= charForward;
			if (RInput.IsKeyDown('A')) moveVec -= charRight;
			if (RInput.IsKeyDown('D')) moveVec += charRight;

			if (moveVec.SquaredMagitude() > 0.0f)
			{
				moveVec.Normalize();
				m_PlayerRot = LerpDegreeAngle(m_PlayerRot, RAD_TO_DEG(atan2f(moveVec.x, moveVec.z)), 10.0f * timer.DeltaTime());

				moveVec *= timer.DeltaTime() * 500.0f;

				m_Player->SetBehavior(BHV_Running);
			}
			else
			{
				m_Player->SetBehavior(BHV_Idle);
			}
		}

		if (RInput.GetBufferedKeyState(VK_SPACE) == BKS_Pressed)
		{
			m_Player->SetBehavior(BHV_SpinAttack);
		}

		moveVec += RVec3(0, -1000.0f * timer.DeltaTime(), 0);

		RVec3 worldMoveVec = moveVec;
		worldMoveVec += (RVec4(m_Player->GetRootOffset(), 0) * m_Player->GetNodeTransform()).ToVec3();
		worldMoveVec = m_Scene.TestMovingAabbWithScene(playerAabb, worldMoveVec);

		m_Player->Translate(worldMoveVec);

		m_Player->SetRotation(RMatrix4::CreateYAxisRotation(m_PlayerRot));

		RMatrix4 playerTranslation = RMatrix4::CreateTranslation(m_Player->GetNodeTransform().GetTranslation());
		RMatrix4 cameraTransform = RMatrix4::CreateTranslation(0.0f, 500.0f, 300.0f) * playerTranslation;
		m_Camera.SetTransform(cameraTransform);
		m_Camera.LookAt(m_Player->GetPosition() + RVec3(0, 125, 0));

		playerAabb.pMin = RVec3(-50.0f, 0.0f, -50.0f) + m_Player->GetPosition();
		playerAabb.pMax = RVec3(50.0f, 150.0f, 50.0f) + m_Player->GetPosition();
		m_DebugRenderer.DrawAabb(playerAabb);

		m_Player->PostUpdate(timer);
	}
}

void FightingGameApp::RenderScene()
{

	// Update scene constant buffer
	SHADER_SCENE_BUFFER cbScene;

	cbScene.viewMatrix = m_Camera.GetViewMatrix();
	cbScene.projMatrix = m_Camera.GetProjectionMatrix();
	cbScene.viewProjMatrix = cbScene.viewMatrix * cbScene.projMatrix;
	cbScene.cameraPos = m_Camera.GetPosition();

	m_Scene.cbScene.UpdateContent(&cbScene);
	m_Scene.cbScene.ApplyToShaders();


	// Update light constant buffer
	SHADER_LIGHT_BUFFER cbLight;
	ZeroMemory(&cbLight, sizeof(cbLight));

	// Setup ambient color
	cbLight.HighHemisphereAmbientColor = RVec4(1.0f, 1.0f, 1.0f, 1.0f);
	cbLight.LowHemisphereAmbientColor = RVec4(0.2f, 0.2f, 0.2f, 1.0f);

	m_Scene.cbLight.UpdateContent(&cbLight);
	m_Scene.cbLight.ApplyToShaders();

	RRenderer.SetSamplerState(0, SamplerState_Texture);
	RRenderer.SetSamplerState(2, SamplerState_ShadowDepthComparison);

	RRenderer.Clear();

	m_Scene.Render();

	SHADER_OBJECT_BUFFER cbObject;

	if (m_Player)
	{
		cbObject.worldMatrix = m_Player->GetNodeTransform();
		m_Scene.cbPerObject.UpdateContent(&cbObject);
		//RRenderer.SetBlendState(Blend_AlphaBlending);
		m_Player->Draw();
		//RRenderer.SetBlendState(Blend_Opaque);
	}

	cbObject.worldMatrix = RMatrix4::IDENTITY;
	m_Scene.cbPerObject.UpdateContent(&cbObject);
	m_DebugRenderer.Render();
	m_DebugRenderer.Reset();

	RRenderer.Present();
}

void FightingGameApp::OnResize(int width, int height)
{

}
