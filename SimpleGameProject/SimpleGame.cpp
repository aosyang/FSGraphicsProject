//=============================================================================
// SimpleGame.cpp by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#include "SimpleGame.h"
#include "TestMovingComponent.h"

SimpleGame::SimpleGame()
{
}


SimpleGame::~SimpleGame()
{
	m_Skybox.Release();
	m_Scene.Release();
}

bool SimpleGame::Initialize()
{
	m_CamYaw = m_CamPitch = 0.0f;

	m_Scene.Initialize();

	// Load texture for skybox
	char SkyboxTextureName[] = "../Assets/powderpeak.dds";
	RResourceManager::Instance().LoadDDSTexture(SkyboxTextureName);
	m_Skybox.CreateSkybox(SkyboxTextureName);

	// TODO: if projection is forgotten to set properly, need to make sure user gets some feedbacks
	m_Camera.SetupView(65.0f, GRenderer.AspectRatio(), 1.0f, 10000.0f);

	SetupScene();

	return true;
}

void SimpleGame::UpdateScene(const RTimer& timer)
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

	float camSpeed = 1000.0f;
	if (RInput.IsKeyDown(VK_LSHIFT))
		camSpeed *= 10.0f;
	RVec3 MoveVector(0.0f, 0.0f, 0.0f);
	if (RInput.IsKeyDown('W'))
		MoveVector += RVec3(0.0f, 0.0f, 1.0f) * timer.DeltaTime() * camSpeed;
	if (RInput.IsKeyDown('S'))
		MoveVector -= RVec3(0.0f, 0.0f, 1.0f) * timer.DeltaTime() * camSpeed;
	if (RInput.IsKeyDown('D'))
		MoveVector += RVec3(1.0f, 0.0f, 0.0f) * timer.DeltaTime() * camSpeed;
	if (RInput.IsKeyDown('A'))
		MoveVector -= RVec3(1.0f, 0.0f, 0.0f) * timer.DeltaTime() * camSpeed;
	if (RInput.IsKeyDown('E'))
		MoveVector += RVec3(0.0f, 1.0f, 0.0f) * timer.DeltaTime() * camSpeed;
	if (RInput.IsKeyDown('Q'))
		MoveVector -= RVec3(0.0f, 1.0f, 0.0f) * timer.DeltaTime() * camSpeed;

	m_Camera.SetRotation(RQuat::Euler(m_CamPitch, m_CamYaw, 0.0f));
	m_Camera.TranslateLocal(MoveVector);

	RMatrix4 viewMatrix = m_Camera.GetViewMatrix();
	RMatrix4 projMatrix = m_Camera.GetProjectionMatrix();

	// Update scene constant buffer
	SHADER_SCENE_BUFFER cbScene;
	ZeroMemory(&cbScene, sizeof(cbScene));

	cbScene.viewMatrix = viewMatrix;
	cbScene.projMatrix = projMatrix;
	cbScene.viewProjMatrix = viewMatrix * projMatrix;
	cbScene.cameraPos = m_Camera.GetPosition();

	RConstantBuffers::cbScene.UpdateBufferData(&cbScene);
	RConstantBuffers::cbScene.BindBuffer();

	// Update light constant buffer
	SHADER_LIGHT_BUFFER cbLight;
	ZeroMemory(&cbLight, sizeof(cbLight));

	const float AmbientIntensity = 0.8f;

	// Setup ambient color
	cbLight.HighHemisphereAmbientColor = RVec4(0.9f, 1.0f, 1.0f, AmbientIntensity);
	cbLight.LowHemisphereAmbientColor = RVec4(0.2f, 0.2f, 0.2f, AmbientIntensity);

	cbLight.CameraPos = m_Camera.GetPosition();

	RConstantBuffers::cbLight.UpdateBufferData(&cbLight);
	RConstantBuffers::cbLight.BindBuffer();

	m_Scene.UpdateScene();
}

void SimpleGame::RenderScene()
{
	// Implemented in engine now
}

void SimpleGame::SetupScene()
{
	RMesh* SphereMesh = RResourceManager::Instance().LoadFbxMesh("../Assets/sphere.rmesh");
	float UnitAngle = 360.0f / 100.0f;

	for (int i = 0; i < 100; i++)
	{
		RSceneObject* SceneObject = m_Scene.CreateSceneObject();
		RVec3 ObjectPosition;

		ObjectPosition.SetX(Math::RandRangedF(-500.0f, 500.0f));
		ObjectPosition.SetY(Math::RandRangedF(-500.0f, 500.0f));
		ObjectPosition.SetZ(Math::RandRangedF(-500.0f, 500.0f));

		SceneObject->SetPosition(ObjectPosition);
		SceneObject->SetRotation(RQuat::Euler(0, DEG_TO_RAD(UnitAngle * i), 0));

		RRenderMeshComponent* RenderMeshComponent = SceneObject->AddNewComponent<RRenderMeshComponent>();
		RenderMeshComponent->SetMesh(SphereMesh);

		TestMovingComponent* MovingComponent = SceneObject->AddNewComponent<TestMovingComponent>();
	}
}
