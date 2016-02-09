//=============================================================================
// RSceneObject.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RSceneObject.h"

RSceneObject::RSceneObject()
{
	XMStoreFloat4x4(&m_NodeTransform, XMMatrixIdentity());
}

RSceneObject::~RSceneObject()
{

}

XMMATRIX RSceneObject::GetNodeTransform() const
{
	return XMLoadFloat4x4(&m_NodeTransform);
}

void RSceneObject::SetPosition(const XMFLOAT3& pos)
{
	m_NodeTransform._41 = pos.x;
	m_NodeTransform._42 = pos.y;
	m_NodeTransform._43 = pos.z;
}

void RSceneObject::Draw()
{

}
