//=============================================================================
// AIBehavior_Roamer.h by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "FTGPlayerController.h"
#include "Navigation/RAINavigationComponent.h"

class RAINavigationComponent;

class AIBehavior_Roamer : public RSceneComponent
{
	DECLARE_SCENE_COMPONENT(AIBehavior_Roamer, RSceneComponent);
public:
	AIBehavior_Roamer(RSceneObject* InOwner);

	virtual void Update(float DeltaTime) override;

private:
	void Wait(float Duration);

	void OnFinishedPathfinding(EAINavResult Result);

private:
	RAINavigationComponent* AINavigationComponent;

	RAabb SceneBounds;
	FTGPlayerController* ControlledPlayer;
	RVec3 MoveTarget;

	/// Speed for current move
	float MoveSpeed;

	/// Remaining wait time before making another path-finding decision
	float WaitDuration;
};
