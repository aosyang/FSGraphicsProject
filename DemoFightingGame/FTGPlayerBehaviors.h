//=============================================================================
// FTGPlayerBehaviors.h by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Rhino.h"

enum EPlayerBehavior
{
	BHV_None,

	BHV_Idle,
	BHV_Run,
	BHV_Punch,
	BHV_Kick,
	BHV_BackKick,
	BHV_SpinAttack,
	BHV_Hit,
	BHV_KnockedDown,
	BHV_GetUp,
};

class FTGPlayerController;
class FTGPlayerStateMachine;

class FTGPlayerBehaviorBase
{
public:
	FTGPlayerBehaviorBase();
	FTGPlayerBehaviorBase(const std::string& AnimResourcePath, int AnimFlags = 0);

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

	void LoadAnimationAsset(const std::string& AssetPath, int flags = 0);

	EPlayerBehavior m_BehaviorEnum;

	RAnimation*		m_Animation;

	float	m_BlendTime;

	bool	m_bAllowRerunSelf;
};

class FTGPlayerBehavior_Idle : public FTGPlayerBehaviorBase
{
public:
	FTGPlayerBehavior_Idle(const std::string& AnimResourcePath, int AnimFlags)
		: FTGPlayerBehaviorBase(AnimResourcePath, AnimFlags)
	{
		m_BehaviorEnum = BHV_Idle;
		m_BlendTime = 0.2f;
	}
};

class FTGPlayerBehavior_Run : public FTGPlayerBehaviorBase
{
public:
	FTGPlayerBehavior_Run(const std::string& AnimResourcePath, int AnimFlags)
		: FTGPlayerBehaviorBase(AnimResourcePath, AnimFlags)
	{
		m_BehaviorEnum = BHV_Run;
		m_BlendTime = 0.2f;
	}
};

class FTGPlayerBehavior_Punch : public FTGPlayerBehaviorBase
{
public:
	FTGPlayerBehavior_Punch(const std::string& AnimResourcePath, int AnimFlags)
		: FTGPlayerBehaviorBase(AnimResourcePath, AnimFlags)
	{
		m_BehaviorEnum = BHV_Punch;
		m_BlendTime = 0.1f;
	}

	virtual void Update(FTGPlayerStateMachine* StateMachine, float DeltaTime) override;
};

class FTGPlayerBehavior_Kick : public FTGPlayerBehaviorBase
{
public:
	FTGPlayerBehavior_Kick(const std::string& AnimResourcePath, int AnimFlags)
		: FTGPlayerBehaviorBase(AnimResourcePath, AnimFlags)
	{
		m_BehaviorEnum = BHV_Kick;
		m_BlendTime = 0.1f;
	}

	virtual void Update(FTGPlayerStateMachine* StateMachine, float DeltaTime) override;
};

class FTGPlayerBehavior_BackKick : public FTGPlayerBehaviorBase
{
public:
	FTGPlayerBehavior_BackKick(const std::string& AnimResourcePath, int AnimFlags)
		: FTGPlayerBehaviorBase(AnimResourcePath, AnimFlags)
	{
		m_BehaviorEnum = BHV_BackKick;
		m_BlendTime = 0.1f;
	}

	virtual bool EvaluateForExecution(FTGPlayerStateMachine* StateMachine) override;
	virtual void Update(FTGPlayerStateMachine* StateMachine, float DeltaTime) override;
};

class FTGPlayerBehavior_SpinAttack : public FTGPlayerBehaviorBase
{
public:
	FTGPlayerBehavior_SpinAttack(const std::string& AnimResourcePath, int AnimFlags)
		: FTGPlayerBehaviorBase(AnimResourcePath, AnimFlags)
	{
		m_BehaviorEnum = BHV_SpinAttack;
		m_BlendTime = 0.2f;
	}

	virtual void Update(FTGPlayerStateMachine* StateMachine, float DeltaTime) override;
};

class FTGPlayerBehavior_Hit : public FTGPlayerBehaviorBase
{
public:
	FTGPlayerBehavior_Hit(const std::string& AnimResourcePath, int AnimFlags)
		: FTGPlayerBehaviorBase(AnimResourcePath, AnimFlags)
	{
		m_BehaviorEnum = BHV_Hit;
		m_BlendTime = 0.1f;
		m_bAllowRerunSelf = true;
	}
};

class FTGPlayerBehavior_KnockedDown : public FTGPlayerBehaviorBase
{
public:
	FTGPlayerBehavior_KnockedDown(const std::string& AnimResourcePath, int AnimFlags)
		: FTGPlayerBehaviorBase(AnimResourcePath, AnimFlags)
	{
		m_BehaviorEnum = BHV_KnockedDown;
		m_BlendTime = 0.1f;
	}

protected:
	virtual void OnBehaviorFinished(FTGPlayerStateMachine* StateMachine) override;
};

class FTGPlayerBehavior_GetUp : public FTGPlayerBehaviorBase
{
public:
	FTGPlayerBehavior_GetUp(const std::string& AnimResourcePath, int AnimFlags)
		: FTGPlayerBehaviorBase(AnimResourcePath, AnimFlags)
	{
		m_BehaviorEnum = BHV_GetUp;
		m_BlendTime = 0.1f;
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
