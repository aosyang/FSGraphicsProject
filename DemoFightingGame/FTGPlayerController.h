//=============================================================================
// FTGPlayerController.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Rhino.h"

#include "FTGPlayerStateMachine.h"
#include "FTGPlayerBehaviors.h"

/// Base fighting game player controller
class FTGPlayerController : public RSMeshObject
{
	DECLARE_SCENE_OBJECT(RSMeshObject);
public:
	void InitAssets();

	void PreUpdate(const RTimer& timer);
	const RVec3& GetRootOffset() const { return m_RootOffset; }
	void UpdateMovement(const RTimer& timer, const RVec3 moveVec);
	void PostUpdate(const RTimer& timer);

	void Draw() override;
	void DrawDepthPass() override;

	/// Set the player's rotation in yaw angles
	void SetPlayerRotation(float rot) { m_Rotation = rot; }
	float GetPlayerRotation() const { return m_Rotation; }

	/// Set the player's rotation by a direction vector
	void SetPlayerFacing(const RVec3& Direction);

	void SetBehavior(EPlayerBehavior behavior);
	EPlayerBehavior GetBehavior() const;
	float GetBehaviorTime();

	RAabb GetMovementCollisionShape() const;
	RCapsule GetCollisionShape() const;

	/// Get the animation blender used for this player controller
	RAnimationBlender& GetAnimBlender() { return m_StateMachine.GetAnimBlender(); }
private:
	FTGPlayerController(RScene* InScene);

	/// Can the player move with user input
	bool CanMovePlayerWithInput() const;

	float					m_Rotation;
	RVec3					m_RootOffset;
	RMatrix4				m_BoneMatrices[MAX_BONE_COUNT];

	FTGPlayerStateMachine	m_StateMachine;
};


FORCEINLINE EPlayerBehavior FTGPlayerController::GetBehavior() const
{
	return m_StateMachine.GetCurrentBehavior();
}
