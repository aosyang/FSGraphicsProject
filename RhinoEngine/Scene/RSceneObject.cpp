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
	GScriptSystem.RegisterScriptableObject(this);
}

RSceneObject::~RSceneObject()
{
	GScriptSystem.UnregisterScriptableObject(this);
}

void RSceneObject::Release()
{
	for (auto iter = SceneComponents.begin(); iter != SceneComponents.end(); iter++)
	{
		delete *iter;
	}

	SceneComponents.clear();
}

RMatrix4 RSceneObject::GetNodeTransform() const
{
	return m_NodeTransform.GetMatrix();
}

void RSceneObject::SetTransform(const RMatrix4& transform)
{
	RVec3 Position;
	RQuat Rotation;
	RVec3 Scale;

	if (transform.Decompose(Position, Rotation, Scale))
	{
		m_NodeTransform.SetPosition(Position);
		m_NodeTransform.SetRotation(Rotation);
		m_NodeTransform.SetScale(Scale);
	}
}

void RSceneObject::SetTransform(const RVec3& InPosition, const RQuat& InRotation, const RVec3& InScale /*= RVec3(1, 1, 1)*/)
{
	m_NodeTransform.SetPosition(InPosition);
	m_NodeTransform.SetRotation(InRotation);
	m_NodeTransform.SetScale(InScale);
}

void RSceneObject::SetRotation(const RQuat& quat)
{
	m_NodeTransform.SetRotation(quat);
}

void RSceneObject::SetPosition(const RVec3& pos)
{
	m_NodeTransform.SetPosition(pos);
}

RVec3 RSceneObject::GetPosition() const
{
	return m_NodeTransform.GetPosition();
}

void RSceneObject::SetScale(const RVec3& scale)
{
	m_NodeTransform.SetScale(scale);
}

const RVec3& RSceneObject::GetScale() const
{
	return m_NodeTransform.GetScale();
}

void RSceneObject::LookAt(const RVec3& target, const RVec3& world_up /*= RVec3(0, 1, 0)*/)
{
	m_NodeTransform.LookAt(target, world_up);

	//RVec3 pos = m_NodeTransform.GetTranslation();
	//RVec3 forward = target - pos;
	//forward.Normalize();
	//RVec3 right = RVec3::Cross(world_up, forward);
	//right.Normalize();
	//RVec3 up = RVec3::Cross(forward, right);

	//m_NodeTransform.SetRow(0, RVec4(right, 0));
	//m_NodeTransform.SetRow(1, RVec4(up, 0));
	//m_NodeTransform.SetRow(2, RVec4(forward, 0));
	//m_NodeTransform.SetRow(3, RVec4(pos, 1));
}

void RSceneObject::Translate(const RVec3& v)
{
	m_NodeTransform.Translate(v);
}

void RSceneObject::TranslateLocal(const RVec3& v)
{
	m_NodeTransform.TranslateLocal(v);
}

void RSceneObject::Update()
{
	UpdateComponents();
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

void RSceneObject::UpdateComponents()
{
	for (RSceneComponentBase* SceneComponent : SceneComponents)
	{
		SceneComponent->Update();
	}
}
