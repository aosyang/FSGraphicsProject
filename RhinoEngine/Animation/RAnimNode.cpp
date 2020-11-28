//=============================================================================
// RAnimNode.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "RAnimNode.h"
#include "RenderSystem/RMesh.h"

RAnimPoseData::RAnimPoseData(const RMesh& InSkinnedMesh) : SkinnedMesh(&InSkinnedMesh)
{
	BoneMatrices.resize(SkinnedMesh->GetBoneCount(), RMatrix4::IDENTITY);
}

void RAnimPoseData::CopyFinalPose(const RMatrix4& ObjectToWorld, RMatrix4* OutMetrics)
{
	for (int i = 0; i < SkinnedMesh->GetBoneCount(); i++)
	{
		OutMetrics[i] = SkinnedMesh->GetBoneInitInvMatrices(i) * BoneMatrices[i] * ObjectToWorld;
	}
}

RAnimPoseData RAnimPoseData::BlendTwoPoses(const RAnimPoseData& Pose1, const RAnimPoseData& Pose2, float BlendFactor)
{
	// Two poses must share the same skinned mesh
	assert(Pose1.SkinnedMesh == Pose2.SkinnedMesh);

	RAnimPoseData OutPose(*Pose1.SkinnedMesh);
	for (int i = 0; i < Pose1.SkinnedMesh->GetBoneCount(); i++)
	{
		OutPose.BoneMatrices[i] = RMatrix4::Slerp(Pose1.BoneMatrices[i], Pose2.BoneMatrices[i], BlendFactor);
	}
	return OutPose;
}

