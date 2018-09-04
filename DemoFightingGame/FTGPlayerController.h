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
	~FTGPlayerController();

	void InitAssets();

	void PreUpdate(const RTimer& timer);
	void UpdateMovement(const RTimer& timer, const RVec3 moveVec);
	void PostUpdate(const RTimer& timer);

	const RVec3& GetRootOffset() const;

	void Draw() override;
	void DrawDepthPass() override;

	/// Set the player's rotation in yaw angles
	void SetPlayerRotation(float rot) { m_Rotation = rot; }
	float GetPlayerRotation() const { return m_Rotation; }

	/// Set the player's rotation by a direction vector
	void SetPlayerFacing(const RVec3& Direction);

	void PerformPunch();
	void PerformKick();
	void PerformSpinAttack();

	void SetBehavior(EPlayerBehavior behavior);
	EPlayerBehavior GetBehavior() const;
	float GetBehaviorTime();

	RAabb GetMovementCollisionShape() const;
	RCapsule GetCollisionShape() const;

	/// Run a sphere shape hit test with other players
	vector<FTGPlayerController*> TestSphereHitWithOtherPlayers(float Radius, const RVec3& LocalSpaceOffset);

	/// Get the animation blender used for this player controller
	RAnimationBlender& GetAnimBlender() { return m_StateMachine.GetAnimBlender(); }

	/// Active player controller list
	static list<RSceneObject*>	ActivePlayerControllers;

	/// Whether to draw debug shape that represents player's hit
	static bool DrawDebugHitShape;

private:
	FTGPlayerController(RScene* InScene);

	/// Can the player move with user input
	bool CanMovePlayerWithInput() const;

	float					m_Rotation;
	RVec3					m_RootOffset;
	RMatrix4				m_BoneMatrices[MAX_BONE_COUNT];

	FTGPlayerStateMachine	m_StateMachine;
};


FORCEINLINE const RVec3& FTGPlayerController::GetRootOffset() const
{
	return m_RootOffset;
}

FORCEINLINE EPlayerBehavior FTGPlayerController::GetBehavior() const
{
	return m_StateMachine.GetCurrentBehavior();
}
