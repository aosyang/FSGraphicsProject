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

const RAabb& RSceneComponent::GetLocalAabb() const
{
	return LocalBounds;
}
