//=============================================================================
// FTGPlayerBehaviors.h by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Rhino.h"

enum EPlayerBehavior
{
	BHV_Idle,
	BHV_Run,
	BHV_Punch,
	BHV_Kick,
	BHV_BackKick,
	BHV_SpinAttack,
	BHV_Hit,
	BHV_KnockedDown,
	BHV_GetUp,

	BHV_None,
};

class FTGPlayerController;
class FTGPlayerStateMachine;

class FTGPlayerBehaviorBase
{
public:
	FTGPlayerBehaviorBase();

	virtual bool EvaluateForExecution(FTGPlayerStateMachine* StateMachine);
	virtual void Update(FTGPlayerStateMachine* StateMachine, float DeltaTime);

	EPlayerBehavior GetBehaviorEnum() const;
	RAnimation* GetAnimation() const;
	float GetBlendInTime() const;
	bool DoesAllowRerunSelf() const;

	void NotifyBegin(FTGPlayerStateMachine* StateMachine);
	void NotifyEnd(FTGPlayerStateMachine* StateMachine);

protected:
	virtual void OnBehaviorFinished(FTGPlayerStateMachine* StateMachine);

	void LoadAnimationAsset(const char* AssetPath, int flags = 0);

	EPlayerBehavior m_BehaviorEnum;

	RAnimation*		m_Animation;

	float	m_BlendTime;

	bool	m_bAllowRerunSelf;
};

class FTGPlayerBehavior_Idle : public FTGPlayerBehaviorBase
{
public:
	FTGPlayerBehavior_Idle()
	{
		m_BehaviorEnum = BHV_Idle;
		m_BlendTime = 0.2f;
		LoadAnimationAsset("/unitychan/FUCM05_0000_Idle.fbx", AnimBitFlag_Loop);
	}
};

class FTGPlayerBehavior_Run : public FTGPlayerBehaviorBase
{
public:
	FTGPlayerBehavior_Run()
	{
		m_BehaviorEnum = BHV_Run;
		m_BlendTime = 0.2f;
		LoadAnimationAsset("/unitychan/FUCM_0012b_EH_RUN_LP_NoZ.fbx", AnimBitFlag_Loop);
	}
};

class FTGPlayerBehavior_Punch : public FTGPlayerBehaviorBase
{
public:
	FTGPlayerBehavior_Punch()
	{
		m_BehaviorEnum = BHV_Punch;
		m_BlendTime = 0.1f;
		LoadAnimationAsset("/unitychan/FUCM05_0001_M_CMN_LJAB.fbx", AnimBitFlag_HasRootMotion);
	}

	virtual void Update(FTGPlayerStateMachine* StateMachine, float DeltaTime) override;
};

class FTGPlayerBehavior_Kick : public FTGPlayerBehaviorBase
{
public:
	FTGPlayerBehavior_Kick()
	{
		m_BehaviorEnum = BHV_Kick;
		m_BlendTime = 0.1f;
		LoadAnimationAsset("/unitychan/FUCM_04_0001_RHiKick.fbx", AnimBitFlag_HasRootMotion);
	}

	virtual void Update(FTGPlayerStateMachine* StateMachine, float DeltaTime) override;
};

class FTGPlayerBehavior_BackKick : public FTGPlayerBehaviorBase
{
public:
	FTGPlayerBehavior_BackKick()
	{
		m_BehaviorEnum = BHV_BackKick;
		m_BlendTime = 0.1f;
		LoadAnimationAsset("/unitychan/FUCM02_0004_CH01_AS_MAWAK.fbx", AnimBitFlag_HasRootMotion);
	}

	virtual bool EvaluateForExecution(FTGPlayerStateMachine* StateMachine) override;
	virtual void Update(FTGPlayerStateMachine* StateMachine, float DeltaTime) override;
};

class FTGPlayerBehavior_SpinAttack : public FTGPlayerBehaviorBase
{
public:
	FTGPlayerBehavior_SpinAttack()
	{
		m_BehaviorEnum = BHV_SpinAttack;
		m_BlendTime = 0.2f;
		LoadAnimationAsset("/unitychan/FUCM02_0029_Cha01_STL01_ScrewK01.fbx", AnimBitFlag_HasRootMotion);
	}

	virtual void Update(FTGPlayerStateMachine* StateMachine, float DeltaTime) override;
};

class FTGPlayerBehavior_Hit : public FTGPlayerBehaviorBase
{
public:
	FTGPlayerBehavior_Hit()
	{
		m_BehaviorEnum = BHV_Hit;
		m_BlendTime = 0.1f;
		m_bAllowRerunSelf = true;
		LoadAnimationAsset("/unitychan/unitychan_DAMAGED00.fbx", AnimBitFlag_HasRootMotion);
	}
};

class FTGPlayerBehavior_KnockedDown : public FTGPlayerBehaviorBase
{
public:
	FTGPlayerBehavior_KnockedDown()
	{
		m_BehaviorEnum = BHV_KnockedDown;
		m_BlendTime = 0.1f;
		LoadAnimationAsset("/unitychan/FUCM02_0025_MYA_TF_DOWN.fbx", AnimBitFlag_HasRootMotion);
	}

protected:
	virtual void OnBehaviorFinished(FTGPlayerStateMachine* StateMachine) override;
};

class FTGPlayerBehavior_GetUp : public FTGPlayerBehaviorBase
{
public:
	FTGPlayerBehavior_GetUp()
	{
		m_BehaviorEnum = BHV_GetUp;
		m_BlendTime = 0.1f;
		LoadAnimationAsset("/unitychan/FUCM03_0019_HeadSpring.fbx", AnimBitFlag_HasRootMotion);
	}
};


FORCEINLINE EPlayerBehavior FTGPlayerBehaviorBase::GetBehaviorEnum() const
{
	return m_BehaviorEnum;
}

FORCEINLINE RAnimation* FTGPlayerBehaviorBase::GetAnimation() const
{
	return m_Animation;
}

FORCEINLINE float FTGPlayerBehaviorBase::GetBlendInTime() const
{
	return m_BlendTime;
}

FORCEINLINE bool FTGPlayerBehaviorBase::DoesAllowRerunSelf() const
{
	return m_bAllowRerunSelf;
}
