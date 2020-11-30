//=============================================================================
// RAnimNode_AnimationPlayer.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once 

#include "RAnimNode_Base.h"

class RAnimation;

/// The animation node that plays a single animation resource
class RAnimNode_AnimationPlayer : public RAnimNode_Base
{
public:
	DECLARE_ANIM_NODE_FACTORY_METHOD(RAnimNode_AnimationPlayer)

	RAnimNode_AnimationPlayer();
	RAnimNode_AnimationPlayer(const std::string& InNodeName, const AnimNodeAttributeMap& Attributes);

	RAnimation*		Animation;
	float			CurrentPlaybackTime;

	// Whether player should loop the playback
	bool			bLoop;
	float			TimeScale;

	RVec3			RootOffset;
	bool			IsAnimDone;

	void Reset();

	/// Proceed the animation playback
	virtual void UpdateNode(float DeltaTime) override;

	/// Start the animation from beginning
	void Rewind();

	// Override RAnimNode_Base methods
	virtual void EvaluatePose(RAnimPoseData& PoseData) override;
};
