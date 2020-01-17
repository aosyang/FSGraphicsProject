//=============================================================================
// FTGPlayerStateMachine.cpp by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#include "FTGPlayerStateMachine.h"
#include "PlayerControllerBase.h"

FTGPlayerStateMachine::FTGPlayerStateMachine(PlayerControllerBase* InPlayerOwner)
	: m_PlayerOwner(InPlayerOwner)
	, m_NextBehavior(BHV_None)
	, m_CurrentBehaviorInstance(nullptr)
	, m_AnimSpeedDeviation(1.0f)
{
}

FTGPlayerStateMachine::~FTGPlayerStateMachine()
{
	ReleaseAssets();
}

void FTGPlayerStateMachine::InitAssets()
{
	m_CurrentBehaviorInstance = FindBehaviorInstance(BHV_Idle);
	if (m_CurrentBehaviorInstance)
	{
		m_AnimBlender.Play(m_CurrentBehaviorInstance->GetAnimation());
	}

	if (m_PlayerOwner)
	{
		RMesh* Mesh = m_PlayerOwner->GetMesh();
		if (Mesh)
		{
			CacheAnimations(Mesh);
		}
	}
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
			m_AnimBlender.BlendOutTo(BehaviorInstance->GetAnimation(),
									 BehaviorInstance->GetAnimation()->GetStartTime(), m_AnimSpeedDeviation,
									 BlendTime);
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
