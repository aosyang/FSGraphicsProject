//=============================================================================
// AIBehavior_Roamer.cpp by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#include "AIBehavior_Roamer.h"

namespace
{
	const static RVec3 InvalidPosition(FLT_MAX, FLT_MAX, FLT_MAX);

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

	MoveTarget = InvalidPosition;

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
		bool bQueryPathSucceeded = false;

		do
		{
			MoveTarget = RVec3(
				RMath::RandRangedF(SceneBounds.pMin.X(), SceneBounds.pMax.X()),
				50.0f,
				RMath::RandRangedF(SceneBounds.pMin.Z(), SceneBounds.pMax.Z())
			);

			bQueryPathSucceeded = GNavigationSystem.QueryPath(ControlledPlayer->GetWorldPosition(), MoveTarget, NavPath);
		} while (!bQueryPathSucceeded);
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
	}
}
