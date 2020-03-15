//=============================================================================
// RSceneComponentBase.cpp by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RSceneComponent.h"

RSceneComponent::RSceneComponent(RSceneObject* InOwner)
	: OwnerSceneObject(InOwner)
{

}

void RSceneComponent::NotifyComponentAdded()
{
	OnComponentAdded();
}

void RSceneComponent::LoadComponentFromXmlElement(tinyxml2::XMLElement* ComponentElem)
{

}

void RSceneComponent::SaveComponentToXmlElement(tinyxml2::XMLElement* ComponentElem) const
{

}

const RAabb& RSceneComponent::GetLocalAabb() const
{
	return LocalBounds;
}
