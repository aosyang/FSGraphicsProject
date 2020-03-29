//=============================================================================
// PlayerBehavior_Navigation.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "PlayerBehavior_Navigation.h"
#include "FTGPlayerStateMachine.h"
#include "PlayerControllerBase.h"

PlayerBehavior_Navigation::PlayerBehavior_Navigation()
	: CurrentSpeed(0.0f)
	, TargetSpeed(CurrentSpeed)
	, NormalizedPlaybackProgress(0.0f)
	, AnimRelevancyFactor(0.0f)
{
	m_BehaviorEnum = BHV_Navigation;
}

void PlayerBehavior_Navigation::AddAnimation(const std::string& AnimationAsset, float Speed)
{
	RMesh* Mesh = RResourceManager::Instance().LoadResource<RMesh>(AnimationAsset, EResourceLoadMode::Immediate);
	RAnimation* NewAnimation = Mesh ? Mesh->GetAnimation() : nullptr;
	if (NewAnimation)
	{
		// Force in-place animations for navigation
		NewAnimation->EnableBitFlags(AnimBitFlag_LockRootBone);

		// If speed is smaller than the first data, insert as head
		if (AnimData.size() > 0 && Speed <= AnimData[0].Speed)
		{
			AnimData.insert(AnimData.begin(), NavigationAnimData(NewAnimation, Speed));
			return;
		}

		for (int i = 0; i < (int)AnimData.size() - 1; i++)
		{
			if (Speed > AnimData[i].Speed && Speed <= AnimData[i + 1].Speed)
			{
				AnimData.insert(AnimData.begin() + i + 1, NavigationAnimData(NewAnimation, Speed));
				return;
			}
		}

		AnimData.insert(AnimData.end(), NavigationAnimData(NewAnimation, Speed));
	}
	else
	{
		RLogWarning("Failed to add animtion %s to navigation behavior. Unable to get animation from mesh asset!\n", AnimationAsset.c_str());
	}
}

void PlayerBehavior_Navigation::Update(FTGPlayerStateMachine* StateMachine, float DeltaTime)
{
	const RVec3 Velocity = StateMachine->GetOwner()->GetVelocity();
	TargetSpeed = Velocity.Magnitude2D();

	// Blend towards target speed
	const float BlendSpeed = 1000.0f * DeltaTime;
	if (fabs(TargetSpeed - CurrentSpeed) < BlendSpeed)
	{
		CurrentSpeed = TargetSpeed;
	}
	else
	{
		if (TargetSpeed - CurrentSpeed > 0.0f)
		{
			CurrentSpeed += BlendSpeed;
		}
		else
		{
			CurrentSpeed -= BlendSpeed;
		}
	}
	
	//RLog("Navigation speed: %f\n", CurrentSpeed);

	AnimRelevancyFactor = EvaluateAnimRelevancyFactor();

	RAnimation* Anim0;
	RAnimation* Anim1;
	float BlendFactor = GetRelevantAnimations(AnimRelevancyFactor, &Anim0, &Anim1);

	float StartTime, EndTime, FrameRate;
	if (Anim1 == nullptr)
	{
		StartTime = Anim0->GetStartTime();
		EndTime = Anim0->GetEndTime();
		FrameRate = Anim0->GetFrameRate();
	}
	else
	{
		StartTime = RMath::Lerp(Anim0->GetStartTime(), Anim1->GetStartTime(), BlendFactor);
		EndTime = RMath::Lerp(Anim0->GetEndTime(), Anim1->GetEndTime(), BlendFactor);
		FrameRate = RMath::Lerp(Anim0->GetFrameRate(), Anim1->GetFrameRate(), BlendFactor);
	}
	float Duration = EndTime - StartTime;
	if (Duration > 0.0f)
	{
		float PlaybackProgress = RMath::Lerp(StartTime, EndTime, NormalizedPlaybackProgress);
		PlaybackProgress += DeltaTime * FrameRate;

		while (PlaybackProgress > EndTime)
		{
			PlaybackProgress -= Duration;
		}

		NormalizedPlaybackProgress = (PlaybackProgress - StartTime) / Duration;
	}
	else
	{
		NormalizedPlaybackProgress = 0.0f;
	}
	assert(NormalizedPlaybackProgress >= 0.0f && NormalizedPlaybackProgress <= 1.0f);
}

