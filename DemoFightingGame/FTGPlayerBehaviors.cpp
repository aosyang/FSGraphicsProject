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

FTGPlayerBehaviorBase::FTGPlayerBehaviorBase(const std::string& AnimResourcePath, int AnimFlags /*= 0*/)
	: FTGPlayerBehaviorBase()
{
	LoadAnimationAsset(AnimResourcePath, AnimFlags);
}

size_t FTGPlayerBehaviorBase::GetBehaviorId() const
{
	return 0;
}

bool FTGPlayerBehaviorBase::EvaluateForExecution(FTGPlayerStateMachine* StateMachine)
{
	return StateMachine->GetNextBehavior() == this;
}

void FTGPlayerBehaviorBase::Update(FTGPlayerStateMachine* StateMachine, float DeltaTime)
{
	AnimPlayer.Proceed(DeltaTime);
}

bool FTGPlayerBehaviorBase::EvaluatePose(const RMesh& SkinnedMesh, RMatrix4* OutBoneMatrices)
{
	if (AnimPlayer.Animation)
	{
		return AnimPlayer.EvaluatePose(SkinnedMesh, OutBoneMatrices);
	}

	return false;
}

void FTGPlayerBehaviorBase::CacheAssets(RMesh& SkinnedMesh)
{
	OnCacheAnimations(SkinnedMesh);
}

std::string FTGPlayerBehaviorBase::GetDebugString() const
{
	return std::string("");
}

void FTGPlayerBehaviorBase::NotifyBegin(FTGPlayerStateMachine* StateMachine)
{
	PlayerControllerBase* OwnerPlayer = StateMachine->GetOwner();
	if (OwnerPlayer)
	{
		FTGPlayerController* OwnerFightingPlayer = OwnerPlayer->CastTo<FTGPlayerController>();
		if (OwnerFightingPlayer)
		{
			OwnerFightingPlayer->ClearHitPlayerControllers();
		}
	}
}

void FTGPlayerBehaviorBase::NotifyEnd(FTGPlayerStateMachine* StateMachine)
{
	OnBehaviorFinished(StateMachine);
}

void FTGPlayerBehaviorBase::OnCacheAnimations(RMesh& SkinnedMesh)
{
	SkinnedMesh.CacheAnimation(m_Animation);
}

void FTGPlayerBehaviorBase::OnBehaviorFinished(FTGPlayerStateMachine* StateMachine)
{
	// By default, all behaviors return to idle pose when animation finishes
	StateMachine->SetNextBehavior(BHV_Navigation);
}

void FTGPlayerBehaviorBase::LoadAnimationAsset(const std::string& AssetPath, int flags /*= 0*/)
{
	assert(m_Animation == nullptr);
	RMesh* mesh = RResourceManager::Instance().LoadResource<RMesh>(AssetPath, EResourceLoadMode::Immediate);

	if (mesh)
	{
		m_Animation = mesh->GetAnimation();
		if (m_Animation)
		{
			m_Animation->SetBitFlags(flags);

			std::string filename = RFileUtil::GetFileNameInPath(AssetPath);
			m_Animation->SetName(filename);

			// Initialize the animation player
			AnimPlayer.Animation = m_Animation;
			AnimPlayer.CurrentPlaybackTime = m_Animation->GetStartTime();
			AnimPlayer.TimeScale = 1.0f;
		}
		else
		{
			RLogWarning("Unable to find animation data in mesh resource \'%s\'\n", AssetPath.c_str());
		}
	}
	else
	{
		RLogWarning("Failed to load mesh resource \'%s\'\n", AssetPath.c_str());
	}
}

bool PlayerBehavior_BackKick::EvaluateForExecution(FTGPlayerStateMachine* StateMachine)
{
	return StateMachine->GetCurrentBehaviorId() == PlayerBehavior_Kick::StaticClassId() &&	// Back kick can only be triggered during a regular kick behavior
		   StateMachine->GetNextBehavior() == this &&
		   StateMachine->GetCurrentBehaviorTime() >= 0.3f;
}

void PlayerBehavior_KnockedDown::OnBehaviorFinished(FTGPlayerStateMachine* StateMachine)
{
	StateMachine->SetNextBehavior(BHV_GetUp);
}

void PlayerBehavior_Kick::Update(FTGPlayerStateMachine* StateMachine, float DeltaTime)
{
	FTGPlayerBehaviorBase::Update(StateMachine, DeltaTime);

	float BehaviorTime = StateMachine->GetCurrentBehaviorTime();
	if (BehaviorTime > 0.1f && BehaviorTime < 0.3f)
	{
		PlayerControllerBase* OwnerPlayer = StateMachine->GetOwner();
		if (OwnerPlayer)
		{
			FTGPlayerController* OwnerFightingPlayer = OwnerPlayer->CastTo<FTGPlayerController>();
			if (OwnerFightingPlayer)
			{
				auto HitPlayers = OwnerFightingPlayer->TestSphereHitWithOtherPlayers(50.0f, RVec3(0, 100, -50));
				for (auto HitPlayer : HitPlayers)
				{
					if (HitPlayer == OwnerFightingPlayer)
					{
						continue;
					}

					if (OwnerFightingPlayer->HasHitPlayerController(HitPlayer))
					{
						continue;
					}

					if (HitPlayer->GetBehavior() != BHV_KnockedDown)
					{
						HitPlayer->SetBehavior(BHV_Hit);

						OwnerFightingPlayer->AddHitPlayerController(HitPlayer);
					}
				}
			}
		}
	}
}

