//=============================================================================
// FTGPlayerStateMachine.cpp by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#include "FTGPlayerStateMachine.h"
#include "PlayerControllerBase.h"

FTGPlayerStateMachine::FTGPlayerStateMachine(PlayerControllerBase* InPlayerOwner)
	: m_PlayerOwner(InPlayerOwner)
	, m_NextBehavior(nullptr)
	, m_CurrentBehaviorInstance(nullptr)
	, m_AnimSpeedDeviation(1.0f)
{
}

FTGPlayerStateMachine::~FTGPlayerStateMachine()
{
}

void FTGPlayerStateMachine::InitAssets()
{
	m_CurrentBehaviorInstance = FindBehaviorInstance(BHV_Navigation);
	if (m_CurrentBehaviorInstance)
	{
		// Set initial behavior for the blend queue
		BlendQueue.AddBlendTarget(m_CurrentBehaviorInstance, 0.0f);
	}
}

void FTGPlayerStateMachine::Update(float DeltaTime)
{
	m_AnimBlender.ProceedAnimation(DeltaTime);

	if (m_AnimBlender.HasFinishedPlaying())
	{
		NotifyAnimationFinished();
	}

	BlendQueue.Proceed(DeltaTime);

	for (auto& BehaviorInstance : m_BehaviorInstances)
	{
		// Advance the behavior animation
		if (BlendQueue.IsBehaviorRelevant(BehaviorInstance.get()))
		{
			BehaviorInstance->Update(this, DeltaTime);
		}

		// Do not re-run current behavior instance
		if (m_CurrentBehaviorInstance == BehaviorInstance.get() && !m_CurrentBehaviorInstance->DoesAllowRerunSelf())
		{
			continue;
		}

		if (BehaviorInstance->EvaluateForExecution(this))
		{
			m_CurrentBehaviorInstance = BehaviorInstance.get();
			m_CurrentBehaviorInstance->NotifyBegin(this);

			// Clear next behavior enum since we're already there
			m_NextBehavior = nullptr;

			float BlendTime = BehaviorInstance->GetBlendInTime();
			BlendQueue.AddBlendTarget(BehaviorInstance.get(), BehaviorInstance->GetBlendInTime());
		}
	}
}

void FTGPlayerStateMachine::EvaluatePose(RAnimPoseData& PoseData)
{
	BlendQueue.EvaluatePose(PoseData);
}

RVec3 FTGPlayerStateMachine::GetCurrentRootOffset() const
{
	return m_AnimBlender.GetCurrentRootOffset();
}

std::string FTGPlayerStateMachine::GetDebugString() const
{
	std::stringstream DebugStream;

	for (auto& BehaviorInstance : m_BehaviorInstances)
	{
		// Advance the behavior animation
		if (BlendQueue.IsBehaviorRelevant(BehaviorInstance.get()))
		{
			DebugStream << BehaviorInstance->GetDebugString();
		}
	}

	return DebugStream.str();
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
	m_NextBehavior = FindBehaviorInstance(NextBehavior);
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

FTGPlayerBehaviorBase* FTGPlayerStateMachine::FindBehaviorInstance(EPlayerBehavior Behavior) const
{
	for (auto& BehaviorInstance : m_BehaviorInstances)
	{
		if (BehaviorInstance->GetBehaviorEnum() == Behavior)
		{
			return BehaviorInstance.get();
		}
	}

	return nullptr;
}
