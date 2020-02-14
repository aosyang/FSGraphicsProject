//=============================================================================
// REditorAxis.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "REditorAxis.h"
#include "EditorCommon.h"

REditorAxis::REditorAxis(const RConstructingParams& Params)
	: Base(Params)
	, MouseControlMode(EMouseControlMode::None)
	, SelectedObject(nullptr)
{
	EditorCamera = Params.Scene->GetRenderCamera();
	assert(EditorCamera);

	AxisX = AddNewComponent<RRenderMeshComponent>();
	AxisX->SetMesh(RResourceManager::Instance().LoadResource<RMesh>("/Editor/ArrowX.fbx", EResourceLoadMode::Immediate));

	AxisY = AddNewComponent<RRenderMeshComponent>();
	AxisY->SetMesh(RResourceManager::Instance().LoadResource<RMesh>("/Editor/ArrowY.fbx", EResourceLoadMode::Immediate));

	AxisZ = AddNewComponent<RRenderMeshComponent>();
	AxisZ->SetMesh(RResourceManager::Instance().LoadResource<RMesh>("/Editor/ArrowZ.fbx", EResourceLoadMode::Immediate));
}

void REditorAxis::Update(float DeltaTime)
{
	Base::Update(DeltaTime);

	if (RInput.GetBufferedKeyState(VK_LBUTTON) == EBufferedKeyState::Released)
	{
		MouseControlMode = EMouseControlMode::None;
	}

	if (MouseControlMode != EMouseControlMode::None)
	{
		if (SelectedObject)
		{
			int mdx, mdy;
			RInput.GetRelativeCursorPosition(mdx, mdy);

			// Hold ctrl key to clone objects
			//if (RInput.GetBufferedKeyState(VK_LBUTTON) == EBufferedKeyState::Pressed &&
			//	RInput.IsKeyDown(VK_LCONTROL))
			//{
			//	RScene* Scene = SelectedObject->GetScene();
			//	SelectedObject = Scene->CloneObject(SelectedObject);
			//	assert(SelectedObject);

			//	if (SceneObjectClonedCallback)
			//	{
			//		(*SceneObjectClonedCallback)(SelectedObject->GetName().c_str());
			//	}
			//}

			RVec3 pos = SelectedObject->GetPosition();

			// Hold alt key to rotate objects
			if (RInput.IsKeyDown(VK_LMENU))
			{
				RQuat Rotation = SelectedObject->GetRotation();
				RQuat DeltaRotation;
				float Angle = SnapTo((float)mdx * 0.05f, PI / 180.0f * 1.0f);

				switch (MouseControlMode)
				{
				case EMouseControlMode::MoveX:
					DeltaRotation = RQuat::Euler(Angle, 0, 0);
					break;

				case EMouseControlMode::MoveY:
					DeltaRotation = RQuat::Euler(0, Angle, 0);
					break;

				case EMouseControlMode::MoveZ:
					DeltaRotation = RQuat::Euler(0, 0, Angle);
					break;
				}

				SelectedObject->SetRotation(Rotation * DeltaRotation);
			}
			else
			{
				// Move scene object along selected axis

				// TODO: Support local space later
				RVec3 AxisVector = RVec3(0, 0, 0);

				switch (MouseControlMode)
				{
				case EMouseControlMode::MoveX:
					AxisVector = RVec3(1, 0, 0);
					break;

				case EMouseControlMode::MoveY:
					AxisVector = RVec3(0, 1, 0);
					break;

				case EMouseControlMode::MoveZ:
					AxisVector = RVec3(0, 0, 1);
					break;

				default:
					break;
				}

				if (AxisVector.SquaredMagitude() > FLT_EPSILON)
				{
					// Get the moving referencing plane and calculate a intersection point with the camera ray.
					// The moving offset is the projected distance of intersection point and start moving on the moving axis.
					RPlane Plane = GetAxisPlane(CursorStartPosition, AxisVector);
					RVec2 CursorPoint = GetMousePositionInViewport();
					RRay CameraRay = MakeRayFromViewportPoint(EditorCamera, CursorPoint);
					float Dist;

					if (CameraRay.TestIntersectionWithPlane(Plane, &Dist))
					{
						RVec3 HitPoint = CameraRay.Origin + CameraRay.Direction * Dist;

						float Offset = SnapTo(RVec3::Dot(HitPoint - CursorStartPosition, AxisVector), 1.0f);
						pos = ObjectStartPosition + AxisVector * Offset;
					}
				}
			}

			SelectedObject->SetPosition(pos);
			// TODO: Notify editor about object being moved
		}
	}

	if (SelectedObject)
	{
		RVec3 cam_pos = EditorCamera->GetPosition();
		RVec3 obj_pos = SelectedObject->GetPosition();
		float dist = (cam_pos - obj_pos).Magnitude();
		dist = max(50.0f, min(100.0f, dist));
		RVec3 axis_pos = cam_pos + (SelectedObject->GetPosition() - cam_pos).GetNormalized() * dist;

		SetPosition(axis_pos);
	}
}

