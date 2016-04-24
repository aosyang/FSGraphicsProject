//=============================================================================
// RCamera.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"
#include "RCamera.h"

RCamera::RCamera()
	: RSceneObject(), m_DirtyProjMatrix(true)
{

}

RCamera::~RCamera()
{

}

const RMatrix4& RCamera::GetViewMatrix()
{
	m_ViewMatrix = m_NodeTransform.FastInverse();
	return m_ViewMatrix;
}

const RMatrix4& RCamera::GetProjectionMatrix()
{
	if (m_DirtyProjMatrix)
	{
		m_ProjectionMatrix = RMatrix4::CreatePerspectiveProjectionLH(m_FOV, m_Aspect, m_Near, m_Far);
		m_DirtyProjMatrix = false;
	}

	return m_ProjectionMatrix;
}

RFrustum RCamera::GetFrustum() const
{
	RFrustum frustum;

	RVec3 nc = m_NodeTransform.GetTranslation() + m_NodeTransform.GetForward() * m_Near;
	RVec3 fc = m_NodeTransform.GetTranslation() + m_NodeTransform.GetForward() * m_Far;

	float hNear = 2.0f * tanf(DEG_TO_RAD(m_FOV) / 2.0f) * m_Near;
	float hFar = 2.0f * tanf(DEG_TO_RAD(m_FOV) / 2.0f) * m_Far;
	float wNear = hNear * m_Aspect;
	float wFar = hFar * m_Aspect;
	frustum.corners[FC_FTL] = fc + m_NodeTransform.GetUp() * (hFar * 0.5f) - m_NodeTransform.GetRight() * (wFar * 0.5f);
	frustum.corners[FC_FTR] = fc + m_NodeTransform.GetUp() * (hFar * 0.5f) + m_NodeTransform.GetRight() * (wFar * 0.5f);
	frustum.corners[FC_FBL] = fc - m_NodeTransform.GetUp() * (hFar * 0.5f) - m_NodeTransform.GetRight() * (wFar * 0.5f);
	frustum.corners[FC_FBR] = fc - m_NodeTransform.GetUp() * (hFar * 0.5f) + m_NodeTransform.GetRight() * (wFar * 0.5f);
	frustum.corners[FC_NTL] = nc + m_NodeTransform.GetUp() * (hNear * 0.5f) - m_NodeTransform.GetRight() * (wNear * 0.5f);
	frustum.corners[FC_NTR] = nc + m_NodeTransform.GetUp() * (hNear * 0.5f) + m_NodeTransform.GetRight() * (wNear * 0.5f);
	frustum.corners[FC_NBL] = nc - m_NodeTransform.GetUp() * (hNear * 0.5f) - m_NodeTransform.GetRight() * (wNear * 0.5f);
	frustum.corners[FC_NBR] = nc - m_NodeTransform.GetUp() * (hNear * 0.5f) + m_NodeTransform.GetRight() * (wNear * 0.5f);

	frustum.planes[0] = RPlane(frustum.corners[FC_NBR], frustum.corners[FC_NBL], frustum.corners[FC_NTL]);
	frustum.planes[1] = RPlane(frustum.corners[FC_FBL], frustum.corners[FC_FBR], frustum.corners[FC_FTR]);
	frustum.planes[2] = RPlane(frustum.corners[FC_NBL], frustum.corners[FC_FBL], frustum.corners[FC_FTL]);
	frustum.planes[3] = RPlane(frustum.corners[FC_FBR], frustum.corners[FC_NBR], frustum.corners[FC_NTR]);
	frustum.planes[4] = RPlane(frustum.corners[FC_NTR], frustum.corners[FC_NTL], frustum.corners[FC_FTL]);
	frustum.planes[5] = RPlane(frustum.corners[FC_NBL], frustum.corners[FC_NBR], frustum.corners[FC_FBR]);

	return frustum;
}

void RCamera::SetupView(float fov, float aspect, float _near, float _far)
{
	m_FOV = fov;
	m_Aspect = aspect;
	m_Near = _near;
	m_Far = _far;
	m_DirtyProjMatrix = true;
}