//=============================================================================
// RSceneObject.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RSceneObject.h"

RSceneObject::RSceneObject()
	: m_Scene(nullptr)
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

void RSceneObject::SetRotation(const RMatrix4& rot)
{
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			m_NodeTransform.m[i][j] = rot.m[i][j];
}

void RSceneObject::SetPosition(const RVec3& pos)
{
	m_NodeTransform.SetTranslation(pos);
}

RVec3 RSceneObject::GetPosition() const
{
	return m_NodeTransform.GetRow(3).ToVec3();
}

void RSceneObject::Translate(const RVec3& v)
{
	m_NodeTransform.Translate(v);
}

void RSceneObject::TranslateLocal(const RVec3& v)
{
	m_NodeTransform.TranslateLocal(v);
}
