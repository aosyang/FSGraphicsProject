//=============================================================================
// FTGPlayerStateMachine.h by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "FTGPlayerBehaviors.h"

class FTGPlayerController;

/// The behavior state machine of player
class FTGPlayerStateMachine
{
public:
	FTGPlayerStateMachine();
	~FTGPlayerStateMachine();

	/// Initialize behavior assets
	void Init(FTGPlayerController* Owner);

	/// Cache all animation assets for a mesh
	void CacheAnimations(RMesh* Mesh);

	/// Update the state machine
	void Update(float DeltaTime);

	/// Get the owner of this state machine
	FTGPlayerController* GetOwner() const;

	/// Get enum of current behavior
	EPlayerBehavior GetCurrentBehavior() const;

	/// Get time elapsed on current behavior
	float GetCurrentBehaviorTime() const;

	/// Set enum of next behavior
	void SetNextBehavior(EPlayerBehavior NextBehavior);

	/// Get enum of next behavior
	EPlayerBehavior GetNextBehavior() const;

	/// Get animation blender
	RAnimationBlender& GetAnimBlender();

	/// Notify the state machine about completion of current animation
	void NotifyAnimationFinished();

	/// Set the deviation of animation playback
	void SetAnimationDeviation(float Deviation);

	/// Get the deviation of animation playback
	float GetAnimationDeviation() const;

private:
	/// Allocate behavior instance asset
	template<typename T>
	void AllocateBehaviorInstance();

	/// Release behavior assets
	void ReleaseAssets();

	/// Find the behavior instance with behavior enum
	FTGPlayerBehaviorBase* FindBehaviorInstance(EPlayerBehavior Behavior) const;

private:
	/// The owning player of this state machine
	FTGPlayerController*			m_PlayerOwner;

	/// Current behavior instance
	FTGPlayerBehaviorBase*			m_CurrentBehaviorInstance;

	/// Next behavior enum
	EPlayerBehavior					m_NextBehavior;

	/// All behavior instances
	vector<FTGPlayerBehaviorBase*>	m_BehaviorInstances;

	/// The deviation of animation playback
	float							m_AnimSpeedDeviation;

	/// Animation blender
	RAnimationBlender				m_AnimBlender;
};


FORCEINLINE FTGPlayerController* FTGPlayerStateMachine::GetOwner() const
{
	return m_PlayerOwner;
}

FORCEINLINE EPlayerBehavior FTGPlayerStateMachine::GetCurrentBehavior() const
{
	return m_CurrentBehaviorInstance ? m_CurrentBehaviorInstance->GetBehaviorEnum() : BHV_None;
}

FORCEINLINE EPlayerBehavior FTGPlayerStateMachine::GetNextBehavior() const
{
	return m_NextBehavior;
}

FORCEINLINE RAnimationBlender& FTGPlayerStateMachine::GetAnimBlender()
{
	return m_AnimBlender;
}

template<typename T>
void FTGPlayerStateMachine::AllocateBehaviorInstance()
{
	m_BehaviorInstances.push_back(new T);
}
