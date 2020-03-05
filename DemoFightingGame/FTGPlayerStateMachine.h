//=============================================================================
// FTGPlayerStateMachine.h by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "FTGPlayerBehaviors.h"
#include "RAnimBlendQueue.h"

class PlayerControllerBase;


/// The behavior state machine of player
class FTGPlayerStateMachine
{
public:
	FTGPlayerStateMachine(PlayerControllerBase* InPlayerOwner);
	~FTGPlayerStateMachine();

	/// Allocate behavior instance asset
	template<typename T, typename ...Args>
	T* AllocateBehaviorInstance(Args&& ...args);

	/// Initialize behavior assets
	void InitAssets();

	/// Update the state machine
	void Update(float DeltaTime);

	/// Evaluate pose for skeletal mesh at current state
	bool EvaluatePose(const RMesh& SkinnedMesh, RMatrix4* OutBoneMatrices);

	RVec3 GetCurrentRootOffset() const;

	/// Get debug string for the state machine
	std::string GetDebugString() const;

	/// Get the owner of this state machine
	PlayerControllerBase* GetOwner() const;

	/// Get enum of current behavior
	FTGPlayerBehaviorBase* GetCurrentBehavior() const;

	size_t GetCurrentBehaviorId() const;

	/// Get time elapsed on current behavior
	float GetCurrentBehaviorTime() const;

	/// Set enum of next behavior
	void SetNextBehavior(EPlayerBehavior NextBehavior);

	/// Get enum of next behavior
	FTGPlayerBehaviorBase* GetNextBehavior() const;

	/// Notify the state machine about completion of current animation
	void NotifyAnimationFinished();

	/// Set the deviation of animation playback
	void SetAnimationDeviation(float Deviation);

	/// Get the deviation of animation playback
	float GetAnimationDeviation() const;

private:
	/// Cache all animation assets for a mesh
	void CacheAnimations(RMesh* Mesh);

	/// Find the behavior instance with behavior enum
	FTGPlayerBehaviorBase* FindBehaviorInstance(EPlayerBehavior Behavior) const;

private:
	/// The owning player of this state machine
	PlayerControllerBase*			m_PlayerOwner;

	/// Current behavior instance
	FTGPlayerBehaviorBase*			m_CurrentBehaviorInstance;

	/// Next behavior
	FTGPlayerBehaviorBase*			m_NextBehavior;

	/// All behavior instances
	std::vector<std::unique_ptr<FTGPlayerBehaviorBase>>	m_BehaviorInstances;

	/// The deviation of animation playback
	float							m_AnimSpeedDeviation;

	/// Animation blender
	RAnimationBlender				m_AnimBlender;

	RAnimBlendQueue					BlendQueue;
};


FORCEINLINE PlayerControllerBase* FTGPlayerStateMachine::GetOwner() const
{
	return m_PlayerOwner;
}

FORCEINLINE FTGPlayerBehaviorBase* FTGPlayerStateMachine::GetCurrentBehavior() const
{
	return m_CurrentBehaviorInstance;
}

FORCEINLINE size_t FTGPlayerStateMachine::GetCurrentBehaviorId() const
{
	if (m_CurrentBehaviorInstance)
	{
		return m_CurrentBehaviorInstance->GetBehaviorId();
	}

	return 0;
}

FORCEINLINE FTGPlayerBehaviorBase* FTGPlayerStateMachine::GetNextBehavior() const
{
	return m_NextBehavior;
}

template<typename T, typename ...Args>
T* FTGPlayerStateMachine::AllocateBehaviorInstance(Args&& ...args)
{
	m_BehaviorInstances.push_back(std::make_unique<T>(std::forward<Args>(args)...));
	return static_cast<T*>(m_BehaviorInstances.back().get());
}
