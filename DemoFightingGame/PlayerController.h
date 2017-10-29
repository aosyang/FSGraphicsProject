//=============================================================================
// PlayerController.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#ifndef _PLAYERCONTROLLER_H
#define _PLAYERCONTROLLER_H

#include "Rhino.h"

enum EPlayerAnimation
{
	PlayerAnim_Idle,
	PlayerAnim_Run,
	PlayerAnim_Punch1,
	PlayerAnim_Kick,
	PlayerAnim_BackKick,
	PlayerAnim_SpinAttack,

	PlayerAnim_Hit,
	PlayerAnim_Down,
	PlayerAnim_GetUp,

	PlayerAnimCount,
};

enum EPlayerBehavior
{
	BHV_Idle,
	BHV_Running,
	BHV_Punch,
	BHV_Kick,
	BHV_BackKick,
	BHV_SpinAttack,
	BHV_Hit,
	BHV_HitDown,
	BHV_GetUp,
};

struct BehaviorInfo
{
	EPlayerAnimation	anim;
	float				blendTime;
};

class PlayerController : public RSMeshObject
{
	DECLARE_SCENE_OBJECT(RSMeshObject);
public:
	void Cache();

	void PreUpdate(const RTimer& timer);
	const RVec3& GetRootOffset() const { return m_RootOffset; }
	void UpdateMovement(const RTimer& timer, const RVec3 moveVec);
	void PostUpdate(const RTimer& timer);

	void Draw() override;
	void DrawDepthPass() override;

	void SetPlayerRotation(float rot) { m_Rotation = rot; }
	float GetPlayerRotation() const { return m_Rotation; }

	void SetBehavior(EPlayerBehavior behavior);
	EPlayerBehavior GetBehavior() const;
	float GetBehaviorTime();

	RAabb GetMovementCollisionShape() const;
	RCapsule GetCollisionShape() const;

	RAnimationBlender& GetAnimationBlender() { return m_AnimBlender; }
private:
	PlayerController(RScene* InScene);

	RAnimation* LoadAnimation(const char* resPath, int flags=0);

	/// Can the player move with user input
	bool CanMovePlayerWithInput() const;

	float					m_Rotation;
	RAnimation*				m_Animations[PlayerAnimCount];
	RAnimationBlender		m_AnimBlender;
	RVec3					m_RootOffset;
	RMatrix4				m_BoneMatrices[MAX_BONE_COUNT];

	EPlayerBehavior			m_Behavior;
};


FORCEINLINE EPlayerBehavior PlayerController::GetBehavior() const
{
	return m_Behavior;
}


#endif
