#pragma once

#include "Scene/RSceneComponentBase.h"

class RMeshComponent : public RSceneComponentBase
{
	typedef RSceneComponentBase Base;
public:
	/// Component creation function
	static RMeshComponent* Create(RSceneObject* InOwner);

	RMeshComponent(RSceneObject* InOwner);
};