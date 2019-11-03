//=============================================================================
// FTGPlayerBehaviors.cpp by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#include "FTGPlayerBehaviors.h"
#include "FTGPlayerStateMachine.h"
#include "FTGPlayerController.h"

FTGPlayerBehaviorBase::FTGPlayerBehaviorBase()
	: m_BehaviorEnum(BHV_None)
	, m_Animation(nullptr)
	, m_BlendTime(0.0f)
	, m_bAllowRerunSelf(false)
{

}

bool FTGPlayerBehaviorBase::EvaluateForExecution(FTGPlayerStateMachine* StateMachine)
{
	return StateMachine->GetNextBehavior() == GetBehaviorEnum();
}

void FTGPlayerBehaviorBase::Update(FTGPlayerStateMachine* StateMachine, float DeltaTime)
{

}

void FTGPlayerBehaviorBase::NotifyBegin(FTGPlayerStateMachine* StateMachine)
{
	FTGPlayerController* OwnerPlayer = StateMachine->GetOwner();
	if (OwnerPlayer)
	{
		OwnerPlayer->ClearHitPlayerControllers();
	}
}

void FTGPlayerBehaviorBase::NotifyEnd(FTGPlayerStateMachine* StateMachine)
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
	RMesh* mesh = RResourceManager::Instance().LoadResource<RMesh>(AssetPath, EResourceLoadMode::Immediate);

	if (mesh)
	{
		m_Animation = mesh->GetAnimation();
		if (m_Animation)
		{
			m_Animation->SetBitFlags(flags);

			std::string strResPath = std::string(AssetPath);
			std::string filename = RFileUtil::GetFileNameInPath(strResPath);
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

void FTGPlayerBehavior_Kick::Update(FTGPlayerStateMachine* StateMachine, float DeltaTime)
{
	float BehaviorTime = StateMachine->GetCurrentBehaviorTime();
	if (BehaviorTime > 0.1f && BehaviorTime < 0.3f)
	{
		FTGPlayerController* PlayerOwner = StateMachine->GetOwner();
		if (PlayerOwner)
		{
			auto HitPlayers = PlayerOwner->TestSphereHitWithOtherPlayers(50.0f, RVec3(0, 100, -50));
			for (auto HitPlayer : HitPlayers)
			{
				if (HitPlayer == PlayerOwner)
				{
					continue;
				}

				if (PlayerOwner->HasHitPlayerController(HitPlayer))
				{
					continue;
				}

				if (HitPlayer->GetBehavior() != BHV_KnockedDown)
				{
					HitPlayer->SetBehavior(BHV_Hit);

					PlayerOwner->AddHitPlayerController(HitPlayer);
				}
			}
		}
	}
}

void FTGPlayerBehavior_Punch::Update(FTGPlayerStateMachine* StateMachine, float DeltaTime)
{
	float BehaviorTime = StateMachine->GetCurrentBehaviorTime();
	if (BehaviorTime > 0.1f && BehaviorTime < 0.3f)
	{
		FTGPlayerController* PlayerOwner = StateMachine->GetOwner();
		if (PlayerOwner)
		{
			auto HitPlayers = PlayerOwner->TestSphereHitWithOtherPlayers(20.0f, RVec3(0, 100, -50));
			for (auto HitPlayer : HitPlayers)
			{
				if (HitPlayer == PlayerOwner)
				{
					continue;
				}

				if (PlayerOwner->HasHitPlayerController(HitPlayer))
				{
					continue;
				}

				if (HitPlayer->GetBehavior() != BHV_KnockedDown)
				{
					HitPlayer->SetBehavior(BHV_Hit);

					PlayerOwner->AddHitPlayerController(HitPlayer);
				}
			}
		}
	}
}

void FTGPlayerBehavior_BackKick::Update(FTGPlayerStateMachine* StateMachine, float DeltaTime)
{
	float BehaviorTime = StateMachine->GetCurrentBehaviorTime();
	if (BehaviorTime > 0.1f && BehaviorTime < 0.3f)
	{
		FTGPlayerController* PlayerOwner = StateMachine->GetOwner();
		if (PlayerOwner)
		{
			auto HitPlayers = PlayerOwner->TestSphereHitWithOtherPlayers(50.0f, RVec3(0, 100, -30));
			for (auto HitPlayer : HitPlayers)
			{
				if (HitPlayer == PlayerOwner)
				{
					continue;
				}

				if (PlayerOwner->HasHitPlayerController(HitPlayer))
				{
					continue;
				}

				if (HitPlayer->GetBehavior() != BHV_KnockedDown)
				{
					RVec3 HitCenter = PlayerOwner->GetTransform()->GetTranslatedVector(RVec3(0, 50, -50), ETransformSpace::Local);
					RVec3 VictimToHitCenter = HitCenter - HitPlayer->GetPosition();
					VictimToHitCenter.SetY(0.0f);
					RVec3 AttackerForward = -PlayerOwner->GetForwardVector();
					if (RVec3::Dot(AttackerForward, VictimToHitCenter) >= 0)
					{
						VictimToHitCenter = -AttackerForward;
					}

					HitPlayer->SetPlayerFacing(VictimToHitCenter, false);
					HitPlayer->SetBehavior(BHV_KnockedDown);

					PlayerOwner->AddHitPlayerController(HitPlayer);
				}
			}
		}
	}
}

void FTGPlayerBehavior_SpinAttack::Update(FTGPlayerStateMachine* StateMachine, float DeltaTime)
{
	float BehaviorTime = StateMachine->GetCurrentBehaviorTime();
	if (BehaviorTime > 0.3f && BehaviorTime < 0.6f)
	{
		FTGPlayerController* PlayerOwner = StateMachine->GetOwner();
		if (PlayerOwner)
		{
			auto HitPlayers = PlayerOwner->TestSphereHitWithOtherPlayers(50.0f, RVec3(0, 50, -50));
			for (auto HitPlayer : HitPlayers)
			{
				if (HitPlayer == PlayerOwner)
				{
					continue;
				}

				if (PlayerOwner->HasHitPlayerController(HitPlayer))
				{
					continue;
				}

				if (HitPlayer->GetBehavior() != BHV_KnockedDown)
				{
					RVec3 HitCenter = PlayerOwner->GetTransform()->GetTranslatedVector(RVec3(0, 50, -50), ETransformSpace::Local);
					RVec3 VictimToHitCenter = HitCenter - HitPlayer->GetPosition();
					VictimToHitCenter.SetY(0.0f);
					RVec3 AttackerForward = -PlayerOwner->GetForwardVector();
					if (RVec3::Dot(AttackerForward, VictimToHitCenter) >= 0)
					{
						VictimToHitCenter = -AttackerForward;
					}

					HitPlayer->SetPlayerFacing(VictimToHitCenter, false);
					HitPlayer->SetBehavior(BHV_KnockedDown);
					
					PlayerOwner->AddHitPlayerController(HitPlayer);
				}
			}
		}
	}
}
