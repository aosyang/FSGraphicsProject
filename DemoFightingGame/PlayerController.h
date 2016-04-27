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
	BHV_HitDown,
	BHV_GetUp,
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

	void SetBehavior(PlayerBehavior behavior);
	PlayerBehavior GetBehavior() const;

	bool IsPlayingLoopAnimation() const;
	RAabb GetMovementCollisionShape() const;
	RCapsule GetCollisionShape() const;

private:
	RAnimation* LoadAnimation(const char* resPath);

	float			m_Rotation;
	RAnimation*		m_Animations[PlayerAnimCount];
	RAnimation*		m_CurrAnimation;
	float			m_CurrAnimTime;
	RVec3			m_RootOffset;
	SHADER_SKINNED_BUFFER cbSkinned;

	PlayerBehavior	m_Behavior;
};

#endif
