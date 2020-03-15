//=============================================================================
// RPointLightComponent.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RPointLightComponent.h"

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
