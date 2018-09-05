//=============================================================================
// FTGPlayerStateMachine.cpp by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#include "FTGPlayerStateMachine.h"

FTGPlayerStateMachine::FTGPlayerStateMachine()
	: m_PlayerOwner(nullptr)
	, m_NextBehavior(BHV_None)
	, m_CurrentBehaviorInstance(nullptr)
	, m_AnimSpeedDeviation(1.0f)
{
}

FTGPlayerStateMachine::~FTGPlayerStateMachine()
{
	ReleaseAssets();
}

void FTGPlayerStateMachine::Init(FTGPlayerController* Owner)
{
	m_PlayerOwner = Owner;

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
		if (m_CurrentBehaviorInstance == BehaviorInstance && !m_CurrentBehaviorInstance->DoesAllowRerunSelf())
		{
			continue;
		}

		if (BehaviorInstance->EvaluateForExecution(this))
		{
			m_CurrentBehaviorInstance = BehaviorInstance;
			m_CurrentBehaviorInstance->NotifyBegin(this);

			// Clear next behavior enum since we're already there
			m_NextBehavior = BHV_None;

			float BlendTime = BehaviorInstance->GetBlendInTime();
			if (BlendTime > 0.0f)
			{
				m_AnimBlender.BlendTo(BehaviorInstance->GetAnimation(),
									  BehaviorInstance->GetAnimation()->GetStartTime(), m_AnimSpeedDeviation,
									  BlendTime);
			}
			else
			{
				m_AnimBlender.Play(BehaviorInstance->GetAnimation(), m_AnimSpeedDeviation);
			}
		}
	}

	if (m_CurrentBehaviorInstance)
	{
		m_CurrentBehaviorInstance->Update(this, DeltaTime);
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
		m_CurrentBehaviorInstance->NotifyEnd(this);
	}
}

void FTGPlayerStateMachine::SetAnimationDeviation(float Deviation)
{
	m_AnimSpeedDeviation = Deviation;
}

float FTGPlayerStateMachine::GetAnimationDeviation() const
{
	return m_AnimSpeedDeviation;
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
