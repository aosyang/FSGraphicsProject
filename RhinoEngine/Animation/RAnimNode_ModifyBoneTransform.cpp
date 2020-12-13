//=============================================================================
// RAnimNode_ModifyBoneTransform.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "RAnimNode_ModifyBoneTransform.h"
#include "Core/StringUtils.h"
#include "RenderSystem/RMesh.h"


RAnimNode_ModifyBoneTransform::RAnimNode_ModifyBoneTransform(const std::string& InNodeName, const AnimNodeAttributeMap& Attributes)
	: RAnimNode_Base(InNodeName, Attributes)
	, BoneIndex(-1)
	, bBoneIndexCached(false)
{
	for (auto& Iter : Attributes.Map)
	{
		if (StringUtils::EqualsIgnoreCase(Iter.first, "BoneName"))
		{
			BoneName = Iter.second;
		}
	}
}

void RAnimNode_ModifyBoneTransform::UpdateNode(float DeltaTime)
{
	RAnimNode_Base::UpdateNode(DeltaTime);

	BoneMatrix.UpdateVal();
}

void RAnimNode_ModifyBoneTransform::EvaluatePose(RAnimPoseData& PoseData)
{
	if (RAnimNode_Base* InputNode = GetInputNodeAtIndex(0))
	{
		if (!bBoneIndexCached)
		{
			BoneIndex = PoseData.SkinnedMesh->FindBoneByName(BoneName);
			bBoneIndexCached = true;
		}

		InputNode->EvaluatePose(PoseData);

		if (BoneIndex != -1)
		{
			PoseData.BoneMatrices[BoneIndex] *= BoneMatrix;
		}
	}
	else
	{
		PoseData.ResetPose();
	}
}

bool RAnimNode_ModifyBoneTransform::BindAnimVariable(const std::string& VariableName, RMatrix4* ValuePtr)
{
	if (StringUtils::EqualsIgnoreCase(VariableName, "Transform"))
	{
		BoneMatrix.SourcePtr = ValuePtr;
		return true;
	}

	return false;
}
