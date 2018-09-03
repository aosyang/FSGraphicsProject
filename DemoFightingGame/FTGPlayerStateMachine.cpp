//=============================================================================
// FTGPlayerStateMachine.cpp by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#include "FTGPlayerStateMachine.h"

FTGPlayerStateMachine::FTGPlayerStateMachine()
	: m_NextBehavior(BHV_Invalid)
	, m_CurrentBehaviorInstance(nullptr)
{
}

FTGPlayerStateMachine::~FTGPlayerStateMachine()
{
	ReleaseAssets();
}

void FTGPlayerStateMachine::InitAssets()
{
	AllocateBehaviorInstance<FTGPlayerBehavior_Idle>();
	AllocateBehaviorInstance<FTGPlayerBehavior_Run>();
	AllocateBehaviorInstance<FTGPlayerBehavior_Punch>();
	AllocateBehaviorInstance<FTGPlayerBehavior_Kick>();
	AllocateBehaviorInstance<FTGPlayerBehavior_BackKick>();
	AllocateBehaviorInstance<FTGPlayerBehavior_SpinAttack>();
	AllocateBehaviorInstance<FTGPlayerBehavior_Hit>();
	AllocateBehaviorInstance<FTGPlayerBehavior_KnockedDown>();
	AllocateBehaviorInstance<FTGPlayerBehavior_GetUp>();

	m_CurrentBehaviorInstance = FindBehaviorInstance(BHV_Idle);
	m_AnimBlender.Play(m_CurrentBehaviorInstance->GetAnimation());
}

void FTGPlayerStateMachine::Update(float DeltaTime)
{
	m_AnimBlender.ProceedAnimation(DeltaTime);

	if (m_AnimBlender.HasFinishedPlaying())
	{
		NotifyAnimationFinished();
	}

	for (auto BehaviorInstance : m_BehaviorInstances)
	{
		// Do not re-run current behavior instance
		if (m_CurrentBehaviorInstance == BehaviorInstance)
		{
			continue;
		}

		if (BehaviorInstance->EvaluateForExecution(this))
		{
			m_CurrentBehaviorInstance = BehaviorInstance;

			float BlendTime = BehaviorInstance->GetBlendInTime();
			if (BlendTime > 0.0f)
			{
				m_AnimBlender.BlendTo(BehaviorInstance->GetAnimation(),
									  BehaviorInstance->GetAnimation()->GetStartTime(), 1.0f,
									  BlendTime);
			}
			else
			{
				m_AnimBlender.Play(BehaviorInstance->GetAnimation());
			}
		}
	}
}

float FTGPlayerStateMachine::GetCurrentBehaviorTime() const
{
	if (m_AnimBlender.GetSourceAnimation())
	{
		if (m_AnimBlender.GetTargetAnimation())
		{
			return m_AnimBlender.GetTargetPlaybackTime() / m_AnimBlender.GetTargetAnimation()->GetFrameRate();
		}
		else
		{
			return m_AnimBlender.GetSourcePlaybackTime() / m_AnimBlender.GetSourceAnimation()->GetFrameRate();
		}
	}

	return 0.0f;
}

void FTGPlayerStateMachine::SetNextBehavior(EPlayerBehavior NextBehavior)
{
	m_NextBehavior = NextBehavior;
}

void FTGPlayerStateMachine::NotifyAnimationFinished()
{
	if (m_CurrentBehaviorInstance)
	{
		m_CurrentBehaviorInstance->NotifyBehaviorFinished(this);
	}
}

void FTGPlayerStateMachine::CacheAnimations(RMesh* Mesh)
{
	for (auto BehaviorInstance : m_BehaviorInstances)
	{
		Mesh->CacheAnimation(BehaviorInstance->GetAnimation());
	}
}

void FTGPlayerStateMachine::ReleaseAssets()
{
	for (auto BehaviorInstance : m_BehaviorInstances)
	{
		delete BehaviorInstance;
	}

	m_BehaviorInstances.clear();
}

FTGPlayerBehaviorBase* FTGPlayerStateMachine::FindBehaviorInstance(EPlayerBehavior Behavior) const
{
	for (auto BehaviorInstance : m_BehaviorInstances)
	{
		if (BehaviorInstance->GetBehaviorEnum() == Behavior)
		{
			return BehaviorInstance;
		}
	}

	return nullptr;
}
