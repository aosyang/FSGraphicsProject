//=============================================================================
// SimpleGame.cpp by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#include "SimpleGame.h"
#include "RgRubik.h"

SimpleGame::SimpleGame()
	: m_Camera(nullptr),
	  m_IsScramblingCube(false)
{

}


SimpleGame::~SimpleGame()
{
}

bool SimpleGame::Initialize()
{
	m_CamYaw = m_CamPitch = 0.0f;

	// Load texture for skybox
	const std::string SkyboxTextureName("/powderpeak.dds");
	RResourceManager::Instance().LoadResource<RTexture>(SkyboxTextureName);
	m_Skybox.CreateSkybox(SkyboxTextureName);

	m_Camera = GSceneManager.DefaultScene()->CreateSceneObjectOfType<RCamera>("UserCamera");

	m_Camera->SetupView(65.0f, GRenderer.AspectRatio(), 1.0f, 10000.0f);
	m_Camera->SetPosition(RVec3(0, 0, -500.0f));

	SetupScene();

	RInput.BindKeyStateEvent(VK_SPACE, EBufferedKeyState::Pressed, this, &SimpleGame::ScrambleCube);

	return true;
}

void SimpleGame::UpdateScene(const RTimer& timer)
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

	m_CameraOrbiter->SetRotation(RQuat::Euler(m_CamPitch, m_CamYaw, 0.0f));

	if (m_IsScramblingCube)
	{
		if (!m_RubikCube->IsMoveInProcess())
		{
			UINT MoveNums = (UINT)m_ScrambleMoves.size();

			if (MoveNums == 0)
			{
				m_IsScramblingCube = false;
			}
			else
			{
				UINT LastIndex = MoveNums - 1;
				int Move = m_ScrambleMoves[LastIndex];

				ERubikRotation Rotation = (ERubikRotation)(Move % 2);
				Move >>= 1;
				ERubikSide Side = (ERubikSide)(Move % 6);

				m_RubikCube->Rotate(Side, Rotation);
				m_ScrambleMoves.pop_back();
			}
		}
	}
	else
	{
		if (RInput.IsKeyDown(VK_UP))
		{
			if (RInput.GetBufferedKeyState(VK_LEFT) == EBufferedKeyState::Pressed)
			{
				m_RubikCube->Rotate(ERubikSide::Top, ERubikRotation::CW);
			}
			else if (RInput.GetBufferedKeyState(VK_RIGHT) == EBufferedKeyState::Pressed)
			{
				m_RubikCube->Rotate(ERubikSide::Top, ERubikRotation::CCW);
			}
		}

		if (RInput.IsKeyDown(VK_DOWN))
		{
			if (RInput.GetBufferedKeyState(VK_LEFT) == EBufferedKeyState::Pressed)
			{
				m_RubikCube->Rotate(ERubikSide::Bottom, ERubikRotation::CCW);
			}
			else if (RInput.GetBufferedKeyState(VK_RIGHT) == EBufferedKeyState::Pressed)
			{
				m_RubikCube->Rotate(ERubikSide::Bottom, ERubikRotation::CW);
			}
		}

		if (RInput.IsKeyDown(VK_LEFT))
		{
			if (RInput.GetBufferedKeyState(VK_UP) == EBufferedKeyState::Pressed)
			{
				m_RubikCube->Rotate(ERubikSide::West, ERubikRotation::CCW);
			}
			else if (RInput.GetBufferedKeyState(VK_DOWN) == EBufferedKeyState::Pressed)
			{
				m_RubikCube->Rotate(ERubikSide::West, ERubikRotation::CW);
			}
		}

		if (RInput.IsKeyDown(VK_RIGHT))
		{
			if (RInput.GetBufferedKeyState(VK_UP) == EBufferedKeyState::Pressed)
			{
				m_RubikCube->Rotate(ERubikSide::East, ERubikRotation::CW);
			}
			else if (RInput.GetBufferedKeyState(VK_DOWN) == EBufferedKeyState::Pressed)
			{
				m_RubikCube->Rotate(ERubikSide::East, ERubikRotation::CCW);
			}
		}

		if (RInput.GetBufferedKeyState('Z') == EBufferedKeyState::Pressed)
		{
			m_RubikCube->Rotate(ERubikSide::South, ERubikRotation::CCW);
		}
		else if (RInput.GetBufferedKeyState('X') == EBufferedKeyState::Pressed)
		{
			m_RubikCube->Rotate(ERubikSide::South, ERubikRotation::CW);
		}

		if (RInput.GetBufferedKeyState('C') == EBufferedKeyState::Pressed)
		{
			m_RubikCube->Rotate(ERubikSide::North, ERubikRotation::CW);
		}
		else if (RInput.GetBufferedKeyState('V') == EBufferedKeyState::Pressed)
		{
			m_RubikCube->Rotate(ERubikSide::North, ERubikRotation::CCW);
		}
	}
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
	RScene* DefaultScene = GSceneManager.DefaultScene();

	m_RubikCube = DefaultScene->CreateSceneObjectOfType<RgRubik>("Rubik Cube");

	RSceneObject* GlobalLightInfo = DefaultScene->CreateSceneObjectOfType<RSceneObject>("DirectionalLight");
	RDirectionalLightComponent* DirLightComponent = GlobalLightInfo->AddNewComponent<RDirectionalLightComponent>();
	DirLightComponent->SetLightDirection(RVec3(-0.5f, 1, -0.3f));
	DirLightComponent->SetLightColor(RColor(0.5f, 0.5f, 0.5f));

	m_CameraOrbiter = DefaultScene->CreateSceneObjectOfType<RSceneObject>("CameraOrbiter");
	m_Camera->SetPosition(RVec3(0, 0, -500.0f));
	m_Camera->AttachTo(m_CameraOrbiter);
}

void SimpleGame::ScrambleCube()
{
	if (!m_IsScramblingCube)
	{
		m_IsScramblingCube = true;

		const int TotalMoveNums = 15;
		UINT8 Last = 0;

		m_ScrambleMoves.reserve(TotalMoveNums);
		for (int i = 0; i < TotalMoveNums; i++)
		{
			UINT8 n = (UINT8)RMath::RandRangedInt(0, 11);

			if (i != 0)
			{
				// If a move happens to cancel the last one, try making a different move
				while (((n >> 1) == (Last >> 1)) && (n % 2 != Last % 2))
				{
					n = (UINT8)RMath::RandRangedInt(0, 11);
				}
			}

			m_ScrambleMoves.push_back(n);
			Last = n;
		}
	}
}
