//=============================================================================
// RPointLightComponent.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Scene/RSceneComponent.h"
#include "ILight.h"

class RPointLightComponent : public RSceneComponent, public RLight
{
	DECLARE_SCENE_COMPONENT(RPointLightComponent, RSceneComponent)
public:
	virtual ~RPointLightComponent();

	virtual ELightType GetLightType() const override;
	virtual RAabb GetEffectiveLightBounds() override;
	virtual void SetupConstantBuffer(int LightIndex) const override;

	void SetRadius(float InRadius);
	float GetRadius() const;

private:
	RPointLightComponent(RSceneObject* InOwner);

private:
	float Radius;
};


FORCEINLINE void RPointLightComponent::SetRadius(float InRadius)
{
	Radius = InRadius;
}

FORCEINLINE float RPointLightComponent::GetRadius() const
{
	return Radius;
}
