//=============================================================================
// RFreeFlyCameraControl.cpp by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#include "RFreeFlyCameraControl.h"

RFreeFlyCameraControl::RFreeFlyCameraControl(RSceneObject* InOwner)
	: Base(InOwner)
	, NavigationSpeed(1000.0f)
	, m_Enabled(true)
	, m_CamYaw(0.0f)
	, m_CamPitch(0.0f)
	, FocalPointDistance(0.0f)
	, bEasingOutToTarget(false)
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

		bool bIsOrbiting = RInput.IsKeyDown(VK_LBUTTON) && RInput.IsKeyDown(VK_LMENU);
		if (RInput.IsKeyDown(VK_RBUTTON) || bIsOrbiting)
		{
			int dx, dy;
			RInput.GetRelativeCursorPosition(dx, dy);
			if (dx || dy)
			{
				m_CamYaw += (float)dx / 200.0f;
				m_CamPitch += (float)dy / 200.0f;
				m_CamPitch = max(-PI / 2, min(PI / 2, m_CamPitch));
			}

			// Stop easing out to target
			bEasingOutToTarget = false;
		}

		if (bEasingOutToTarget)
		{
			RVec3 CurrentPosition = GetOwner()->GetWorldPosition();
			RVec3 NewPosition = RVec3::Lerp(CurrentPosition, TargetPosition, RMath::Min(DeltaTime * 10.0f, 1.0f));
			GetOwner()->SetPosition(NewPosition);
			if (NewPosition == TargetPosition)
			{
				bEasingOutToTarget = false;
			}
		}

		float SpeedBoost = 1.0f;
		if (RInput.IsKeyDown(VK_LSHIFT))
		{
			SpeedBoost *= 4.0f;
		}

		if (RInput.IsKeyDown(VK_LCONTROL))
		{
			SpeedBoost *= 0.25f;
		}

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
			RVec3 FocalPoint = RVec3::Zero();
			if (bIsOrbiting)
			{
				FocalPoint = Camera->GetWorldPosition() + Camera->GetForwardVector() * FocalPointDistance;
			}

			Camera->Translate(moveVec, ETransformSpace::Local);
			Camera->SetRotation(RQuat::Euler(m_CamPitch, m_CamYaw, 0.0f));

			if (bIsOrbiting)
			{
				Camera->SetPosition(FocalPoint - Camera->GetForwardVector() * FocalPointDistance);
			}
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

void RFreeFlyCameraControl::FocusOnObject(RSceneObject* SceneObject)
{
	RAabb Bounds = SceneObject->GetAabb();
	if (Bounds.IsValid())
	{
		FocalPointDistance = (Bounds.pMax - Bounds.pMin).Magnitude();
		TargetPosition = Bounds.GetCenter() - GetOwner()->GetForwardVector() * FocalPointDistance;
		bEasingOutToTarget = true;
	}
}
