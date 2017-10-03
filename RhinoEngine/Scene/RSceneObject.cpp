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

void RSceneObject::LookAt(const RVec3& target, const RVec3& world_up /*= RVec3(0, 1, 0)*/)
{
	RVec3 pos = m_NodeTransform.GetTranslation();
	RVec3 forward = target - pos;
	forward.Normalize();
	RVec3 right = world_up.Cross(forward);
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

const vector<string>& RSceneObject::GetParsedScript()
{
	if (!m_Script.empty() && m_ParsedScript.empty())
	{
		size_t pos = 0;
		string script = m_Script;

		while ((pos = script.find(',')) != string::npos)
		{
			script = script.replace(pos, 1, " ");
		}

		istringstream iss(script);
		copy(istream_iterator<string>(iss),
			istream_iterator<string>(),
			back_inserter(m_ParsedScript));
	}

	return m_ParsedScript;
}
