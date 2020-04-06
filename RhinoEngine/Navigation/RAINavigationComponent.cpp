//=============================================================================
// RAINavigationComponent.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "RAINavigationComponent.h"

#include "RNavigationSystem.h"
#include "Scene/RSceneObject.h"

#include "RenderSystem/RDebugRenderer.h"

RAINavigationComponent::RAINavigationComponent(RSceneObject* InOwner)
	: Base(InOwner)
	, DesiredMoveDirection(0.0f, 0.0f, 0.0f)
	, ReachRadius(10.0f)
	, LastAgentPosition(RNavigationSystem::InvalidPosition)
	, TimeStuck(0.0f)
	, StuckCheckRadius(50.0f)
	, MaxTimeAllowedInStuck(1.0f)
{

}

void RAINavigationComponent::Update(float DeltaTime)
{
	if (NavPath.size() != 0)
	{
		const RVec3 AIPosition = GetOwner()->GetWorldPosition();

		// Once agent arrives, remove a path node from the path array
		if (RVec3::SquaredDistance2D(NavPath[0], AIPosition) < RMath::Square(ReachRadius))
		{
			NavPath.erase(NavPath.begin());
		}

		if (NavPath.size() > 0)
		{
			RVec3 MoveVector = NavPath[0] - AIPosition;
			DesiredMoveDirection = MoveVector.GetNormalized2D();

			if (RVec3::SquaredDistance(LastAgentPosition, AIPosition) > RMath::Square(StuckCheckRadius))
			{
				LastAgentPosition = AIPosition;
				TimeStuck = 0.0f;
			}
			else
			{
				TimeStuck += DeltaTime;
				if (TimeStuck > MaxTimeAllowedInStuck)
				{
					// Agent is stuck. Invalidate the path and finish current path-finding
					NavPath.clear();
					OnFinishedNavigation.Execute(EAINavResult::Failed);
					TimeStuck = 0.0f;
				}
			}
		}
		else
		{
			OnFinishedNavigation.Execute(EAINavResult::Succeeded);
			DesiredMoveDirection = RVec3::Zero();
		}
	}
}

bool RAINavigationComponent::RequestMoveTo(const RVec3& MoveTarget)
{
	return GNavigationSystem.QueryPath(GetOwner()->GetWorldPosition(), MoveTarget, NavPath);
}

void RAINavigationComponent::StopMovement()
{
	NavPath.clear();
	DesiredMoveDirection = RVec3::Zero();
}

EAINavState RAINavigationComponent::GetNavState() const
{
	if (NavPath.size() != 0)
	{
		return EAINavState::Moving;
	}

	return EAINavState::Idle;
}

void RAINavigationComponent::DebugDrawPath() const
{
	if (NavPath.size() > 0)
	{
		GDebugRenderer.DrawLine(GetOwner()->GetWorldPosition(), NavPath[0], RColor::Cyan);
	}

	for (int i = 0; i < (int)NavPath.size() - 1; i++)
	{
		GDebugRenderer.DrawLine(NavPath[i], NavPath[i + 1], RColor::Cyan);
	}
}
