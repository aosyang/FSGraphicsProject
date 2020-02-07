//=============================================================================
// RFreeFlyCameraControl.h by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Rhino.h"

/// A component handling free fly camera inputs
class RFreeFlyCameraControl : public RSceneComponent
{
	DECLARE_SCENE_COMPONENT(RFreeFlyCameraControl, RSceneComponent);
public:
	RFreeFlyCameraControl(RSceneObject* InOwner);

	/// Overrides RSceneComponent
	virtual void Update(float DeltaTime) override;

	void SetEnabled(bool Enabled);

	float GetNavigationSpeed() const;
	void SetNavigationSpeed(float InSpeed);

	void FocusOnObject(RSceneObject* SceneObject);

private:
	/// How fast the camera can move
	float NavigationSpeed;

	bool m_Enabled;

	float m_CamYaw;
	float m_CamPitch;

	float FocalPointDistance;
	bool bEasingOutToTarget;
	RVec3 TargetPosition;
};


FORCEINLINE float RFreeFlyCameraControl::GetNavigationSpeed() const
{
	return NavigationSpeed;
}

FORCEINLINE void RFreeFlyCameraControl::SetNavigationSpeed(float InSpeed)
{
	NavigationSpeed = RMath::Max(InSpeed, 1.0f);
}
