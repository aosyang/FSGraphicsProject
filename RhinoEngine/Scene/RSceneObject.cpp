//=============================================================================
// RSceneObject.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RSceneObject.h"

RSceneObject::RSceneObject(const RConstructingParams& Params)
	: m_Scene(Params.Scene)
	, m_bVisible(true)
	, m_Flags(Params.Flags)
{
	GScriptSystem.RegisterScriptableObject(this);
}

RSceneObject::~RSceneObject()
{
	GScriptSystem.UnregisterScriptableObject(this);
}

void RSceneObject::Release()
{
	SceneComponents.clear();
}

RTransform* RSceneObject::GetTransform()
{
	return &m_NodeTransform;
}

const RMatrix4& RSceneObject::GetTransformMatrix()
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

RVec3 RSceneObject::GetWorldPosition()
{
	RTransform* Parent = m_NodeTransform.GetParent();

	return Parent ? Parent->GetMatrix().Transform(m_NodeTransform.GetPosition()) : m_NodeTransform.GetPosition();
}

RVec3 RSceneObject::GetForwardVector() const
{
	return m_NodeTransform.GetForward();
}

RVec3 RSceneObject::GetRightVector() const
{
	return m_NodeTransform.GetRight();
}

RVec3 RSceneObject::GetUpVector() const
{
	return m_NodeTransform.GetUp();
}

const RQuat RSceneObject::GetRotation() const
{
	return m_NodeTransform.GetRotation();
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
}

void RSceneObject::Translate(const RVec3& v, ETransformSpace Space)
{
	m_NodeTransform.Translate(v, Space);
}

void RSceneObject::AttachTo(RSceneObject* Parent)
{
	m_NodeTransform.Attach(Parent->GetTransform());
}

void RSceneObject::DetachFromParent()
{
	m_NodeTransform.Detach();
}

void RSceneObject::Update(float DeltaTime)
{
	if (m_NodeTransform.IsCacheDirty())
	{
		m_NodeTransform.NotifyChildrenMatricesChanged();
	}

	UpdateComponents(DeltaTime);
}

const std::vector<std::string>& RSceneObject::GetParsedScript()
{
	if (!m_Script.empty() && m_ParsedScript.empty())
	{
		size_t pos = 0;
		std::string script = m_Script;

		while ((pos = script.find(',')) != std::string::npos)
		{
			script = script.replace(pos, 1, " ");
		}

		std::istringstream iss(script);
		copy(std::istream_iterator<std::string>(iss),
			std::istream_iterator<std::string>(),
			back_inserter(m_ParsedScript));
	}

	return m_ParsedScript;
}

void RSceneObject::UpdateComponents(float DeltaTime)
{
	for (auto& SceneComponent : SceneComponents)
	{
		SceneComponent->Update(DeltaTime);
	}
}
