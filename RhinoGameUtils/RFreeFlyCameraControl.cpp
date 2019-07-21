//=============================================================================
// RFreeFlyCameraControl.cpp by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#include "RFreeFlyCameraControl.h"

RFreeFlyCameraControl::RFreeFlyCameraControl(RSceneObject* InOwner)
	: Base(InOwner)
	, NavigationSpeed(1000.0f)
	, m_Enabled(false)
	, m_CamYaw(0.0f)
	, m_CamPitch(0.0f)
{
}

void RFreeFlyCameraControl::Update(float DeltaTime)
{
	if (m_Enabled)
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

		float SpeedBoost = 1.0f;
		if (RInput.IsKeyDown(VK_LSHIFT))
			SpeedBoost *= 4.0f;
		RVec3 moveVec(0.0f, 0.0f, 0.0f);
		if (RInput.IsKeyDown('W'))
			moveVec += RVec3(0.0f, 0.0f, 1.0f) * DeltaTime * NavigationSpeed * SpeedBoost;
		if (RInput.IsKeyDown('S'))
			moveVec -= RVec3(0.0f, 0.0f, 1.0f) * DeltaTime * NavigationSpeed * SpeedBoost;
		if (RInput.IsKeyDown('A'))
			moveVec -= RVec3(1.0f, 0.0f, 0.0f) * DeltaTime * NavigationSpeed * SpeedBoost;
		if (RInput.IsKeyDown('D'))
			moveVec += RVec3(1.0f, 0.0f, 0.0f) * DeltaTime * NavigationSpeed * SpeedBoost;
		if (RInput.IsKeyDown('Q'))
			moveVec -= RVec3(0.0f, 1.0f, 0.0f) * DeltaTime * NavigationSpeed * SpeedBoost;
		if (RInput.IsKeyDown('E'))
			moveVec += RVec3(0.0f, 1.0f, 0.0f) * DeltaTime * NavigationSpeed * SpeedBoost;

		RCamera* Camera = GetOwner()->CastTo<RCamera>();
		if (Camera)
		{
			Camera->Translate(moveVec, ETransformSpace::Local);
			Camera->SetRotation(RQuat::Euler(m_CamPitch, m_CamYaw, 0.0f));
		}
	}
}

void RFreeFlyCameraControl::SetEnabled(bool Enabled)
{
	m_Enabled = Enabled;
	if (m_Enabled)
	{
		RCamera* Camera = GetOwner()->CastTo<RCamera>();
		if (Camera)
		{
			// TODO: RQuat::ToEuler does not return angles in the same order as they are created
			RVec3 CameraRotation = Camera->GetRotation().ToEuler();
			m_CamPitch = CameraRotation.X();

			// Limit pitch between -PI/2 and PI/2
			{
				while (m_CamPitch > PI / 2)
				{
					m_CamPitch -= PI;
				}

				while (m_CamPitch < -PI / 2)
				{
					m_CamPitch += PI;
				}
			}

			m_CamYaw = CameraRotation.Z();
		}
	}
}
