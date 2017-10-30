//=============================================================================
// SimpleGame.cpp by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#include "SimpleGame.h"
#include "RgRubik.h"

SimpleGame::SimpleGame()
	: m_Camera(nullptr)
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

	m_Camera = m_Scene.CreateSceneObjectOfType<RCamera>("UserCamera");

	// TODO: if projection is forgotten to set properly, need to make sure user gets some feedbacks
	m_Camera->SetupView(65.0f, GRenderer.AspectRatio(), 1.0f, 10000.0f);
	m_Camera->SetPosition(RVec3(0, 0, -500.0f));

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

	m_CameraOrbiter->SetRotation(RQuat::Euler(m_CamPitch, m_CamYaw, 0.0f));

	if (RInput.IsKeyDown(VK_UP))
	{
		if (RInput.GetBufferedKeyState(VK_LEFT) == BKS_Pressed)
		{
			m_RubikCube->Rotate(ERubikSide::Top, ERubikRotation::CW);
		}
		else if (RInput.GetBufferedKeyState(VK_RIGHT) == BKS_Pressed)
		{
			m_RubikCube->Rotate(ERubikSide::Top, ERubikRotation::CCW);
		}
	}

	if (RInput.IsKeyDown(VK_DOWN))
	{
		if (RInput.GetBufferedKeyState(VK_LEFT) == BKS_Pressed)
		{
			m_RubikCube->Rotate(ERubikSide::Bottom, ERubikRotation::CCW);
		}
		else if (RInput.GetBufferedKeyState(VK_RIGHT) == BKS_Pressed)
		{
			m_RubikCube->Rotate(ERubikSide::Bottom, ERubikRotation::CW);
		}
	}

	if (RInput.IsKeyDown(VK_LEFT))
	{
		if (RInput.GetBufferedKeyState(VK_UP) == BKS_Pressed)
		{
			m_RubikCube->Rotate(ERubikSide::West, ERubikRotation::CCW);
		}
		else if (RInput.GetBufferedKeyState(VK_DOWN) == BKS_Pressed)
		{
			m_RubikCube->Rotate(ERubikSide::West, ERubikRotation::CW);
		}
	}

	if (RInput.IsKeyDown(VK_RIGHT))
	{
		if (RInput.GetBufferedKeyState(VK_UP) == BKS_Pressed)
		{
			m_RubikCube->Rotate(ERubikSide::East, ERubikRotation::CW);
		}
		else if (RInput.GetBufferedKeyState(VK_DOWN) == BKS_Pressed)
		{
			m_RubikCube->Rotate(ERubikSide::East, ERubikRotation::CCW);
		}
	}

	if (RInput.GetBufferedKeyState('Z') == BKS_Pressed)
	{
		m_RubikCube->Rotate(ERubikSide::South, ERubikRotation::CCW);
	}
	else if (RInput.GetBufferedKeyState('X') == BKS_Pressed)
	{
		m_RubikCube->Rotate(ERubikSide::South, ERubikRotation::CW);
	}

	if (RInput.GetBufferedKeyState('C') == BKS_Pressed)
	{
		m_RubikCube->Rotate(ERubikSide::North, ERubikRotation::CW);
	}
	else if (RInput.GetBufferedKeyState('V') == BKS_Pressed)
	{
		m_RubikCube->Rotate(ERubikSide::North, ERubikRotation::CCW);
	}

	m_Scene.UpdateScene();
}

void SimpleGame::RenderScene()
{
	// Implemented in engine now
}

void SimpleGame::OnResize(int width, int height)
{
	if (m_Camera)
	{
		m_Camera->SetAspectRatio((float)width / (float)height);
	}
}

TCHAR* SimpleGame::WindowTitle()
{
	return L"Rubik\'s Cube";
}

void SimpleGame::SetupScene()
{
	m_RubikCube = m_Scene.CreateSceneObjectOfType<RgRubik>("Rubik Cube");

	RSceneObject* GlobalLightInfo = m_Scene.CreateSceneObjectOfType<RSceneObject>("DirectionalLight");
	RDirectionalLightComponent* DirLightComponent = GlobalLightInfo->AddNewComponent<RDirectionalLightComponent>();
	DirLightComponent->SetParameters({ RVec3(-0.5f, 1, -0.3f), RColor(0.5f, 0.5f, 0.5f) });

	m_CameraOrbiter = m_Scene.CreateSceneObjectOfType<RSceneObject>("CameraOrbiter");
	m_Camera->SetPosition(RVec3(0, 0, -500.0f));
	m_Camera->AttachTo(m_CameraOrbiter);
}
