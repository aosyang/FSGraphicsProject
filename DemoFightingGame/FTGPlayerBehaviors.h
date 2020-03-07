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

	BHV_Navigation,
	BHV_Idle,
	BHV_Walk,
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


/// Declares the static GetBehaviorId method. Usage: PlayerBehavior::GetBehaviorId()
#define DECLARE_PLAYER_BEHAVIOR(Behavior) \
	public: \
		static size_t StaticClassId() { \
			static size_t BehaviorId = std::hash<std::string>{}(std::string(#Behavior)); \
			return BehaviorId; \
		} \
		virtual size_t GetBehaviorId() const { \
			return StaticClassId(); \
		} \
	private:


class FTGPlayerBehaviorBase
{
public:
	FTGPlayerBehaviorBase();
	FTGPlayerBehaviorBase(const std::string& AnimResourcePath, int AnimFlags = 0);
	virtual ~FTGPlayerBehaviorBase() = default;

	virtual size_t GetBehaviorId() const;

	/// Evaluate if a behavior can be executed
	virtual bool EvaluateForExecution(FTGPlayerStateMachine* StateMachine);
	virtual void Update(FTGPlayerStateMachine* StateMachine, float DeltaTime);

	virtual bool EvaluatePose(const RMesh& SkinnedMesh, RMatrix4* OutBoneMatrices);

	void CacheAssets(RMesh& SkinnedMesh);

	virtual std::string GetDebugString() const;

	EPlayerBehavior GetBehaviorEnum() const;
	RAnimation* GetAnimation() const;
	float GetBlendInTime() const;
	bool DoesAllowRerunSelf() const;

	void NotifyBegin(FTGPlayerStateMachine* StateMachine);
	void NotifyEnd(FTGPlayerStateMachine* StateMachine);

protected:
	virtual void OnCacheAnimations(RMesh& SkinnedMesh);

	virtual void OnBehaviorFinished(FTGPlayerStateMachine* StateMachine);

	void LoadAnimationAsset(const std::string& AssetPath, int flags = 0);

	EPlayerBehavior m_BehaviorEnum;

	RAnimation*		m_Animation;
	RAnimationPlayer AnimPlayer;

	float	m_BlendTime;

	bool	m_bAllowRerunSelf;
};

class PlayerBehavior_Idle : public FTGPlayerBehaviorBase
{
	DECLARE_PLAYER_BEHAVIOR(PlayerBehavior_Idle)
public:
	PlayerBehavior_Idle(const std::string& AnimResourcePath, int AnimFlags)
		: FTGPlayerBehaviorBase(AnimResourcePath, AnimFlags)
	{
		m_BehaviorEnum = BHV_Idle;
		m_BlendTime = 0.2f;
	}
};

class PlayerBehavior_Walk : public FTGPlayerBehaviorBase
{
	DECLARE_PLAYER_BEHAVIOR(PlayerBehavior_Walk)
public:
	PlayerBehavior_Walk(const std::string& AnimResourcePath, int AnimFlags)
		: FTGPlayerBehaviorBase(AnimResourcePath, AnimFlags)
	{
		m_BehaviorEnum = BHV_Walk;
		m_BlendTime = 0.2f;
	}
};

class PlayerBehavior_Run : public FTGPlayerBehaviorBase
{
	DECLARE_PLAYER_BEHAVIOR(PlayerBehavior_Run)
public:
	PlayerBehavior_Run(const std::string& AnimResourcePath, int AnimFlags)
		: FTGPlayerBehaviorBase(AnimResourcePath, AnimFlags)
	{
		m_BehaviorEnum = BHV_Run;
		m_BlendTime = 0.2f;
	}
};

class PlayerBehavior_Punch : public FTGPlayerBehaviorBase
{
	DECLARE_PLAYER_BEHAVIOR(PlayerBehavior_Punch)
public:
	PlayerBehavior_Punch(const std::string& AnimResourcePath, int AnimFlags)
		: FTGPlayerBehaviorBase(AnimResourcePath, AnimFlags)
	{
		m_BehaviorEnum = BHV_Punch;
		m_BlendTime = 0.1f;
	}

	virtual void Update(FTGPlayerStateMachine* StateMachine, float DeltaTime) override;
};

class PlayerBehavior_Kick : public FTGPlayerBehaviorBase
{
	DECLARE_PLAYER_BEHAVIOR(PlayerBehavior_Kick)
public:
	PlayerBehavior_Kick(const std::string& AnimResourcePath, int AnimFlags)
		: FTGPlayerBehaviorBase(AnimResourcePath, AnimFlags)
	{
		m_BehaviorEnum = BHV_Kick;
		m_BlendTime = 0.1f;
	}

	virtual void Update(FTGPlayerStateMachine* StateMachine, float DeltaTime) override;
};

class PlayerBehavior_BackKick : public FTGPlayerBehaviorBase
{
	DECLARE_PLAYER_BEHAVIOR(PlayerBehavior_BackKick)
public:
	PlayerBehavior_BackKick(const std::string& AnimResourcePath, int AnimFlags)
		: FTGPlayerBehaviorBase(AnimResourcePath, AnimFlags)
	{
		m_BehaviorEnum = BHV_BackKick;
		m_BlendTime = 0.1f;
	}

	virtual bool EvaluateForExecution(FTGPlayerStateMachine* StateMachine) override;
	virtual void Update(FTGPlayerStateMachine* StateMachine, float DeltaTime) override;
};

class PlayerBehavior_SpinAttack : public FTGPlayerBehaviorBase
{
	DECLARE_PLAYER_BEHAVIOR(PlayerBehavior_SpinAttack)
public:
	PlayerBehavior_SpinAttack(const std::string& AnimResourcePath, int AnimFlags)
		: FTGPlayerBehaviorBase(AnimResourcePath, AnimFlags)
	{
		m_BehaviorEnum = BHV_SpinAttack;
		m_BlendTime = 0.2f;
	}

	virtual void Update(FTGPlayerStateMachine* StateMachine, float DeltaTime) override;
};

class PlayerBehavior_Hit : public FTGPlayerBehaviorBase
{
	DECLARE_PLAYER_BEHAVIOR(PlayerBehavior_Hit)
public:
	PlayerBehavior_Hit(const std::string& AnimResourcePath, int AnimFlags)
		: FTGPlayerBehaviorBase(AnimResourcePath, AnimFlags)
	{
		m_BehaviorEnum = BHV_Hit;
		m_BlendTime = 0.1f;
		m_bAllowRerunSelf = true;
	}
};

class PlayerBehavior_KnockedDown : public FTGPlayerBehaviorBase
{
	DECLARE_PLAYER_BEHAVIOR(PlayerBehavior_KnockedDown)
public:
	PlayerBehavior_KnockedDown(const std::string& AnimResourcePath, int AnimFlags)
		: FTGPlayerBehaviorBase(AnimResourcePath, AnimFlags)
	{
		m_BehaviorEnum = BHV_KnockedDown;
		m_BlendTime = 0.1f;
	}

protected:
	virtual void OnBehaviorFinished(FTGPlayerStateMachine* StateMachine) override;
};

class PlayerBehavior_GetUp : public FTGPlayerBehaviorBase
{
	DECLARE_PLAYER_BEHAVIOR(PlayerBehavior_GetUp)
public:
	PlayerBehavior_GetUp(const std::string& AnimResourcePath, int AnimFlags)
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
