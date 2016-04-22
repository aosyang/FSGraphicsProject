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
