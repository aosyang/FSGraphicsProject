//=============================================================================
// AIFighterLogic.cpp by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#include "AIFighterLogic.h"

AIFighterLogic::AIFighterLogic(FTGPlayerController* PlayerController)
	: ControlledPlayer(PlayerController)
	, AttackTarget(nullptr)
	, bIsWaiting(false)
	, WaitTime(0.0f)
{

}

void AIFighterLogic::Update(float DeltaTime)
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

		RVec3 ToTarget = AttackTarget->GetWorldPosition() - ControlledPlayer->GetWorldPosition();
		ToTarget.SetY(0.0f);

		if (ToTarget.SquaredMagitude() > Math::Square(100.0f))
		{
			ControlledPlayer->SetMovementInput(ToTarget.GetNormalized() * 600.0f);
		}
		else
		{
			// AI has reached its attack target, dice roll and decide what to do

			int ActionIndex = Math::RandRangedInt(0, 7);
			switch (ActionIndex)
			{
			case 0:
			case 1:
			case 2:
				ControlledPlayer->SetPlayerFacing(ToTarget);
				ControlledPlayer->PerformPunch();
				Wait(0.2f);
				break;

			case 3:
			case 4:
			case 5:
				ControlledPlayer->SetPlayerFacing(ToTarget);
				ControlledPlayer->PerformKick();
				Wait(0.5f);
				break;

			case 6:
				ControlledPlayer->SetPlayerFacing(ToTarget);
				ControlledPlayer->PerformSpinAttack();
				Wait(0.5f);
				break;

			case 7:
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

void AIFighterLogic::Wait(float Seconds)
{
	bIsWaiting = true;
	WaitTime = Seconds;

	if (ControlledPlayer)
	{
		ControlledPlayer->SetMovementInput(RVec3::Zero());
	}
}

void AIFighterLogic::FindAttackTarget()
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
				int Index = Math::RandRangedInt(0, NumControllers - 1);
				Target = PlayerControllers[Index];
			} while (Target == ControlledPlayer);

			AttackTarget = Target;
			AttackTargetTimeOut = Math::RandRangedF(10.0f, 15.0f);
			return;
		}
	}

	AttackTarget = nullptr;
}
