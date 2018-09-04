//=============================================================================
// AIFighterLogic.h by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "FTGPlayerController.h"

class AIFighterLogic
{
public:
	AIFighterLogic(FTGPlayerController* PlayerController);

	void Update(float DeltaTime);

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