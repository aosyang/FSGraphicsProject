//=============================================================================
// AIBehavior_Roamer.cpp by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#include "AIBehavior_Roamer.h"

#include "Navigation/RNavigationSystem.h"

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
{
	SceneBounds = GetStaticSceneBounds(InOwner->GetScene());
	ControlledPlayer = InOwner->CastTo<FTGPlayerController>();

	MoveTarget = RNavigationSystem::InvalidPosition;

	// When reseting the position of a player, also clear its current nav path.
	ControlledPlayer->OnPlayerReset.BindLambda([&]()
	{
		NavPath.clear();
	});
}

void AIBehavior_Roamer::Update(float DeltaTime)
{
	if (NavPath.size() == 0)
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

			if (GNavigationSystem.QueryPath(ControlledPlayer->GetWorldPosition(), MoveTarget, NavPath))
			{
				break;
			}
		}
	}

	if (NavPath.size() != 0)
	{
		const RVec3 PlayerPosition = ControlledPlayer->GetWorldPosition();

		if (RVec3::SquaredDistance2D(NavPath[0], PlayerPosition) < RMath::Square(5.0f))
		{
			NavPath.erase(NavPath.begin());
		}

		if (NavPath.size() > 0)
		{
			RVec3 MoveVector = NavPath[0] - PlayerPosition;
			MoveVector.SetY(0);

			ControlledPlayer->SetMovementInput(MoveVector.GetNormalized() * 600.0f);
		}
		else
		{
			ControlledPlayer->SetMovementInput(RVec3::Zero());
		}

		DebugDrawPath();
	}
}

void AIBehavior_Roamer::DebugDrawPath() const
{
	if (NavPath.size() > 0)
	{
		GDebugRenderer.DrawLine(ControlledPlayer->GetWorldPosition(), NavPath[0], RColor::Cyan);
	}

	for (int i = 0; i < (int)NavPath.size() - 1; i++)
	{
		GDebugRenderer.DrawLine(NavPath[i], NavPath[i + 1], RColor::Cyan);
	}
}
