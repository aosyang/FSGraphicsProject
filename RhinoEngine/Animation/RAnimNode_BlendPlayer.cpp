//=============================================================================
// RAnimNode_BlendPlayer.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "RAnimNode_BlendPlayer.h"
#include "Resource/RResourceManager.h"
#include "RenderSystem/RMesh.h"
#include "RAnimation.h"

RAnimNode_BlendPlayer::RAnimNode_BlendPlayer(const std::string& InNodeName, const AnimNodeAttributeMap& Attributes)
	: RAnimNode_Base(InNodeName, Attributes)
	, BlendInput(0)
	, NormalizedPlaybackProgress(0.0f)
	, AnimRelevancyFactor(0.0f)
{
	for (const auto& Iter : Attributes.ChildEntries)
	{
		if (Iter.EntryName == "BlendEntry")
		{
			RAnimation* Animation = nullptr;
			float SampleValue = 0.0f;

			std::string AnimationName;
			if (AnimNodeAttributeMap::Query(Iter.Map, "Animation", AnimationName))
			{
				if (RMesh* MeshResource = RResourceManager::Instance().LoadResource<RMesh>(AnimationName, EResourceLoadMode::Immediate))
				{
					Animation = MeshResource->GetAnimation();
				}
			}

			AnimNodeAttributeMap::QueryFloat(Iter.Map, "SampleValue", SampleValue);

			BlendEntries.emplace_back(Animation, SampleValue);
		}
	}

	// Sort entires by ascending values
	auto CompareBySampleValue = [](const BlendEntry& Lhs, const BlendEntry& Rhs) {
		return Lhs.Value < Rhs.Value;
	};
	std::stable_sort(BlendEntries.begin(), BlendEntries.end(), CompareBySampleValue);
}

void RAnimNode_BlendPlayer::UpdateNode(float DeltaTime)
{
	BlendInput.UpdateVal();

	AnimRelevancyFactor = EvaluateAnimRelevancyFactor();

	RAnimation* Anim0, * Anim1;
	float BlendFactor = GetRelevantAnimations(AnimRelevancyFactor, &Anim0, &Anim1);

	float StartTime, EndTime, FrameRate;

	// Sync two animations by progress
	{
		// The start time, end time and frame rate are blending results of two relevant animations
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
}

void RAnimNode_BlendPlayer::EvaluatePose(RAnimPoseData& PoseData)
{
	RAnimation* Anim0 = nullptr, * Anim1 = nullptr;
	float BlendFactor = GetRelevantAnimations(AnimRelevancyFactor, &Anim0, &Anim1);
	assert(Anim0);

	float PlaybackTime0 = RMath::Lerp(Anim0->GetStartTime(), Anim0->GetEndTime(), NormalizedPlaybackProgress);
	if (Anim1)
	{
		// Find playback time on animation1 by the playback progression
		float PlaybackTime1 = RMath::Lerp(Anim1->GetStartTime(), Anim1->GetEndTime(), NormalizedPlaybackProgress);

		// Evaluate poses for both animations and then blend them together
		RAnimPoseData Pose0(*PoseData.SkinnedMesh),
					  Pose1(*PoseData.SkinnedMesh);

		Anim0->EvaluatePoseAtTime(Pose0, PlaybackTime0);
		Anim1->EvaluatePoseAtTime(Pose1, PlaybackTime1);

		PoseData = RAnimPoseData::BlendTwoPoses(Pose0, Pose1, BlendFactor);
	}
	else
	{
		Anim0->EvaluatePoseAtTime(PoseData, PlaybackTime0);
	}
}

bool RAnimNode_BlendPlayer::BindAnimVariable(const std::string& VariableName, float* ValPtr)
{
	if (VariableName == "BlendInput")
	{
		BlendInput.SourcePtr = ValPtr;
		return true;
	}

	return false;
}

float RAnimNode_BlendPlayer::EvaluateAnimRelevancyFactor() const
{
	if (BlendEntries.size() == 0)
	{
		return -1.0f;
	}

	if (BlendInput <= BlendEntries[0].Value)
	{
		return 0.0f;
	}

	for (int i = 0; i < (int)BlendEntries.size() - 1; i++)
	{
		if (BlendInput > BlendEntries[i].Value && BlendInput <= BlendEntries[i + 1].Value)
		{
			return (float)i + (BlendInput - BlendEntries[i].Value) / (BlendEntries[i + 1].Value - BlendEntries[i].Value);
		}
	}

	return (float)(BlendEntries.size() - 1);
}

float RAnimNode_BlendPlayer::GetRelevantAnimations(float InRelevancyFactor, RAnimation** OutAnim0, RAnimation** OutAnim1) const
{
	int Index = (int)InRelevancyFactor;
	float BlendFactor = InRelevancyFactor - Index;
	int NumEntries = (int)BlendEntries.size();

	*OutAnim0 = NumEntries > 0 ? BlendEntries[Index].Animation : nullptr;
	*OutAnim1 = (Index < NumEntries - 1) ? BlendEntries[Index + 1].Animation : nullptr;

	return BlendFactor;
}
