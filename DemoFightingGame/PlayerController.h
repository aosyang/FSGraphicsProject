//=============================================================================
// PlayerController.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#ifndef _PLAYERCONTROLLER_H
#define _PLAYERCONTROLLER_H

#include "Rhino.h"

enum PlayerAnimation
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

enum PlayerBehavior
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
	PlayerAnimation anim;
	float			blendTime;
};

class PlayerController : public RSMeshObject
{
public:
	PlayerController();

	void Cache();

	void PreUpdate(const RTimer& timer);
	const RVec3& GetRootOffset() const { return m_RootOffset; }
	void UpdateMovement(const RTimer& timer, const RVec3 moveVec);
	void PostUpdate(const RTimer& timer);

	void Draw();
	void DrawDepthPass();

	void SetPlayerRotation(float rot) { m_Rotation = rot; }
	float GetPlayerRotation() const { return m_Rotation; }

	void SetBehavior(PlayerBehavior behavior);
	PlayerBehavior GetBehavior() const;
	float GetBehaviorTime();

	RAabb GetMovementCollisionShape() const;
	RCapsule GetCollisionShape() const;

	RAnimationBlender& GetAnimationBlender() { return m_AnimBlender; }
private:
	RAnimation* LoadAnimation(const char* resPath, int flags=0);

	float					m_Rotation;
	RAnimation*				m_Animations[PlayerAnimCount];
	RAnimationBlender		m_AnimBlender;
	RVec3					m_RootOffset;
	RMatrix4				m_BoneMatrices[MAX_BONE_COUNT];

	PlayerBehavior			m_Behavior;
};

#endif
