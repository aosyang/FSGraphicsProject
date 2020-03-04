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
	m_CurrentBehaviorInstance = FindBehaviorInstance(BHV_Idle);
	if (m_CurrentBehaviorInstance)
	{
		m_AnimBlender.Play(m_CurrentBehaviorInstance->GetAnimation());

		// Set initial behavior for the blend queue
		BlendQueue.AddBlendTarget(m_CurrentBehaviorInstance, 0.0f);
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
			//m_AnimBlender.BlendOutTo(BehaviorInstance->GetAnimation(),
			//						 BehaviorInstance->GetAnimation()->GetStartTime(), m_AnimSpeedDeviation,
			//						 BlendTime);

			BlendQueue.AddBlendTarget(BehaviorInstance.get(), BehaviorInstance->GetBlendInTime());
		}
	}
}

bool FTGPlayerStateMachine::EvaluatePose(const RMesh& SkinnedMesh, RMatrix4* OutBoneMatrices)
{
	return BlendQueue.EvaluatePose(SkinnedMesh, OutBoneMatrices);
}

RVec3 FTGPlayerStateMachine::GetCurrentRootOffset() const
{
	return m_AnimBlender.GetCurrentRootOffset();
}

std::string FTGPlayerStateMachine::GetDebugString() const
{
	std::stringstream DebugStream;
	DebugStream << "Source animation : ";
	if (m_AnimBlender.GetSourceAnimation())
	{
		DebugStream << m_AnimBlender.GetSourceAnimation()->GetName();
	}
	DebugStream << std::endl;

	DebugStream << "Source start time : " << m_AnimBlender.GetSourceAnimation()->GetStartTime() << std::endl;
	DebugStream << "Source end time   : " << m_AnimBlender.GetSourceAnimation()->GetEndTime() << std::endl;
	DebugStream << "Source playback time : " << m_AnimBlender.GetSourcePlaybackTime() << std::endl;
	DebugStream << "Current root motion : " << m_AnimBlender.GetCurrentRootOffset().ToString() << std::endl;

	DebugStream << "Blend from : " << (m_AnimBlender.GetSourceAnimation() ? m_AnimBlender.GetSourceAnimation()->GetName() : "") << std::endl;
	DebugStream << "Blend to   : " << (m_AnimBlender.GetTargetAnimation() ? m_AnimBlender.GetTargetAnimation()->GetName() : "") << std::endl;
	DebugStream << "Blend time : " << m_AnimBlender.GetElapsedBlendTime() << std::endl;

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

void FTGPlayerStateMachine::CacheAnimations(RMesh* Mesh)
{
	for (auto& BehaviorInstance : m_BehaviorInstances)
	{
		Mesh->CacheAnimation(BehaviorInstance->GetAnimation());
	}
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
