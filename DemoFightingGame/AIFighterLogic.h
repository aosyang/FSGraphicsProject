//=============================================================================
// AIFighterLogic.h by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "FTGPlayerController.h"

// AI combat logic class
class AIFighterLogic : public RSceneComponentBase
{
	DECLARE_SCENE_COMPONENT(AIFighterLogic, RSceneComponentBase);
public:
	AIFighterLogic(RSceneObject* InOwner);

	/// Overrides ISceneComponent
	virtual void Update(float DeltaTime) override;

private:
	/// Order the AI to wait
	void Wait(float Seconds);

	/// Find a target to fight
	void FindAttackTarget();

private:
	FTGPlayerController* ControlledPlayer;
	FTGPlayerController* AttackTarget;

	bool bIsWaiting;
	float WaitTime;
	float AttackTargetTimeOut;
};