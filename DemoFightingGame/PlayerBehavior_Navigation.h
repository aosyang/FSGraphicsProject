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
	virtual bool EvaluatePose(const RMesh& SkinnedMesh, RMatrix4* OutBoneMatrices) override;

protected:
	virtual void OnCacheAnimations(RMesh& SkinnedMesh) override;

private:
	/// Get a float number representing the index (decimal part) and the blend factor (fraction part) of current animation selection
	float EvaluateIndexAndBlendFactor() const;

	/// Get relevant animations from index and blend factor.
	/// Returns the fraction of blend factor.
	float GetRelevantAnimations(float IndexAndBlendFactor, RAnimation** OutAnim0, RAnimation** OutAnim1) const;

	float GetAnimStartTime(float IndexAndBlendFactor) const;

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

	float NormalizedPlaybackProgress;
	float IndexAndBlendFactor;
};
