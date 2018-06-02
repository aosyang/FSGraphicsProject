//=============================================================================
// RCamera.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#ifndef _RCAMERA_H
#define _RCAMERA_H

#include "RSceneObject.h"

class RCamera : public RSceneObject
{
	typedef RSceneObject Base;
	friend class RScene;
public:
	ESceneObjectType GetType() const { return ESceneObjectType::Camera; }

	const RMatrix4& GetViewMatrix();
	const RMatrix4& GetProjectionMatrix();
	RFrustum GetFrustum() const;

	void SetupView(float InFov, float InAspectRatio, float InNearZ, float InFarZ);
	void SetAspectRatio(float aspect) { m_Aspect = aspect; m_bIsProjectionMatrixDirty = true; }

	float GetFOV() const { return m_Fov; }
	float GetAspectRatio() const { return m_Aspect; }
	float GetNearPlane() const { return m_NearZ; }
	float GetFarPlane() const { return m_FarZ; }

protected:
	RCamera(RScene* InScene);
	~RCamera();

private:
	RMatrix4	m_ViewMatrix;
	RMatrix4	m_ProjectionMatrix;

	float		m_Fov;					// Field of view
	float		m_Aspect;				// Aspect ratio
	float		m_NearZ, m_FarZ;

	bool		m_bIsProjectionMatrixDirty;
};

#endif
