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

private:
	/// The scene object owning this component
	RSceneObject*	OwnerSceneObject;
};