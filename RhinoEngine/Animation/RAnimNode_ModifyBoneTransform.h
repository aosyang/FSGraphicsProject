//=============================================================================
// RAnimNode_ModifyBoneTransform.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "RAnimNode_Base.h"

// Anim node that modifies the transform of a bone
class RAnimNode_ModifyBoneTransform : public RAnimNode_Base
{
public:
	DECLARE_ANIM_NODE_FACTORY_METHOD(RAnimNode_ModifyBoneTransform)

	RAnimNode_ModifyBoneTransform(const std::string& InNodeName, const AnimNodeAttributeMap& Attributes);

	virtual void UpdateNode(float DeltaTime) override;
	virtual void EvaluatePose(RAnimPoseData& PoseData) override;

	virtual bool BindAnimVariable(const std::string& VariableName, RMatrix4* ValuePtr) override;

private:
	std::string BoneName;
	int BoneIndex;
	bool bBoneIndexCached;

	AnimVariable<RMatrix4> BoneMatrix;
};
