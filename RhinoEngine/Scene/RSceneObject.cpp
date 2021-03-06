//=============================================================================
// RSceneObject.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "RSceneObject.h"

#include "tinyxml2/tinyxml2.h"
#include "RSceneComponentFactory.h"

#include "Core/RScriptSystem.h"
#include "Core/REngine.h"
#include "Scene/RScene.h"

RSceneObject::RSceneObject(const RConstructingParams& Params)
	: m_Scene(Params.Scene)
	, m_bVisible(true)
	, bTransformModified(true)
	, m_Flags(Params.Flags)
	, RenderPass(ERenderPass::SceneObject)
	, bNoCulling(false)
	, bNoShadow(false)
	, BoundsUpdateFrame(0)
	, InternalTransformUpdateCounter(0)
{
	GScriptSystem.RegisterScriptableObject(this);
}

RSceneObject::~RSceneObject()
{
	GScriptSystem.UnregisterScriptableObject(this);
}

void RSceneObject::CalculateBounds()
{
	BoundsUpdateFrame = GEngine.GetFrameCounter();

	const static RAabb UnitBounds(RVec3(-.5f, -.5f, -.5f), RVec3(.5f, .5f, .5f));
	Bounds = UnitBounds.GetTransformedAabb(GetTransformMatrix());

	for (auto& Component : SceneComponents)
	{
		const RAabb& ComponentBounds = Component->GetLocalAabb();
		if (ComponentBounds.IsValid())
		{
			Bounds.Expand(ComponentBounds.GetTransformedAabb(GetTransformMatrix()));
		}
	}
}

void RSceneObject::LoadObjectFromXmlElement(tinyxml2::XMLElement* ObjectElem)
{
	const char* ObjectName = ObjectElem->Attribute("Name");
	if (ObjectName)
	{
		SetName(ObjectName);
	}

	const char* ObjectScript = ObjectElem->Attribute("Script");
	if (ObjectScript)
	{
		SetScript(ObjectScript);
	}

	int RenderPass = (int)ERenderPass::SceneObject;
	ObjectElem->QueryIntAttribute("RenderPass", &RenderPass);
	SetRenderPass((ERenderPass)RenderPass);

	bool NoCullingValue = false;
	ObjectElem->QueryBoolAttribute("NoCulling", &NoCullingValue);
	bNoCulling = NoCullingValue;

	bool NoShadowValue = false;
	ObjectElem->QueryBoolAttribute("NoShadow", &NoShadowValue);
	bNoShadow = NoShadowValue;

	// Load components for object
	tinyxml2::XMLElement* ComponentElem = ObjectElem->FirstChildElement("Component");
	while (ComponentElem)
	{
		const char* ClassName = ComponentElem->Attribute("Class");
		if (ClassName)
		{
			std::string ClassNameStr = ClassName;
			RSceneComponent* Comp = FactoryCreateSceneComponent(ClassNameStr, this);
			if (Comp)
			{
				Comp->LoadComponentFromXmlElement(ComponentElem);
			}
		}

		ComponentElem = ComponentElem->NextSiblingElement("Component");
	}
}

void RSceneObject::SaveObjectToXmlElement(tinyxml2::XMLElement* ObjectElem)
{
	if (GetName() != "")
	{
		ObjectElem->SetAttribute("Name", GetName().c_str());
	}

	if (GetScript() != "")
	{
		ObjectElem->SetAttribute("Script", GetScript().c_str());
	}

	ERenderPass RenderPass = GetRenderPass();
	if (RenderPass != ERenderPass::SceneObject)
	{
		ObjectElem->SetAttribute("RenderPass", (int)RenderPass);
	}

	if (bNoCulling)
	{
		ObjectElem->SetAttribute("NoCulling", true);
	}

	if (bNoShadow)
	{
		ObjectElem->SetAttribute("NoShadow", true);
	}

	// Save components to xml
	for (auto& Comp : SceneComponents)
	{
		tinyxml2::XMLElement* ComponentElem = ObjectElem->GetDocument()->NewElement("Component");
		ComponentElem->SetAttribute("Class", Comp->GetClassName());
		Comp->SaveComponentToXmlElement(ComponentElem);
		ObjectElem->InsertEndChild(ComponentElem);
	}
}

void RSceneObject::Destroy()
{
	assert(m_Scene);
	m_Scene->DestroyObject(this);
}

RTransform* RSceneObject::GetTransform()
{
	return &m_NodeTransform;
}

const RMatrix4& RSceneObject::GetTransformMatrix() const
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

	NotifyTransformModified();
}

void RSceneObject::SetRotation(const RQuat& quat)
{
	m_NodeTransform.SetRotation(quat);
	NotifyTransformModified();
}

void RSceneObject::SetPosition(const RVec3& pos)
{
	m_NodeTransform.SetPosition(pos);
	NotifyTransformModified();
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
	NotifyTransformModified();
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

RVec3 RSceneObject::GetVelocity() const
{
	return RVec3::Zero();
}

float RSceneObject::GetMotorAcceleration() const
{
	return 0.0f;
}

void RSceneObject::IncreaseInternalTransformUpdateCounter()
{
	InternalTransformUpdateCounter++;
}

void RSceneObject::DecreaseInternalTransformUpdateCounter()
{
	InternalTransformUpdateCounter--;
	assert(InternalTransformUpdateCounter >= 0);
}

const RAabb& RSceneObject::GetAabb()
{
	// If the bounding box has not been updated this frame, do it now.
	if (GEngine.GetFrameCounter() > BoundsUpdateFrame)
	{
		CalculateBounds();
	}

	return Bounds;
}

void RSceneObject::SetRenderPass(ERenderPass NewPass)
{
	RenderPass = NewPass;
}

ERenderPass RSceneObject::GetRenderPass() const
{
	return RenderPass;
}

void RSceneObject::DrawDebugShape() const
{
	OnDrawDebugShape();

	for (auto& Component : SceneComponents)
	{
		Component->DrawDebugShape();
	}
}

void RSceneObject::Update(float DeltaTime)
{
	if (m_NodeTransform.IsCacheDirty())
	{
		m_NodeTransform.NotifyChildrenMatricesChanged();
	}

	if (bTransformModified)
	{
		OnTransformModified();
		bTransformModified = false;
	}

	UpdateComponents(DeltaTime);
	CalculateBounds();
}

void RSceneObject::Update_PostPhysics(float DeltaTime)
{

}

RSceneComponent* RSceneObject::AddComponent(std::unique_ptr<RSceneComponent>&& Component)
{
	SceneComponents.push_back(std::move(Component));

	RSceneComponent* NewComponent = SceneComponents.back().get();
	NewComponent->NotifyComponentAdded();

	return NewComponent;
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

void RSceneObject::NotifyTransformModified()
{
	if (InternalTransformUpdateCounter == 0)
	{
		bTransformModified = true;
	}
}

void RSceneObject::OnTransformModified()
{
}

RScopeInternalTransformUpdate::RScopeInternalTransformUpdate(RSceneObject* InSceneObject)
	: SceneObject(InSceneObject)
{
	assert(SceneObject != nullptr);
	SceneObject->IncreaseInternalTransformUpdateCounter();
}

RScopeInternalTransformUpdate::~RScopeInternalTransformUpdate()
{
	SceneObject->DecreaseInternalTransformUpdateCounter();
}
