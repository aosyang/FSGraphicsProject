//=============================================================================
// RAnimNode_BlendPlayer.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "RAnimNode_Base.h"

class RAnimation;

// Animation node that blends and plays multiple animations by an input value
class RAnimNode_BlendPlayer : public RAnimNode_Base
{
	// Data struct for animations involved in the blending
	struct BlendEntry
	{
		BlendEntry(RAnimation* InAnimation, float InValue)
			: Animation(InAnimation)
			, Value(InValue)
		{
		}

		RAnimation* Animation;
		float Value;
	};

public:
	DECLARE_ANIM_NODE_FACTORY_METHOD(RAnimNode_BlendPlayer)

	RAnimNode_BlendPlayer(const std::string& InNodeName, const AnimNodeAttributeMap& Attributes);

	// Override RAnimNode_Base methods
	virtual void UpdateNode(float DeltaTime) override;
	virtual void EvaluatePose(RAnimPoseData& PoseData) override;

	virtual bool BindAnimVariable(const std::string& VariableName, float* ValPtr) override;

private:
	/// Get a float number representing the index (decimal part) and the blend factor (fraction part) of current animation selection
	float EvaluateAnimRelevancyFactor() const;

	/// Get relevant animations from index and blend factor.
	/// Returns the fraction of blend factor.
	float GetRelevantAnimations(float InRelevancyFactor, RAnimation** OutAnim0, RAnimation** OutAnim1) const;

	std::vector<BlendEntry> BlendEntries;

	// The input blend value that determines which animations should be played
	AnimVariable<float> BlendInput;

	// Normalized playback progress between 0 and 1
	float NormalizedPlaybackProgress;

	float AnimRelevancyFactor;
};
