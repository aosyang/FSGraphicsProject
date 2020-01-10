//=============================================================================
// AIBehavior_Fighter.h by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "FTGPlayerController.h"

class RAINavigationComponent;

// AI combat logic class
class AIBehavior_Fighter : public RSceneComponent
{
	DECLARE_SCENE_COMPONENT(AIBehavior_Fighter, RSceneComponent);
public:
	AIBehavior_Fighter(RSceneObject* InOwner);

	/// Overrides RSceneComponent
	virtual void Update(float DeltaTime) override;

private:
	/// Order the AI to wait
	void Wait(float Seconds);

	/// Find a target to fight
	void FindAttackTarget();

	RVec3 EvaluateMovingVector();

	RVec3 GetVectorToAttackTarget() const;

	bool QueryPathToAttackTarget();

private:
	RAINavigationComponent* AINavigationComponent;

	FTGPlayerController* ControlledPlayer;
	FTGPlayerController* AttackTarget;

	bool bIsWaiting;
	float WaitTime;
	float AttackTargetTimeOut;

	std::vector<RVec3> NavPath;
	RVec3 NavigationStartPoint;
	RVec3 NavigationGoalPoint;
};