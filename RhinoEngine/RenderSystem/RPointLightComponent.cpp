//=============================================================================
// RPointLightComponent.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "RPointLightComponent.h"

#include "RRenderSystem.h"
#include "RShaderConstantBuffer.h"
#include "RDebugRenderer.h"

#include "Scene/RSceneObject.h"

#include "tinyxml2/tinyxml2.h"

IMPLEMENT_SCENE_COMPONENT(RPointLightComponent);

RPointLightComponent::RPointLightComponent(RSceneObject* InOwner)
	: Base(InOwner)
	, Radius(100.0f)
{
	GRenderer.RegisterLight(this);
	LocalBounds.ExpandBySphere(RVec3::Zero(), 10.0f);
}

RPointLightComponent::~RPointLightComponent()
{
	GRenderer.UnregisterLight(this);
}

ELightType RPointLightComponent::GetLightType() const
{
	return ELightType::PointLight;
}

RAabb RPointLightComponent::GetEffectiveLightBounds()
{
	RAabb Bounds;
	Bounds.ExpandBySphere(GetOwner()->GetWorldPosition(), Radius);
	return Bounds;
}

void RPointLightComponent::SetupConstantBuffer(int LightIndex) const
{
	POINT_LIGHT& PointLightData = RConstantBuffers::cbLight.Data.PointLight[LightIndex];
	PointLightData.PosAndRadius = RVec4(GetOwner()->GetWorldPosition(), Radius);

	const RColor& LightColor = GetLightColor();
	PointLightData.Color = RVec4(LightColor.r, LightColor.g, LightColor.b, GetLightIntensity());
}

void RPointLightComponent::LoadComponentFromXmlElement(tinyxml2::XMLElement* ComponentElem)
{
	Base::LoadComponentFromXmlElement(ComponentElem);
	RLight::LoadFromXmlElement(ComponentElem);
	ComponentElem->QueryFloatAttribute("Radius", &Radius);
}

void RPointLightComponent::SaveComponentToXmlElement(tinyxml2::XMLElement* ComponentElem) const
{
	Base::SaveComponentToXmlElement(ComponentElem);
	RLight::SaveToXmlElement(ComponentElem);
	ComponentElem->SetAttribute("Radius", Radius);
}

void RPointLightComponent::OnDrawDebugShape() const
{
	if (GEngine.IsEditor())
	{
		GDebugRenderer.DrawSphere(GetOwner()->GetWorldPosition(), Radius, RColor::Yellow);
	}
}
