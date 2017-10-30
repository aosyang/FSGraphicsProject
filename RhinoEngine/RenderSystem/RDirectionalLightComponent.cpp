//=============================================================================
// RDirectionalLightComponent.cpp by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RDirectionalLightComponent.h"

RDirectionalLightComponent::RDirectionalLightComponent(RSceneObject* InOwner)
	: Base(InOwner),
	  m_Direction(0.0f, 1.0f, 0.0f),
	  m_Color(1.0f, 1.0f, 1.0f)
{
	GRenderer.RegisterLight(this);
}

RDirectionalLightComponent::~RDirectionalLightComponent()
{
	GRenderer.UnregisterLight(this);
}

RDirectionalLightComponent* RDirectionalLightComponent::Create(RSceneObject* InOwner)
{
	RDirectionalLightComponent* DirectionalLightComponent = new RDirectionalLightComponent(InOwner);
	return DirectionalLightComponent;
}

ELightType RDirectionalLightComponent::GetLightType()
{
	return ELightType::DirectionalLight;
}

void RDirectionalLightComponent::SetParameters(const DirectionalLightParam& Parameters)
{
	m_Direction = Parameters.Direction;
	m_Color = Parameters.Color;
}

