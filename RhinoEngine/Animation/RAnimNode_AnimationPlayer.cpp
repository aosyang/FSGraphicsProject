//=============================================================================
// RAnimNode_AnimationPlayer.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "RAnimNode_AnimationPlayer.h"
#include "Resource/RResourceManager.h"
#include "RenderSystem/RMesh.h"
#include "Animation/RAnimation.h"

RAnimNode_AnimationPlayer::RAnimNode_AnimationPlayer()
	: Animation(nullptr)
	, CurrentPlaybackTime(0.0f)
	, bLoop(false)
	, TimeScale(1.0f)
	, IsAnimDone(false)
{
}

RAnimNode_AnimationPlayer::RAnimNode_AnimationPlayer(const std::string& InNodeName, const AnimNodeAttributeMap& Attributes)
	: RAnimNode_Base(InNodeName, Attributes)
	, Animation(nullptr)
	, CurrentPlaybackTime(0.0f)
	, bLoop(false)
	, TimeScale(1.0f)
	, IsAnimDone(false)
{
	std::string AnimationName;
	if (AnimNodeAttributeMap::Query(Attributes.Map, "Animation", AnimationName))
	{
		if (RMesh* MeshResource = RResourceManager::Instance().LoadResource<RMesh>(AnimationName, EResourceLoadMode::Immediate))
		{
			Animation = MeshResource->GetAnimation();
		}
	}

	AnimNodeAttributeMap::QueryBool(Attributes.Map, "Loop", bLoop);
}

void RAnimNode_AnimationPlayer::Reset()
{
	Animation = nullptr;
	CurrentPlaybackTime = 0.0f;
	bLoop = false;
	TimeScale = 1.0f;
	RootOffset = RVec3::Zero();
	IsAnimDone = false;
}

void RAnimNode_AnimationPlayer::UpdateNode(float DeltaTime)
{
	if (Animation && !IsAnimDone)
	{
		// Changing time may cause start time greater than end time
		if (CurrentPlaybackTime >= Animation->GetEndTime())
		{
			CurrentPlaybackTime = Animation->GetStartTime();
		}

		// Root motion offset at the beginning of this frame
		RVec3 PrevRootOffset = Animation->GetRootPosition(CurrentPlaybackTime);

		CurrentPlaybackTime += DeltaTime * Animation->GetFrameRate() * TimeScale;
		bool startOver = false;

		// The playback time has passed the end time of the animation
		if (CurrentPlaybackTime >= Animation->GetEndTime())
		{
			if (Animation->IsLooping() || bLoop)
			{
				float AnimDuration = Animation->GetEndTime() - Animation->GetStartTime();

				if (AnimDuration == 0.0f)
				{
					// If the animation has only one frame, always loop the first frame
					CurrentPlaybackTime = Animation->GetStartTime();
					startOver = true;
				}
				else
				{
					while (CurrentPlaybackTime >= Animation->GetEndTime())
					{
						CurrentPlaybackTime -= AnimDuration;
						startOver = true;
					}
				}
			}
			else
			{
				CurrentPlaybackTime = Animation->GetEndTime() - 0.01f;
				IsAnimDone = true;
			}
		}

		if (Animation->HasRootMotion())
		{
			RootOffset = Animation->GetRootPosition(CurrentPlaybackTime) - PrevRootOffset;
			if (startOver)
			{
				RootOffset = Animation->GetRootPosition(Animation->GetEndTime() - 1) - PrevRootOffset +
					Animation->GetRootPosition(CurrentPlaybackTime) - Animation->GetInitRootPosition();
			}
		}
		else
		{
			RootOffset = RVec3(0, 0, 0);
		}
	}
}

void RAnimNode_AnimationPlayer::Rewind()
{
	IsAnimDone = false;
	CurrentPlaybackTime = Animation ? Animation->GetStartTime() : 0.0f;
}

void RAnimNode_AnimationPlayer::EvaluatePose(RAnimPoseData& PoseData)
{
	Animation->EvaluatePoseAtTime(PoseData, CurrentPlaybackTime);
}