bool PlayerBehavior_Navigation::EvaluatePose(const RMesh& SkinnedMesh, RMatrix4* OutBoneMatrices)
{
	RAnimation* Anim0;
	RAnimation* Anim1;
	float BlendFactor = GetRelevantAnimations(AnimRelevancyFactor, &Anim0, &Anim1);
	assert(Anim0);

	float PlaybackTime0 = RMath::Lerp(Anim0->GetStartTime(), Anim0->GetEndTime(), NormalizedPlaybackProgress);
	if (Anim1 == nullptr)
	{
		RAnimationPlayer Player;

		Player.Animation = Anim0;
		Player.CurrentPlaybackTime = PlaybackTime0;

		return Player.EvaluatePose(SkinnedMesh, OutBoneMatrices);
	}
	else
	{
		float PlaybackTime1 = RMath::Lerp(Anim1->GetStartTime(), Anim1->GetEndTime(), NormalizedPlaybackProgress);
		RAnimationPlayer Player0, Player1;

		Player0.Animation = Anim0;
		Player0.CurrentPlaybackTime = PlaybackTime0;
		Player1.Animation = Anim1;
		Player1.CurrentPlaybackTime = PlaybackTime1;

		std::vector<RMatrix4> Pose0, Pose1;
		Pose0.resize(SkinnedMesh.GetBoneCount());
		Pose1.resize(SkinnedMesh.GetBoneCount());

		if (Player0.EvaluatePose(SkinnedMesh, Pose0.data()) && Player1.EvaluatePose(SkinnedMesh, Pose1.data()))
		{
			for (int i = 0; i < SkinnedMesh.GetBoneCount(); i++)
			{
				OutBoneMatrices[i] = RMatrix4::Slerp(Pose0[i], Pose1[i], BlendFactor);
			}
		}
		
		return false;
	}
}

std::string PlayerBehavior_Navigation::GetDebugString() const
{
	std::stringstream DebugStream;
	DebugStream << "Animations:" << std::endl;
	for (const auto& Data : AnimData)
	{
		DebugStream << Data.Speed << " : " << Data.Animation->GetName() << std::endl;
	}
	DebugStream << "Current speed: " << CurrentSpeed << std::endl;
	DebugStream << "Target speed: " << TargetSpeed << std::endl;

	return DebugStream.str();
}

void PlayerBehavior_Navigation::OnCacheAnimations(RMesh& SkinnedMesh)
{
	for (auto Iter : AnimData)
	{
		SkinnedMesh.CacheAnimation(Iter.Animation);
	}
}

float PlayerBehavior_Navigation::EvaluateAnimRelevancyFactor() const
{
	if (AnimData.size() == 0)
	{
		return -1.0f;
	}

	if (CurrentSpeed <= AnimData[0].Speed)
	{
		return 0.0f;
	}

	for (int i = 0; i < (int)AnimData.size() - 1; i++)
	{
		if (CurrentSpeed > AnimData[i].Speed && CurrentSpeed <= AnimData[i + 1].Speed)
		{
			return (float)i + (CurrentSpeed - AnimData[i].Speed) / (AnimData[i + 1].Speed - AnimData[i].Speed);
		}
	}

	return (float)(AnimData.size() - 1);
}

float PlayerBehavior_Navigation::GetRelevantAnimations(float InRelevancyFactor, RAnimation** OutAnim0, RAnimation** OutAnim1) const
{
	int Index = (int)InRelevancyFactor;
	float BlendFactor = InRelevancyFactor - Index;

	*OutAnim0 = AnimData[Index].Animation;
	*OutAnim1 = (Index < (AnimData).size() - 1) ? AnimData[Index + 1].Animation : nullptr;

	return BlendFactor;
}
