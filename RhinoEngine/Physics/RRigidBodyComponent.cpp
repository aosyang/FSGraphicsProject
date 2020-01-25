//=============================================================================
// RRigidBodyComponent.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RRigidBodyComponent.h"

#include "RPhysicsPrivate.h"
#include "RPhysicsEngine.h"
#include "Scene/RSceneObject.h"

RRigidBodyComponent::RRigidBodyComponent(RSceneObject* InOwner)
	: Base(InOwner)
	, Context(std::make_unique<RPhysicsObjectContext>())
{

}

RRigidBodyComponent::~RRigidBodyComponent()
{
	RPhysicsEngineContext* PhysicsEngineContext = GPhysicsEngine.GetContext();

	if (Context->Body)
	{
		PhysicsEngineContext->DynamicWorld->removeCollisionObject(Context->Body.get());
	}
}

void RRigidBodyComponent::Update(float DeltaTime)
{
	RSceneObject* Owner = GetOwner();
	if (Owner)
	{
		btTransform PhysicsTransform;
		if (Context->Body && Context->MotionState)
		{
			Context->MotionState->getWorldTransform(PhysicsTransform);
		}
		else
		{
			PhysicsTransform.setIdentity();
		}

		Owner->SetPosition(btVec3ToRVec3(PhysicsTransform.getOrigin()));
		Owner->SetRotation(btQuatToRQuat(PhysicsTransform.getRotation()));

		// Debug draw physics object
		RTransform Transform = *Owner->GetTransform();
		Transform.SetScale(RVec3(1.0f, 1.0f, 1.0f));
		GDebugRenderer.DrawBox(Context->BoxHalfSize * 2.0f, Transform.GetMatrix());
	}
}

void RRigidBodyComponent::SetMass(float InMass)
{
	Context->Mass = InMass;
	SetPhysicsBodyMass(InMass);
}

void RRigidBodyComponent::SetMovable(bool bMovable)
{
	if (bMovable)
	{
		SetPhysicsBodyMass(Context->Mass);
	}
	else
	{
		SetPhysicsBodyMass(0.0f);
	}
}

void RRigidBodyComponent::OnComponentAdded()
{
	// Create physics object from aabb
	RSceneObject* Owner = GetOwner();
	if (Owner)
	{
		RPhysicsEngineContext* PhysicsEngineContext = GPhysicsEngine.GetContext();

		// Determine size of the collision box
		RSMeshObject* MeshObject = Owner->CastTo<RSMeshObject>();
		RMesh* Mesh = MeshObject ? MeshObject->GetMesh() : nullptr;
		if (Mesh)
		{
			Context->BoxHalfSize = Mesh->GetLocalSpaceAabb().GetLocalDimension() * Owner->GetScale() / 2.0f;
		}
		else
		{
			Context->BoxHalfSize = Owner->GetAabb().GetLocalDimension() / 2.0f;
		}

		btVector3 BoxHalfSize(RVec3TobtVec3(Context->BoxHalfSize));
		Context->Shape = std::make_unique<btBoxShape>(BoxHalfSize);

		btTransform InitTransform;
		InitTransform.setOrigin(RVec3TobtVec3(Owner->GetWorldPosition()));
		InitTransform.setRotation(RQuatTobtQuat(Owner->GetRotation()));

		btScalar Mass(1.0f);
		Context->Mass = Mass;

		btVector3 LocalInertia(0.0f, 0.0f, 0.0f);
		if (Mass != 0.0f)
		{
			Context->Shape->calculateLocalInertia(Mass, LocalInertia);
		}

		Context->MotionState = std::make_unique<btDefaultMotionState>(InitTransform);
		btRigidBody::btRigidBodyConstructionInfo BodyInfo(Mass, Context->MotionState.get(), Context->Shape.get(), LocalInertia);
		Context->Body = std::make_unique<btRigidBody>(BodyInfo);

		PhysicsEngineContext->DynamicWorld->addRigidBody(Context->Body.get());
	}
}

void RRigidBodyComponent::SetPhysicsBodyMass(float InMass)
{
	btScalar Mass(InMass);

	btVector3 LocalInertia(0.0f, 0.0f, 0.0f);
	if (Mass != 0.0f)
	{
		Context->Shape->calculateLocalInertia(Mass, LocalInertia);
	}

	Context->Body->setMassProps(Mass, LocalInertia);
}
