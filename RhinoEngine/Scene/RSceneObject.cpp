//=============================================================================
// RSceneObject.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RSceneObject.h"

RSceneObject::RSceneObject()
{
	m_NodeTransform = RMatrix4::IDENTITY;
}

RSceneObject::~RSceneObject()
{

}

const RMatrix4& RSceneObject::GetNodeTransform() const
{
	return m_NodeTransform;
}

void RSceneObject::SetTransform(const RMatrix4& transform)
{
	m_NodeTransform = transform;
}

void RSceneObject::SetPosition(const RVec3& pos)
{
	m_NodeTransform.SetTranslation(pos);
}

RVec3 RSceneObject::GetPosition() const
{
	return m_NodeTransform.GetRow(3).ToVec3();
}

void RSceneObject::Draw()
{

}
