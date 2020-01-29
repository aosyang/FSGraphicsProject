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
		RScopeInternalTransformUpdate InternalTransformUpdate(Owner);

		btTransform PhysicsTransform;
		if (Context->Body && Context->MotionState)
		{
			Context->MotionState->getWorldTransform(PhysicsTransform);
		}
		else
		{
			PhysicsTransform.setIdentity();
		}

		// Half box size already contains scale transform so the transform has scales of 1 for debug rendering
		RTransform Transform(btVec3ToRVec3(PhysicsTransform.getOrigin()), btQuatToRQuat(PhysicsTransform.getRotation()));
		GDebugRenderer.DrawBox(Context->BoxHalfSize * 2.0f, Transform.GetMatrix());

		// Now we apply the scale to the object and translate it back by its offset to the box center
		Transform.SetScale(Owner->GetScale());
		Transform.Translate(-Context->BoxOffset, ETransformSpace::Local);
		Owner->SetTransform(Transform);
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
			if (Mesh->GetCollisionType() == EMeshCollisionType::TriangleMesh)
			{
				BuildTriangleMeshCollision(Mesh);

				// Note: Triangle mesh can't be dynamic
				Context->Mass = 0.0f;
			}
			else
			{
				Context->BoxHalfSize = Mesh->GetLocalSpaceAabb().GetLocalDimension() * Owner->GetScale() / 2.0f;
				Context->BoxOffset = Mesh->GetLocalSpaceAabb().GetCenter();
		
				btVector3 BoxHalfSize(RVec3TobtVec3(Context->BoxHalfSize));
				Context->Shape = std::make_unique<btBoxShape>(BoxHalfSize);

				Context->Mass = 1.0f;
			}
		}
		else
		{
			Context->BoxHalfSize = Owner->GetAabb().GetLocalDimension() / 2.0f;
			Context->BoxOffset = Owner->GetAabb().GetCenter() - Owner->GetWorldPosition();

			btVector3 BoxHalfSize(RVec3TobtVec3(Context->BoxHalfSize));
			Context->Shape = std::make_unique<btBoxShape>(BoxHalfSize);

			Context->Mass = 1.0f;
		}

		btTransform InitTransform;
		InitTransform.setOrigin(RVec3TobtVec3(Owner->GetWorldPosition() + Owner->GetRotation() * Context->BoxOffset));
		InitTransform.setRotation(RQuatTobtQuat(Owner->GetRotation()));

		btVector3 LocalInertia(0.0f, 0.0f, 0.0f);
		if (Context->Mass != 0.0f)
		{
			Context->Shape->calculateLocalInertia(Context->Mass, LocalInertia);
		}

		Context->MotionState = std::make_unique<btDefaultMotionState>(InitTransform);
		btRigidBody::btRigidBodyConstructionInfo BodyInfo(Context->Mass, Context->MotionState.get(), Context->Shape.get(), LocalInertia);
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

void RRigidBodyComponent::BuildTriangleMeshCollision(RMesh* Mesh)
{
	// Physics bodies CAN'T be scaled so we must create triangle meshes for objects with each scale
	// TODO: Reuse triangle meshes for multiple rigid bodies with the same scale
	Context->TriangleMesh = std::make_unique<btTriangleMesh>();
	RVec3 Scale = GetOwner()->GetScale();

	for (const RMeshElement& MeshElement : Mesh->GetMeshElements())
	{
		for (int i = 0; i < MeshElement.TriangleIndices.size(); i += 3)
		{
			int idx0 = MeshElement.TriangleIndices[i];
			int idx1 = MeshElement.TriangleIndices[i + 1];
			int idx2 = MeshElement.TriangleIndices[i + 2];

			Context->TriangleMesh->addTriangle(
				RVec3TobtVec3(RVec3(&MeshElement.PositionArray[idx0].x) * Scale),
				RVec3TobtVec3(RVec3(&MeshElement.PositionArray[idx1].x) * Scale),
				RVec3TobtVec3(RVec3(&MeshElement.PositionArray[idx2].x) * Scale)
			);
		}
	}

	Context->Shape = std::make_unique<btBvhTriangleMeshShape>(Context->TriangleMesh.get(), true, true);
}
