//=============================================================================
// RAnimNode_Base.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "RAnimNode_Base.h"
#include "RenderSystem/RMesh.h"
#include "RenderSystem/RDebugRenderer.h"

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
#define DEBUG_DRAW_BONES 0

#if 0
	// -- Evaluate bone poses in mesh space --

	// The method is deprecated as it doesn't allow per bone based transform tweaking.
	// The code is left here for reference. To re-enable this block, also change
	// the bone pose evaluation in RAnimation::EvaluatePoseAtTime from mesh space
	// to local space.
	for (int i = 0; i < SkinnedMesh->GetBoneCount(); i++)
	{
		// This is the final world transform of the bone
		RMatrix4 WorldTransform = BoneMatrices[i] * ObjectToWorld;

#if DEBUG_DRAW_BONES == 1
		GDebugRenderer.DrawSphere(WorldTransform.GetTranslation(), 1.0f, 4);
#endif	// DEBUG_DRAW_BONES

		// Convert to relative transform in binding pose space
		OutMetrics[i] = SkinnedMesh->GetBoneInitInvMatrices(i) * WorldTransform;
	}
#else
	// -- Evaluate bone poses in local space --

	// Bools indicating if a matrix has been set
	std::vector<bool> bMatrixSet(SkinnedMesh->GetBoneCount(), false);

	// Bone matrices in world space
	std::vector<RMatrix4> WorldSpaceMatrix(SkinnedMesh->GetBoneCount());
	const SkeletalData& MeshSkelData = SkinnedMesh->GetSkeletalData();
	for (int i = 0; i < SkinnedMesh->GetBoneCount(); i++)
	{
		// This is the bone id from skinned mesh
		int ParentId = MeshSkelData.FindParentForBone(i);
		if (ParentId == -1)
		{
			WorldSpaceMatrix[i] = BoneMatrices[i] * ObjectToWorld;
		}
		else
		{
			assert(bMatrixSet[ParentId]);
			WorldSpaceMatrix[i] = BoneMatrices[i] * WorldSpaceMatrix[ParentId];

#if DEBUG_DRAW_BONES == 1
			GDebugRenderer.DrawLine(WorldSpaceMatrix[i].GetTranslation(), WorldSpaceMatrix[ParentId].GetTranslation());
#endif	// DEBUG_DRAW_BONES
		}

#if DEBUG_DRAW_BONES == 1
		GDebugRenderer.DrawSphere(WorldSpaceMatrix[i].GetTranslation(), 1.0f, 4);
#endif	// DEBUG_DRAW_BONES
		bMatrixSet[i] = true;
	}

	for (int i = 0; i < SkinnedMesh->GetBoneCount(); i++)
	{
		WorldSpaceMatrix[i] = SkinnedMesh->GetBoneInitInvMatrices(i) * WorldSpaceMatrix[i];
	}

	// Copy all bone results
	memcpy(OutMetrics, WorldSpaceMatrix.data(), sizeof(RMatrix4) * SkinnedMesh->GetBoneCount());
#endif
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

bool RAnimNode_Base::BindAnimVariable(const std::string& VariableName, float* ValuePtr)
{
	return false;
}
