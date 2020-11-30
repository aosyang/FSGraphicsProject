//=============================================================================
// RAnimNode_Base.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "RAnimNode_Base.h"
#include "RenderSystem/RMesh.h"

bool AnimNodeAttributeMap::Query(const AttributeMap& Map, const std::string& KeyName, std::string& OutValue)
{
	if (Map.find(KeyName) != Map.end())
	{
		OutValue = Map.at(KeyName);
		return true;
	}

	return false;
}

bool AnimNodeAttributeMap::QueryBool(const AttributeMap& Map, const std::string& KeyName, bool& OutBool)
{
	std::string Value;
	if (Query(Map, KeyName, Value))
	{
		for (auto& c : Value)
		{
			c = tolower(c);
		}

		static const std::string StrLowerTrue("true");
		OutBool = (Value == StrLowerTrue);
		return true;
	}

	return false;
}

bool AnimNodeAttributeMap::QueryFloat(const AttributeMap& Map, const std::string& KeyName, float& OutFloat)
{
	std::string Value;
	if (Query(Map, KeyName, Value))
	{
		OutFloat = std::stof(Value);
		return true;
	}

	return false;
}

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

RAnimNode_Base::RAnimNode_Base()
{
}

RAnimNode_Base::RAnimNode_Base(const std::string& InNodeName, const AnimNodeAttributeMap& Attributes)
	: NodeName(InNodeName)
{
}

RAnimNode_Base::~RAnimNode_Base()
{
}

void RAnimNode_Base::UpdateNode(float DeltaTime)
{
}

void RAnimNode_Base::EvaluatePose(RAnimPoseData& PoseData)
{
}

bool RAnimNode_Base::BindAnimVariable(const std::string& VariableName, float* ValPtr)
{
	return false;
}
