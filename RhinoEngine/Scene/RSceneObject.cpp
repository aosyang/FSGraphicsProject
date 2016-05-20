//=============================================================================
// RSceneObject.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RSceneObject.h"

static int MoveTo(lua_State* state)
{
	int n = lua_gettop(state);

	if (n < 4)
		return 0;

	RSceneObject* obj = static_cast<RSceneObject*>(lua_touserdata(state, 1));
	RVec3 target;
	target.x = (float)lua_tonumber(state, 2);
	target.y = (float)lua_tonumber(state, 3);
	target.z = (float)lua_tonumber(state, 4);

	RVec3 rel = (target - obj->GetPosition()).GetNormalizedVec3();
	if (rel.SquaredMagitude() > 0.0f)
		obj->Translate(rel);

	return 0;
}

void RSceneObject::RegisterScriptFunctions()
{
	RScript.RegisterFunction("MoveTo", MoveTo);
}

RSceneObject::RSceneObject()
	: m_Scene(nullptr)
{
	m_NodeTransform = RMatrix4::IDENTITY;
	RScript.RegisterScriptableObject(this);
}

RSceneObject::~RSceneObject()
{
	RScript.UnregisterScriptableObject(this);
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

void RSceneObject::LookAt(const RVec3 target)
{
	RVec3 pos = m_NodeTransform.GetTranslation();
	RVec3 forward = target - pos;
	forward.Normalize();
	RVec3 right = RVec3(0, 1, 0).Cross(forward);
	right.Normalize();
	RVec3 up = forward.Cross(right);

	m_NodeTransform.SetRow(0, RVec4(right, 0));
	m_NodeTransform.SetRow(1, RVec4(up, 0));
	m_NodeTransform.SetRow(2, RVec4(forward, 0));
	m_NodeTransform.SetRow(3, RVec4(pos, 1));
}

void RSceneObject::Translate(const RVec3& v)
{
	m_NodeTransform.Translate(v);
}

void RSceneObject::TranslateLocal(const RVec3& v)
{
	m_NodeTransform.TranslateLocal(v);
}
