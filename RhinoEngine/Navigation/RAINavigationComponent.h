//=============================================================================
// RAINavigationComponent.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Scene/RSceneComponent.h"

enum class EAINavState : UINT8
{
	Idle,
	Moving,
};

enum class EAINavResult
{
	Succeeded,
	Failed,
};

class RAINavigationComponent : public RSceneComponent
{
	DECLARE_SCENE_COMPONENT(RAINavigationComponent, RSceneComponent);
public:
	RAINavigationComponent(RSceneObject* InOwner);

	virtual void Update(float DeltaTime) override;

	/// Query a new path for the navigation component
	/// Returns true if query succeeds, otherwise false.
	bool RequestMoveTo(const RVec3& MoveTarget);

	/// Stop any movements AI nav component current has and clear the nav path
	void StopMovement();

	/// Get the state of current navigation
	EAINavState GetNavState() const;

	const RVec3& GetDesiredMoveDirection() const;

	/// Debug draw the navigation path of AI
	void DebugDrawPath() const;

	/// Delegate called when AI has arrived at the goal
	RDelegate<EAINavResult> OnFinishedNavigation;

private:
	std::vector<RVec3>	NavPath;
	RVec3				DesiredMoveDirection;
	float				ReachRadius;

	RVec3				LastAgentPosition;
	float				TimeStuck;
	float				StuckCheckRadius;
	float				MaxTimeAllowedInStuck;
};

FORCEINLINE const RVec3& RAINavigationComponent::GetDesiredMoveDirection() const
{
	return DesiredMoveDirection;
}
