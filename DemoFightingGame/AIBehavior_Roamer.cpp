//=============================================================================
// AIBehavior_Roamer.cpp by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#include "AIBehavior_Roamer.h"

#include "Navigation/RNavigationSystem.h"
#include "Navigation/RAINavigationComponent.h"

namespace
{
	const RAabb& GetStaticSceneBounds(RScene* Scene)
	{
		static RAabb SceneBounds;

		if (!SceneBounds.IsValid())
		{
			auto SceneObjects = Scene->EnumerateSceneObjects();
			for (auto& SceneObj : SceneObjects)
			{
				const RAabb& ObjectBounds = SceneObj->GetAabb();
				if (ObjectBounds.IsValid())
				{
					SceneBounds.Expand(ObjectBounds);
				}
			}
		}

		return SceneBounds;
	}
}

AIBehavior_Roamer::AIBehavior_Roamer(RSceneObject* InOwner)
	: Base(InOwner)
	, WaitDuration(0.0f)
{
	AINavigationComponent = InOwner->FindOrAddComponent<RAINavigationComponent>();
	AINavigationComponent->OnFinishedNavigation.Bind(this, &AIBehavior_Roamer::OnFinishedPathfinding);

	SceneBounds = GetStaticSceneBounds(InOwner->GetScene());
	ControlledPlayer = InOwner->CastTo<FTGPlayerController>();

	MoveTarget = RNavigationSystem::InvalidPosition;
}

void AIBehavior_Roamer::Update(float DeltaTime)
{
	if (WaitDuration > 0.0f)
	{
		WaitDuration -= DeltaTime;
		return;
	}

	if (AINavigationComponent->GetNavState() == EAINavState::Idle)
	{
		// Try finding a valid destination for pathfinding
		const int NumMaxAttempts = 10;
		for (int i = 0; i < NumMaxAttempts; i++)
		{
			MoveTarget = RVec3(
				RMath::RandRangedF(SceneBounds.pMin.X(), SceneBounds.pMax.X()),
				50.0f,
				RMath::RandRangedF(SceneBounds.pMin.Z(), SceneBounds.pMax.Z())
			);

			if (AINavigationComponent->RequestMoveTo(MoveTarget))
			{
				break;
			}
		}
	}

	if (AINavigationComponent->GetNavState() == EAINavState::Moving)
	{
		ControlledPlayer->SetMovementInput(AINavigationComponent->GetDesiredMoveDirection() * 600.0f);
		AINavigationComponent->DebugDrawPath();
	}
}

void AIBehavior_Roamer::Wait(float Duration)
{
	WaitDuration = Duration;
}

void AIBehavior_Roamer::OnFinishedPathfinding()
{
	ControlledPlayer->SetMovementInput(RVec3::Zero());

	// Randomly wait at the destination
	Wait(RMath::RandRangedF(1.5f, 3.5f));
}
