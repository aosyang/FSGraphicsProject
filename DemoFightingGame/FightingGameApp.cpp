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

	m_Player = (RSMeshObject*)m_Scene.FindObject("Player");
	if (m_Player)
	{
		m_Scene.RemoveObjectFromScene(m_Player);
	}

	return true;
}

void FightingGameApp::UpdateScene(const RTimer& timer)
{
	if (m_Player)
	{
		RVec3 moveVec = RVec3(0, 0, 0);
		RAabb playerAabb;
		playerAabb.pMin = RVec3(-50.0f, 0.0f, -50.0f) + m_Player->GetPosition();
		playerAabb.pMax = RVec3(50.0f, 95.0f, 50.0f) + m_Player->GetPosition();

		if (RInput.IsKeyDown('W')) moveVec += RVec3(1, 0, 0);
		if (RInput.IsKeyDown('S')) moveVec += RVec3(-1, 0, 0);
		if (RInput.IsKeyDown('A')) moveVec += RVec3(0, 0, 1);
		if (RInput.IsKeyDown('D')) moveVec += RVec3(0, 0, -1);

		if (moveVec.SquaredMagitude() > 0.0f)
		{
			moveVec.Normalize();
			moveVec *= timer.DeltaTime() * 1000.0f;

			RVec3 worldMoveVec = (RVec4(moveVec, 0) * m_Player->GetNodeTransform()).ToVec3();
			worldMoveVec = m_Scene.TestMovingAabbWithScene(playerAabb, worldMoveVec);

			m_Player->Translate(worldMoveVec);
		}

		if (RInput.IsKeyDown('Q')) m_PlayerRot -= timer.DeltaTime() * 100.0f;
		if (RInput.IsKeyDown('E')) m_PlayerRot += timer.DeltaTime() * 100.0f;
		m_Player->SetRotation(RMatrix4::CreateYAxisRotation(m_PlayerRot));

		RMatrix4 cameraTransform = RMatrix4::CreateTranslation(50.0f, 100.0f, -200.0f) * RMatrix4::CreateYAxisRotation(90.0f) * m_Player->GetNodeTransform();
		m_Camera.SetTransform(cameraTransform);

		playerAabb.pMin = RVec3(-50.0f, 0.0f, -50.0f) + m_Player->GetPosition();
		playerAabb.pMax = RVec3(50.0f, 95.0f, 50.0f) + m_Player->GetPosition();
		m_DebugRenderer.DrawAabb(playerAabb);
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
		m_Player->Draw();
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
