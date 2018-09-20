//=============================================================================
// RSceneComponentBase.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "ISceneComponent.h"

class RSceneObject;

class RSceneComponentBase : public ISceneComponent
{
public:
	RSceneComponentBase(RSceneObject* InOwner);

	/// Get the scene object which is owning this component
	RSceneObject* GetOwner() const;

	virtual void Update() {}

private:
	/// The scene object owning this component
	RSceneObject*	OwnerSceneObject;
};

FORCEINLINE RSceneObject* RSceneComponentBase::GetOwner() const
{
	return OwnerSceneObject;
}