void PlayerBehavior_Punch::Update(FTGPlayerStateMachine* StateMachine, float DeltaTime)
{
	FTGPlayerBehaviorBase::Update(StateMachine, DeltaTime);

	float BehaviorTime = StateMachine->GetCurrentBehaviorTime();
	if (BehaviorTime > 0.1f && BehaviorTime < 0.3f)
	{
		PlayerControllerBase* OwnerPlayer = StateMachine->GetOwner();
		if (OwnerPlayer)
		{
			FTGPlayerController* OwnerFightingPlayer = OwnerPlayer->CastTo<FTGPlayerController>();
			if (OwnerFightingPlayer)
			{
				auto HitPlayers = OwnerFightingPlayer->TestSphereHitWithOtherPlayers(20.0f, RVec3(0, 100, -50));
				for (auto HitPlayer : HitPlayers)
				{
					if (HitPlayer == OwnerFightingPlayer)
					{
						continue;
					}

					if (OwnerFightingPlayer->HasHitPlayerController(HitPlayer))
					{
						continue;
					}

					if (HitPlayer->GetBehavior() != BHV_KnockedDown)
					{
						HitPlayer->SetBehavior(BHV_Hit);

						OwnerFightingPlayer->AddHitPlayerController(HitPlayer);
					}
				}
			}
		}
	}
}

void PlayerBehavior_BackKick::Update(FTGPlayerStateMachine* StateMachine, float DeltaTime)
{
	FTGPlayerBehaviorBase::Update(StateMachine, DeltaTime);

	float BehaviorTime = StateMachine->GetCurrentBehaviorTime();
	if (BehaviorTime > 0.1f && BehaviorTime < 0.3f)
	{
		PlayerControllerBase* OwnerPlayer = StateMachine->GetOwner();
		if (OwnerPlayer)
		{
			FTGPlayerController* OwnerFightingPlayer = OwnerPlayer->CastTo<FTGPlayerController>();
			if (OwnerFightingPlayer)
			{
				auto HitPlayers = OwnerFightingPlayer->TestSphereHitWithOtherPlayers(50.0f, RVec3(0, 100, -30));
				for (auto HitPlayer : HitPlayers)
				{
					if (HitPlayer == OwnerFightingPlayer)
					{
						continue;
					}

					if (OwnerFightingPlayer->HasHitPlayerController(HitPlayer))
					{
						continue;
					}

					if (HitPlayer->GetBehavior() != BHV_KnockedDown)
					{
						RVec3 HitCenter = OwnerFightingPlayer->GetTransform()->GetTranslatedVector(RVec3(0, 50, -50), ETransformSpace::Local);
						RVec3 VictimToHitCenter = HitCenter - HitPlayer->GetPosition();
						VictimToHitCenter.SetY(0.0f);
						RVec3 AttackerForward = -OwnerFightingPlayer->GetForwardVector();
						if (RVec3::Dot(AttackerForward, VictimToHitCenter) >= 0)
						{
							VictimToHitCenter = -AttackerForward;
						}

						HitPlayer->SetPlayerFacing(VictimToHitCenter, false);
						HitPlayer->SetBehavior(BHV_KnockedDown);

						OwnerFightingPlayer->AddHitPlayerController(HitPlayer);
					}
				}
			}
		}
	}
}

void PlayerBehavior_SpinAttack::Update(FTGPlayerStateMachine* StateMachine, float DeltaTime)
{
	FTGPlayerBehaviorBase::Update(StateMachine, DeltaTime);

	float BehaviorTime = StateMachine->GetCurrentBehaviorTime();
	if (BehaviorTime > 0.3f && BehaviorTime < 0.6f)
	{
		PlayerControllerBase* OwnerPlayer = StateMachine->GetOwner();
		if (OwnerPlayer)
		{
			FTGPlayerController* OwnerFightingPlayer = OwnerPlayer->CastTo<FTGPlayerController>();
			if (OwnerFightingPlayer)
			{
				auto HitPlayers = OwnerFightingPlayer->TestSphereHitWithOtherPlayers(50.0f, RVec3(0, 50, -50));
				for (auto HitPlayer : HitPlayers)
				{
					// Avoid self-damage
					if (HitPlayer == OwnerFightingPlayer)
					{
						continue;
					}

					// Avoid hitting a target twice with the same action
					if (OwnerFightingPlayer->HasHitPlayerController(HitPlayer))
					{
						continue;
					}

					if (HitPlayer->GetBehavior() != BHV_KnockedDown)
					{
						RVec3 HitCenter = OwnerFightingPlayer->GetTransform()->GetTranslatedVector(RVec3(0, 50, -50), ETransformSpace::Local);
						RVec3 VictimToHitCenter = HitCenter - HitPlayer->GetPosition();
						VictimToHitCenter.SetY(0.0f);
						RVec3 AttackerForward = -OwnerFightingPlayer->GetForwardVector();
						if (RVec3::Dot(AttackerForward, VictimToHitCenter) >= 0)
						{
							VictimToHitCenter = -AttackerForward;
						}

						HitPlayer->SetPlayerFacing(VictimToHitCenter, false);
						HitPlayer->SetBehavior(BHV_KnockedDown);

						OwnerFightingPlayer->AddHitPlayerController(HitPlayer);
					}
				}
			}
		}
	}
}
