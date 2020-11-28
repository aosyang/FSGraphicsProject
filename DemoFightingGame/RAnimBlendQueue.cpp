//=============================================================================
// RAnimBlendQueue.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RAnimBlendQueue.h"

#include "FTGPlayerBehaviors.h"

void RAnimBlendQueue::AddBlendTarget(FTGPlayerBehaviorBase* TargetBehavior, float BlendTime)
{
	if (BlendQueueData.size() > 0)
	{
		auto Iter = BlendQueueData.begin();
		if (Iter->Target == TargetBehavior)
		{
			// Target behavior is already at the top of queue, do nothing
			return;
		}
	}

	BlendQueueData.emplace_front(TargetBehavior, BlendTime);
}

void RAnimBlendQueue::Proceed(float DeltaTime)
{
	auto Iter = BlendQueueData.begin();

	while (Iter != BlendQueueData.end())
	{
		Iter->Progress += DeltaTime;

		// Has any animation finished blending? Discard any following member in queue as they are no longer relevant
		if (Iter->Progress >= Iter->TotalBlendTime)
		{
			Iter->Progress = Iter->TotalBlendTime;
			BlendQueueData.erase(++Iter, BlendQueueData.end());

			return;
		}

		Iter++;
	}
}

void RAnimBlendQueue::EvaluatePose(RAnimPoseData& PoseData) const
{
	const int NumBones = PoseData.SkinnedMesh->GetBoneCount();

	RAnimPoseData TargetPose(*PoseData.SkinnedMesh);

	/// Resolve the blend queue bottom-up
	for (auto Iter = BlendQueueData.rbegin(); Iter != BlendQueueData.rend(); Iter++)
	{
		float BlendFactor = Iter->TotalBlendTime == 0.0f ? 1.0f : Iter->Progress / Iter->TotalBlendTime;
		assert(BlendFactor >= 0.0f && BlendFactor <= 1.0f);

		// Evaluate pose for the blend-to target
		Iter->Target->EvaluatePose(TargetPose);

		PoseData = RAnimPoseData::BlendTwoPoses(PoseData, TargetPose, BlendFactor);
	}
}

bool RAnimBlendQueue::IsBehaviorRelevant(FTGPlayerBehaviorBase* Behavior) const
{
	for (auto Iter : BlendQueueData)
	{
		if (Iter.Target == Behavior)
		{
			return true;
		}
	}

	return false;
}
