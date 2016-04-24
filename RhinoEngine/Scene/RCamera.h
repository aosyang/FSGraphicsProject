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
public:
	RCamera();
	~RCamera();

	SceneObjectType GetType() const { return SO_Camera; }

	const RMatrix4& GetViewMatrix();
	const RMatrix4& GetProjectionMatrix();
	RFrustum GetFrustum() const;

	void SetupView(float fov, float aspect, float _near, float _far);

	float GetFOV() const { return m_FOV; }
	float GetAspectRatio() const { return m_Aspect; }
	float GetNearPlane() const { return m_Near; }
	float GetFarPlane() const { return m_Far; }

private:
	RMatrix4	m_ViewMatrix;
	RMatrix4	m_ProjectionMatrix;

	float		m_FOV;
	float		m_Aspect;
	float		m_Near, m_Far;

	bool		m_DirtyProjMatrix;
};

#endif