EMouseControlMode REditorAxis::ProcessMouseActions(const RRay& CameraRay, RSceneObject* SelectedObject)
{
	// Transform camera ray to axis local space
	RRay axis_ray = CameraRay.Transform(GetTransformMatrix().FastInverse());

	if (axis_ray.TestIntersectionWithAabb(GetLocalAabb(AXIS_X)))
	{
		MouseControlMode = EMouseControlMode::MoveX;
	}
	else if (axis_ray.TestIntersectionWithAabb(GetLocalAabb(AXIS_Y)))
	{
		MouseControlMode = EMouseControlMode::MoveY;
	}
	else if (axis_ray.TestIntersectionWithAabb(GetLocalAabb(AXIS_Z)))
	{
		MouseControlMode = EMouseControlMode::MoveZ;
	}

	if (MouseControlMode != EMouseControlMode::None)
	{
		// Since the axis may move along camera direction for scaling, the hit point is not always what we're looking for.
		// We'll check ray intersection with axis planes for the actual start position.

		RVec3 ObjectPosition = SelectedObject->GetWorldPosition();
		RPlane AxisPlane;

		switch (MouseControlMode)
		{
		case EMouseControlMode::MoveX:
			AxisPlane = GetAxisPlane(ObjectPosition, RVec3(1, 0, 0));
			break;

		case EMouseControlMode::MoveY:
			AxisPlane = GetAxisPlane(ObjectPosition, RVec3(0, 1, 0));
			break;

		case EMouseControlMode::MoveZ:
			AxisPlane = GetAxisPlane(ObjectPosition, RVec3(0, 0, 1));
			break;

		default:
			break;
		}

		if (AxisPlane.IsValid())
		{
			float Dist;
			if (CameraRay.TestIntersectionWithPlane(AxisPlane, &Dist))
			{
				CursorStartPosition = CameraRay.GetPointAtDistance(Dist);
			}
		}

		ObjectStartPosition = ObjectPosition;
	}

	return MouseControlMode;
}

void REditorAxis::SetSelectedObject(RSceneObject* Selected)
{
	SelectedObject = Selected;
	SetVisible(SelectedObject != nullptr);
}

float REditorAxis::SnapTo(float Value, float Unit)
{
	return Unit * int((Value + Unit * 0.5f) / Unit);
}

const RAabb& REditorAxis::GetLocalAabb(EAxis Axis) const
{
	switch (Axis)
	{
	case AXIS_X:
		return AxisX->GetLocalAabb();
	case AXIS_Y:
		return AxisY->GetLocalAabb();
	case AXIS_Z:
		return AxisZ->GetLocalAabb();
	}

	return RAabb::Default;
}

RPlane REditorAxis::GetAxisPlane(const RVec3& Point, const RVec3& AxisDirection) const
{
	RVec3 p0 = Point;
	RVec3 p1 = Point + AxisDirection;
	RVec3 SideVec = RVec3::Cross((EditorCamera->GetPosition() - Point).GetNormalized(), AxisDirection);
	RVec3 p2 = Point + SideVec;

	return RPlane(p0, p1, p2);
}
