//=============================================================================
// AIBehavior_Roamer.h by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "FTGPlayerController.h"

class AIBehavior_Roamer : public RSceneComponentBase
{
	DECLARE_SCENE_COMPONENT(AIBehavior_Roamer, RSceneComponentBase);
public:
	AIBehavior_Roamer(RSceneObject* InOwner);

	virtual void Update(float DeltaTime) override;

private:
	RAabb SceneBounds;
	FTGPlayerController* ControlledPlayer;
	RVec3 MoveTarget;
	std::vector<RVec3> NavPath;
};
