//=============================================================================
// AIBehavior_Fighter.cpp by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#include "AIBehavior_Fighter.h"

AIBehavior_Fighter::AIBehavior_Fighter(RSceneObject* InOwner)
	: Base(InOwner)
	, AttackTarget(nullptr)
	, bIsWaiting(false)
	, WaitTime(0.0f)
{
	ControlledPlayer = InOwner->CastTo<FTGPlayerController>();
}

void AIBehavior_Fighter::Update(float DeltaTime)
{
	if (ControlledPlayer)
	{
		if (bIsWaiting)
		{
			WaitTime -= DeltaTime;
			if (WaitTime <= 0.0f)
			{
				bIsWaiting = false;
			}

			return;
		}

		if (AttackTarget == nullptr)
		{
			FindAttackTarget();
		}

		// Still no target? Wait for a bit
		if (AttackTarget == nullptr)
		{
			Wait(5.0f);
			return;
		}

		RVec3 MoveVector = EvaluateMovingVector();
		RVec3 VecToTarget = GetVectorToAttackTarget();

		if (VecToTarget.SquaredMagitude() > RMath::Square(100.0f))
		{
			ControlledPlayer->SetMovementInput(MoveVector.GetNormalized() * 600.0f);
		}
		else
		{
			// AI has reached its attack target, dice roll and decide what to do
			static const std::vector<float> ActionWeights({ 3, 3, 1, 1 });
			//static const std::vector<float> ActionWeights({ 0, 0, 0, 1 });

			int ActionIndex = RMath::WeightedDiceRoll(ActionWeights);
			switch (ActionIndex)
			{
			case 0:
				ControlledPlayer->SetPlayerFacing(VecToTarget);
				ControlledPlayer->PerformPunch();
				Wait(0.2f);
				break;

			case 1:
				ControlledPlayer->SetPlayerFacing(VecToTarget);
				ControlledPlayer->PerformKick();
				Wait(0.5f);
				break;

			case 2:
				ControlledPlayer->SetPlayerFacing(VecToTarget);
				ControlledPlayer->PerformSpinAttack();
				Wait(0.5f);
				break;

			case 3:
				Wait(0.2f);
				break;
			}
		}

		AttackTargetTimeOut -= DeltaTime;
		if (AttackTargetTimeOut <= 0.0f)
		{
			AttackTarget = nullptr;
		}
	}
}

void AIBehavior_Fighter::Wait(float Seconds)
{
	bIsWaiting = true;
	WaitTime = Seconds;

	if (ControlledPlayer)
	{
		ControlledPlayer->SetMovementInput(RVec3::Zero());
	}
}

void AIBehavior_Fighter::FindAttackTarget()
{
	RScene* Scene = ControlledPlayer->GetScene();
	if (Scene)
	{
		auto PlayerControllers = Scene->FindAllObjectsOfType<FTGPlayerController>();
		int NumControllers = (int)PlayerControllers.size();

		// Note: If there is only one controller, it's likely the controlled player itself
		if (NumControllers > 1)
		{
			FTGPlayerController* Target = nullptr;

			do 
			{
				int Index = RMath::RandRangedInt(0, NumControllers - 1);
				Target = PlayerControllers[Index];
			} while (Target == ControlledPlayer);

			AttackTarget = Target;
			AttackTargetTimeOut = RMath::RandRangedF(10.0f, 15.0f);
			return;
		}
	}

	AttackTarget = nullptr;
}

RVec3 AIBehavior_Fighter::EvaluateMovingVector()
{
	RVec3 ResultVec;
	bool bDirectPathToGoal = false;

	// Query a path to the attack target and follow it
	if (QueryPathToAttackTarget())
	{
		const RVec3 PlayerPosition = ControlledPlayer->GetWorldPosition();

		if (RVec3::SquaredDistance2D(NavPath[0], PlayerPosition) < RMath::Square(10.0f))
		{
			NavPath.erase(NavPath.begin());
		}

		if (NavPath.size() > 0)
		{
			ResultVec = NavPath[0] - ControlledPlayer->GetWorldPosition();
		}
		else
		{
			bDirectPathToGoal = true;
		}
	}
	else
	{
		bDirectPathToGoal = true;
	}

	if (bDirectPathToGoal)
	{
		// If a path is not available, try move to the attack target directly
		ResultVec = AttackTarget->GetWorldPosition() - ControlledPlayer->GetWorldPosition();
	}

	ResultVec.SetY(0.0f);

	return ResultVec;
}

RVec3 AIBehavior_Fighter::GetVectorToAttackTarget() const
{
	RVec3 VecToTarget = AttackTarget->GetWorldPosition() - ControlledPlayer->GetWorldPosition();
	VecToTarget.SetY(0);

	return VecToTarget;
}

bool AIBehavior_Fighter::QueryPathToAttackTarget()
{
	if ((ControlledPlayer->GetWorldPosition() - NavigationStartPoint).SquaredMagitude() > RMath::Square(100.0f) ||
		(AttackTarget->GetWorldPosition() - NavigationGoalPoint).SquaredMagitude() > RMath::Square(100.0f))
	{
		if (GNavigationSystem.QueryPath(ControlledPlayer->GetWorldPosition(), AttackTarget->GetWorldPosition(), NavPath))
		{
			// Remove the starting point from path
			NavPath.erase(NavPath.begin());

			NavigationStartPoint = ControlledPlayer->GetWorldPosition();
			NavigationGoalPoint = AttackTarget->GetWorldPosition();

			return true;
		}

		return false;
	}

	return NavPath.size() >= 1;
}
