//=============================================================================
// PlayerControllerBase.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Rhino.h"

#include "FTGPlayerStateMachine.h"
#include "btBulletDynamicsCommon.h"
#include "BulletDynamics/Character/btKinematicCharacterController.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"

class PlayerControllerBase : public RSMeshObject
{
	DECLARE_SCENE_OBJECT(PlayerControllerBase, RSMeshObject);
public:
	~PlayerControllerBase();

	virtual void Update(float DeltaTime) override;

	/// Initialize assets used by the player controller
	void InitAssets(const std::string& MeshResourcePath);

	void UpdateController(float DeltaTime);

	void PreUpdate(float DeltaTime);
	void UpdateMovement(float DeltaTime, const RVec3 MoveVec);
	void PostUpdate(float DeltaTime);

	const RVec3& GetRootOffset() const;

	// Overrides RSceneObject render methods
	virtual void Draw() override;
	virtual void DrawDepthPass() override;

	void SetMovementInput(const RVec3& Input);

	/// Set the player's rotation in yaw angles
	void SetPlayerRotation(float rot) { m_Rotation = rot; }
	float GetPlayerRotation() const { return m_Rotation; }

	/// Set the player's rotation by a direction vector
	void SetPlayerFacing(const RVec3& Direction, bool bCheckMoveAllowed = true);

	void SetBehavior(EPlayerBehavior behavior);
	EPlayerBehavior GetBehavior() const;
	float GetBehaviorTime();

	RAabb GetMovementCollisionShape() const;
	RCapsule GetCollisionShape() const;

	void SetAnimationDeviation(float Deviation);

	/// Get the animation blender used for this player controller
	RAnimationBlender& GetAnimBlender() { return m_StateMachine.GetAnimBlender(); }

	FTGPlayerStateMachine& GetStateMachine() { return m_StateMachine; }

	/// Active player controller list
	static std::list<RSceneObject*>	ActivePlayerControllers;

	void Reset();

	RDelegate<> OnPlayerReset;

protected:
	PlayerControllerBase(const RConstructingParams& Params);

	virtual void OnTransformModified() override;

private:
	RVec3 GetHalfCapsuleOffset() const;

	/// Can the player move with user input
	bool CanMovePlayerWithInput() const;

	// Offset of stair which player can step up
	const RVec3				StairOffset;

	RVec3					m_MovementInput;

	float	CapsuleRadius;
	float	CapsuleHeight;

	std::unique_ptr<btPairCachingGhostObject> GhostObject;
	std::unique_ptr<btCapsuleShape> CapsuleShape;
	std::unique_ptr<btKinematicCharacterController> KinematicCharacterController;
	std::unique_ptr<btGhostPairCallback> GhostPairCallback;

	float					m_Rotation;
	RVec3					m_RootOffset;
	RMatrix4				m_BoneMatrices[MAX_BONE_COUNT];

	FTGPlayerStateMachine	m_StateMachine;
};

FORCEINLINE const RVec3& PlayerControllerBase::GetRootOffset() const
{
	return m_RootOffset;
}

FORCEINLINE EPlayerBehavior PlayerControllerBase::GetBehavior() const
{
	return m_StateMachine.GetCurrentBehavior();
}
