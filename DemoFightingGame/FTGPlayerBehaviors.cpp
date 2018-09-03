//=============================================================================
// FTGPlayerBehaviors.cpp by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#include "FTGPlayerBehaviors.h"
#include "FTGPlayerStateMachine.h"

FTGPlayerBehaviorBase::FTGPlayerBehaviorBase()
	: m_BehaviorEnum(BHV_Invalid)
	, m_Animation(nullptr)
	, m_BlendTime(0.0f)
{

}

bool FTGPlayerBehaviorBase::EvaluateForExecution(FTGPlayerStateMachine* StateMachine)
{
	return StateMachine->GetNextBehavior() == GetBehaviorEnum();
}

void FTGPlayerBehaviorBase::NotifyBehaviorFinished(FTGPlayerStateMachine* StateMachine)
{
	OnBehaviorFinished(StateMachine);
}

void FTGPlayerBehaviorBase::OnBehaviorFinished(FTGPlayerStateMachine* StateMachine)
{
	// By default, all behaviors return to idle pose when animation finishes
	StateMachine->SetNextBehavior(BHV_Idle);
}

void FTGPlayerBehaviorBase::LoadAnimationAsset(const char* AssetPath, int flags /*= 0*/)
{
	assert(m_Animation == nullptr);
	RMesh* mesh = RResourceManager::Instance().LoadFbxMesh(AssetPath, EResourceLoadMode::Immediate);

	if (mesh)
	{
		m_Animation = mesh->GetAnimation();
		if (m_Animation)
		{
			m_Animation->SetBitFlags(flags);

			string strResPath = string(AssetPath);
			string filename = RFileUtil::GetFileNameInPath(strResPath);
			m_Animation->SetName(filename);
		}
		else
		{
			RLogWarning("Unable to find animation data in mesh resource \'%s\'\n", AssetPath);
		}
	}
	else
	{
		RLogWarning("Failed to load mesh resource \'%s\'\n", AssetPath);
	}
}

bool FTGPlayerBehavior_BackKick::EvaluateForExecution(FTGPlayerStateMachine* StateMachine)
{
	return StateMachine->GetCurrentBehavior() == BHV_Kick &&
		   StateMachine->GetNextBehavior() == GetBehaviorEnum() &&
		   StateMachine->GetCurrentBehaviorTime() >= 0.3f;
}

void FTGPlayerBehavior_KnockedDown::OnBehaviorFinished(FTGPlayerStateMachine* StateMachine)
{
	StateMachine->SetNextBehavior(BHV_GetUp);
}
