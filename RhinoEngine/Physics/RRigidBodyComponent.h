//=============================================================================
// RRigidBodyComponent.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Scene/RSceneComponent.h"

struct RPhysicsObjectContext;

class RRigidBodyComponent : public RSceneComponent
{
	DECLARE_SCENE_COMPONENT(RRigidBodyComponent, RSceneComponent);
public:
	RRigidBodyComponent(RSceneObject* InOwner);
	virtual ~RRigidBodyComponent();

	virtual void Update(float DeltaTime) override;

	void SetMass(float InMass);

	void SetMovable(bool bMovable);

protected:
	virtual void OnComponentAdded() override;

private:
	void SetPhysicsBodyMass(float InMass);

	void BuildTriangleMeshCollision(RMesh* Mesh);

private:
	std::unique_ptr<RPhysicsObjectContext> Context;
};
