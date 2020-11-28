//=============================================================================
// RAnimNode.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Core/CoreTypes.h"

class RMesh;

// Animation pose data contains all the bone transformations of a skinned mesh at one moment in time.
struct RAnimPoseData
{
	RAnimPoseData(const RMesh& InSkinnedMesh);

	RAnimPoseData(const RAnimPoseData& Other)
		: SkinnedMesh(Other.SkinnedMesh)
		, BoneMatrices(Other.BoneMatrices)
	{
	}

	RAnimPoseData(RAnimPoseData&& Other)
		: SkinnedMesh(std::move(Other.SkinnedMesh))
		, BoneMatrices(std::move(Other.BoneMatrices))
	{
	}

	RAnimPoseData& operator=(const RAnimPoseData& Other)
	{
		SkinnedMesh = Other.SkinnedMesh;
		BoneMatrices = Other.BoneMatrices;
		return *this;
	}

	RAnimPoseData& operator=(RAnimPoseData&& Other)
	{
		SkinnedMesh = std::move(Other.SkinnedMesh);
		BoneMatrices = std::move(Other.BoneMatrices);
		return *this;
	}

	// Convert all metrics into world space and output to the metrics array
	void CopyFinalPose(const RMatrix4& ObjectToWorld, RMatrix4* OutMetrics);

	// Blend together two poses
	static RAnimPoseData BlendTwoPoses(const RAnimPoseData& Pose1, const RAnimPoseData& Pose2, float BlendFactor);

	// The skinned mesh used in pose evaluation
	const RMesh* SkinnedMesh;

	// Transforms for each bone in object space
	std::vector<RMatrix4> BoneMatrices;
};

// The base class for all animation nodes
class RAnimNode
{
public:
	virtual ~RAnimNode() {}

	// Evaluate pose for this node and any ancestor nodes
	virtual void EvaluatePose(RAnimPoseData& PoseData) { }
};
