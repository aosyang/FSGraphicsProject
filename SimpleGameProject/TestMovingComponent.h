//=============================================================================
// TestMovingComponent.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "Rhino.h"

class TestMovingComponent : public RSceneComponentBase
{
	typedef RSceneComponentBase Base;
public:
	static TestMovingComponent* Create(RSceneObject* InOwner);

	virtual void Update() override;

private:
	TestMovingComponent(RSceneObject* InOwner);
};
