//=============================================================================
// RAINavigationComponent.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "RAINavigationComponent.h"

#include "NavigationSystem/RNavigationSystem.h"
#include "Scene/RSceneObject.h"

#include "RenderSystem/RDebugRenderer.h"

RAINavigationComponent::RAINavigationComponent(RSceneObject* InOwner)
	: Base(InOwner)
	, DesiredMoveDirection(0.0f, 0.0f, 0.0f)
	, ReachRadius(10.0f)
	, LastAgentPosition(RNavigationSystem::InvalidPosition)
	, TimeStuck(0.0f)
	, StuckCheckRadius(10.0f)
	, MaxTimeAllowedInStuck(1.0f)
	, bApproachingGoal(false)
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
			const RVec3 VectorToGoal = NavPath[0] - AIPosition;
			DesiredMoveDirection = VectorToGoal.GetNormalized2D();
			float MotorAcceleration = GetOwner()->GetMotorAcceleration();

			if (NavPath.size() == 1 && MotorAcceleration > 0.0f)
			{
				if (!bApproachingGoal)
				{
					// If we're moving toward the final point, slow down to approach the point
					const float DistanceToGoal = VectorToGoal.Magnitude2D();

					// Vt = V0 + at			: Vt = 0
					// t = (Vt - V0) / a
					float CurrentSpeed = GetOwner()->GetVelocity().Magnitude2D();
					float TimeToGoal = CurrentSpeed / MotorAcceleration;

					// S = V0 * t + 1/2 * a * t^2
					float DecelerationDistance = CurrentSpeed * TimeToGoal + 0.5f * -MotorAcceleration * TimeToGoal * TimeToGoal;

					if (DistanceToGoal <= DecelerationDistance)
					{
						bApproachingGoal = true;
					}
				}

				if (bApproachingGoal)
				{
					DesiredMoveDirection = RVec3::Zero();
				}
			}

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
					StopMovement();
					OnFinishedNavigation.Execute(EAINavResult::Failed);
					TimeStuck = 0.0f;
				}
			}
		}
		else
		{
			// No more path, stop now
			OnFinishedNavigation.Execute(EAINavResult::Succeeded);
			DesiredMoveDirection = RVec3::Zero();
		}
	}
}

bool RAINavigationComponent::RequestMoveTo(const RVec3& MoveTarget)
{
	bApproachingGoal = false;
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
