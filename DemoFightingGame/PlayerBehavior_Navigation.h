//=============================================================================
// PlayerBehavior_Navigation.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "FTGPlayerBehaviors.h"

/// The basic navigation behavior
class PlayerBehavior_Navigation : public FTGPlayerBehaviorBase
{
	DECLARE_PLAYER_BEHAVIOR(PlayerBehavior_Navigation)
public:
	PlayerBehavior_Navigation();

	void AddAnimation(const std::string& AnimationAsset, float Speed);

	virtual void Update(FTGPlayerStateMachine* StateMachine, float DeltaTime) override;
	virtual void EvaluatePose(RAnimPoseData& PoseData) override;

	virtual std::string GetDebugString() const override;

protected:
	virtual void OnCacheAnimations(RMesh& SkinnedMesh) override;

private:
	/// Get a float number representing the index (decimal part) and the blend factor (fraction part) of current animation selection
	float EvaluateAnimRelevancyFactor() const;

	/// Get relevant animations from index and blend factor.
	/// Returns the fraction of blend factor.
	float GetRelevantAnimations(float InRelevancyFactor, RAnimation** OutAnim0, RAnimation** OutAnim1) const;

	struct NavigationAnimData
	{
		NavigationAnimData(RAnimation* InAnim, float InSpeed)
			: Animation(InAnim)
			, Speed(InSpeed)
		{
		}

		RAnimation* Animation;
		float Speed;
	};

	std::vector<NavigationAnimData> AnimData;

	float CurrentSpeed;
	float TargetSpeed;

	float NormalizedPlaybackProgress;
	float AnimRelevancyFactor;
};
