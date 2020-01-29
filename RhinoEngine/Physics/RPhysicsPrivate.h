//=============================================================================
// RPhysicsPrivate.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"

struct RPhysicsEngineContext
{
	RPhysicsEngineContext()
	{}

	std::unique_ptr<btDefaultCollisionConfiguration> CollisionConfiguration;

	// The default collision dispatcher
	std::unique_ptr<btCollisionDispatcher> Dispatcher;

	// The broadphase used by physics simulation
	std::unique_ptr<btBroadphaseInterface> Broadphase;

	// The default behavior for ghost object collisions
	std::unique_ptr<btGhostPairCallback> GhostPairCallback;

	// The default constraint solver
	std::unique_ptr<btSequentialImpulseConstraintSolver> Solver;

	// The physics world of simulation
	std::unique_ptr<btDiscreteDynamicsWorld> DynamicWorld;
};

struct RPhysicsObjectContext
{
	RPhysicsObjectContext()
		: BoxHalfSize(-0.5f, -0.5f, -0.5f)
		, BoxOffset(0.0f, 0.0f, 0.0f)
		, Mass(-1)
	{}

	std::unique_ptr<btCollisionShape> Shape;
	std::unique_ptr<btMotionState> MotionState;
	std::unique_ptr<btRigidBody> Body;
	RVec3 BoxHalfSize;
	RVec3 BoxOffset;
	float Mass;
};

FORCEINLINE RVec3 btVec3ToRVec3(const btVector3& InVec)
{
	return RVec3(InVec.x(), InVec.y(), InVec.z());
}

FORCEINLINE btVector3 RVec3TobtVec3(const RVec3& InVec)
{
	return btVector3(InVec.X(), InVec.Y(), InVec.Z());
}

FORCEINLINE RQuat btQuatToRQuat(const btQuaternion& InQuat)
{
	return RQuat(InQuat.w(), InQuat.x(), InQuat.y(), InQuat.z());
}

FORCEINLINE btQuaternion RQuatTobtQuat(const RQuat& InQuat)
{
	return btQuaternion(InQuat.x, InQuat.y, InQuat.z, InQuat.w);
}
