//=============================================================================
// RAnimBlendQueue.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Core/CoreTypes.h"

class FTGPlayerBehaviorBase;
struct RAnimPoseData;

class RAnimBlendQueue
{
public:
	/// Add a new target behavior to the queue
	void AddBlendTarget(FTGPlayerBehaviorBase* TargetBehavior, float BlendTime);

	/// Proceed queue in time
	void Proceed(float DeltaTime);

	void EvaluatePose(RAnimPoseData& PoseData) const;

	/// Check if behavior is in blend queue
	bool IsBehaviorRelevant(FTGPlayerBehaviorBase* Behavior) const;

private:
	/// Data representing a blend instance of two behaviors
	struct RBlendQueueData
	{
		RBlendQueueData(FTGPlayerBehaviorBase* InTarget, float InBlendTime)
			: Target(InTarget)
			, TotalBlendTime(InBlendTime)
			, Progress(0.0f)
		{
		}

		FTGPlayerBehaviorBase* Target;

		float TotalBlendTime;
		float Progress;
	};

	std::list<RBlendQueueData> BlendQueueData;
};
