//=============================================================================
// REditorAxis.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Scene/RSceneObject.h"

class RCamera;

enum EAxis
{
	AXIS_X,
	AXIS_Y,
	AXIS_Z,

	AXIS_COUNT,
};

enum class EMouseControlMode
{
	None,
	MoveX,
	MoveY,
	MoveZ,
};

class REditorAxis : public RSceneObject
{
	DECLARE_SCENE_OBJECT(REditorAxis, RSceneObject)
public:
	virtual void Update(float DeltaTime) override;

	EMouseControlMode ProcessMouseActions(const RRay& CameraRay, RSceneObject* SelectedObject);

	void SetSelectedObject(RSceneObject* Selected);

	EMouseControlMode GetMouseControlMode() const;

protected:
	REditorAxis(const RConstructingParams& Params);

	float SnapTo(float Value, float Unit);

private:
	const RAabb& GetLocalAabb(EAxis Axis) const;

	RPlane GetAxisPlane(const RVec3& Point, const RVec3& AxisDirection) const;

	EMouseControlMode MouseControlMode;
	RVec3 CursorStartPosition;
	RVec3 ObjectStartPosition;

	RSceneObject* SelectedObject;
	RCamera* EditorCamera;

	RRenderMeshComponent* AxisX;
	RRenderMeshComponent* AxisY;
	RRenderMeshComponent* AxisZ;
};

FORCEINLINE EMouseControlMode REditorAxis::GetMouseControlMode() const
{
	return MouseControlMode;
}
