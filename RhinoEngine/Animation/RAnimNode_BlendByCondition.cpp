//=============================================================================
// RAnimNode_BlendByCondition.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================


#include "RAnimNode_BlendByCondition.h"
#include "Core/StringUtils.h"


RAnimNode_BlendByCondition::RAnimNode_BlendByCondition(const std::string& InNodeName, const AnimNodeAttributeMap& Attributes)
	: RAnimNode_Base(InNodeName, Attributes)
	, bCondition(false)
	, BlendTime(0.2f)
	, CurrentBlendFactor(0.0f)
{

}

void RAnimNode_BlendByCondition::UpdateNode(float DeltaTime)
{
	RAnimNode_Base::UpdateNode(DeltaTime);

	bCondition.UpdateVal();

	if (bCondition)
	{
		if (CurrentBlendFactor < 1.0f)
		{
			CurrentBlendFactor = RMath::Min(1.0f, CurrentBlendFactor + DeltaTime / BlendTime);
		}
	}
	else
	{
		if (CurrentBlendFactor > 0.0f)
		{
			CurrentBlendFactor = RMath::Max(0.0f, CurrentBlendFactor - DeltaTime / BlendTime);
		}
	}
}

void RAnimNode_BlendByCondition::EvaluatePose(RAnimPoseData& PoseData)
{
	if (CurrentBlendFactor == 0.0f || CurrentBlendFactor == 1.0f)
	{
		int InputIndex = CurrentBlendFactor == 1.0f ? 0 : 1;
		if (RAnimNode_Base* InputNode = GetInputNodeAtIndex(InputIndex))
		{
			InputNode->EvaluatePose(PoseData);
		}
		else
		{
			PoseData.ResetPose();
		}
	}
	else
	{
		RAnimNode_Base* InputNode0 = GetInputNodeAtIndex(0);
		RAnimNode_Base* InputNode1 = GetInputNodeAtIndex(1);
		if (InputNode0 && InputNode1)
		{
			RAnimPoseData Pose0(*PoseData.SkinnedMesh), Pose1(*PoseData.SkinnedMesh);
			InputNode0->EvaluatePose(Pose0);
			InputNode1->EvaluatePose(Pose1);

			PoseData = RAnimPoseData::BlendTwoPoses(Pose0, Pose1, 1.0f - CurrentBlendFactor);
		}
		else
		{
			PoseData.ResetPose();
		}
	}
}

bool RAnimNode_BlendByCondition::BindAnimVariable(const std::string& VariableName, bool* ValuePtr)
{
	if (StringUtils::EqualsIgnoreCase(VariableName, "Condition"))
	{
		bCondition.SourcePtr = ValuePtr;
		return true;
	}

	return false;
}

