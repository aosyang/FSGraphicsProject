//=============================================================================
// SimpleGame.cpp by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#include "SimpleGame.h"


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
	RVec3 moveVec(0.0f, 0.0f, 0.0f);
	if (RInput.IsKeyDown('W'))
		moveVec += RVec3(0.0f, 0.0f, 1.0f) * timer.DeltaTime() * camSpeed;
	if (RInput.IsKeyDown('S'))
		moveVec -= RVec3(0.0f, 0.0f, 1.0f) * timer.DeltaTime() * camSpeed;
	if (RInput.IsKeyDown('A'))
		moveVec -= RVec3(1.0f, 0.0f, 0.0f) * timer.DeltaTime() * camSpeed;
	if (RInput.IsKeyDown('D'))
		moveVec += RVec3(1.0f, 0.0f, 0.0f) * timer.DeltaTime() * camSpeed;

	RMatrix4 cameraMatrix = RMatrix4::CreateXAxisRotation(m_CamPitch * 180 / PI) * RMatrix4::CreateYAxisRotation(m_CamYaw * 180 / PI);
	m_Camera.SetTransform(cameraMatrix);

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
}

void SimpleGame::RenderScene()
{
	// Clear back buffer
	GRenderer.Clear();

	RConstantBuffers::cbScene.BindBuffer();

	SHADER_OBJECT_BUFFER cbObject;
	ZeroMemory(&cbObject, sizeof(cbObject));

	cbObject.worldMatrix = RMatrix4::IDENTITY;
	RConstantBuffers::cbPerObject.UpdateBufferData(&cbObject);
	RConstantBuffers::cbPerObject.BindBuffer();

	// Use default texture sampler for texture slot 0 
	GRenderer.SetSamplerState(0, SamplerState_Texture);

	m_Skybox.Draw();

	// Draw the scene
	RFrustum Frustum = m_Camera.GetFrustum();
	m_Scene.Render(&Frustum);

	// Present current frame
	GRenderer.Present();
}
