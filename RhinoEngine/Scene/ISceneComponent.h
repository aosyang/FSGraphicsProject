//=============================================================================
// ISceneComponent.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

/// Component interface
class ISceneComponent
{
public:
	virtual ~ISceneComponent() {}

	virtual void Update(float DeltaTime) = 0;
};