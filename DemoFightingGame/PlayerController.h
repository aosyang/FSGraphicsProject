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
	PlayerAnim_SpinAttack,

	PlayerAnimCount,
};

enum PlayerBehavior
{
	BHV_Idle,
	BHV_Running,
	BHV_SpinAttack,
};

class PlayerController : public RSMeshObject
{
public:
	PlayerController();

	void Cache();

	void PreUpdate(const RTimer& timer);
	const RVec3& GetRootOffset() const { return m_RootOffset; }
	void PostUpdate(const RTimer& timer);

	void SetBehavior(PlayerBehavior behavior);
	PlayerBehavior GetBehavior() const;

	bool IsPlayingLoopAnimation() const;

private:
	RAnimation* LoadAnimation(const char* resPath);

	RAnimation*		m_Animations[PlayerAnimCount];
	RAnimation*		m_CurrAnimation;
	float			m_CurrAnimTime;
	RVec3			m_RootOffset;

	PlayerBehavior	m_Behavior;
};

#endif
