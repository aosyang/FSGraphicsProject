//=============================================================================
// RAnimNode_BlendByCondition.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "RAnimNode_Base.h"


class RAnimNode_BlendByCondition : public RAnimNode_Base
{
public:
	DECLARE_ANIM_NODE_FACTORY_METHOD(RAnimNode_BlendByCondition)

	RAnimNode_BlendByCondition(const std::string& InNodeName, const AnimNodeAttributeMap& Attributes);

	virtual void UpdateNode(float DeltaTime) override;
	virtual void EvaluatePose(RAnimPoseData& PoseData) override;

	virtual bool BindAnimVariable(const std::string& VariableName, bool* ValuePtr) override;

private:
	AnimVariable<bool> bCondition;

	float BlendTime;
	float CurrentBlendFactor;
};
