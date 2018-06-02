//=============================================================================
// RCamera.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"
#include "RCamera.h"

RCamera::RCamera(RScene* InScene)
	: Base(InScene),
	  m_bIsProjectionMatrixDirty(true)
{
	if (GRenderer.GetRenderCamera() == nullptr)
	{
		GRenderer.SetRenderCamera(this);
	}

	m_Fov = 65.0f;
	m_Aspect = GRenderer.AspectRatio();
	m_NearZ = 1.0f;
	m_FarZ = 10000.0f;
}

RCamera::~RCamera()
{
	if (GRenderer.GetRenderCamera() == this)
	{
		GRenderer.SetRenderCamera(nullptr);
	}
}

const RMatrix4& RCamera::GetViewMatrix()
{
	m_ViewMatrix = m_NodeTransform.GetMatrix().FastInverse();
	return m_ViewMatrix;
}

const RMatrix4& RCamera::GetProjectionMatrix()
{
	if (m_bIsProjectionMatrixDirty)
	{
		m_ProjectionMatrix = RMatrix4::CreatePerspectiveProjectionLH(m_Fov, m_Aspect, m_NearZ, m_FarZ);
		m_bIsProjectionMatrixDirty = false;
	}

	return m_ProjectionMatrix;
}

RFrustum RCamera::GetFrustum() const
{
	RFrustum frustum;

	RVec3 nc = m_NodeTransform.GetPosition() + m_NodeTransform.GetForward() * m_NearZ;
	RVec3 fc = m_NodeTransform.GetPosition() + m_NodeTransform.GetForward() * m_FarZ;

	float hNear = 2.0f * tanf(DEG_TO_RAD(m_Fov) / 2.0f) * m_NearZ;
	float hFar = 2.0f * tanf(DEG_TO_RAD(m_Fov) / 2.0f) * m_FarZ;
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

	frustum.BuildPlanesFromCorners();

	return frustum;
}

void RCamera::SetupView(float InFov, float InAspectRatio, float InNearZ, float InFarZ)
{
	m_Fov = InFov;
	m_Aspect = InAspectRatio;
	m_NearZ = InNearZ;
	m_FarZ = InFarZ;
	m_bIsProjectionMatrixDirty = true;
}