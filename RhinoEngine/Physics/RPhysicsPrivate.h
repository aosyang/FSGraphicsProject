//=============================================================================
// RPhysicsPrivate.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "btBulletDynamicsCommon.h"

struct RPhysicsEngineContext
{
	RPhysicsEngineContext()
	{}

	std::unique_ptr<btDefaultCollisionConfiguration> CollisionConfiguration;

	// The default collision dispatcher
	std::unique_ptr<btCollisionDispatcher> Dispatcher;

	// The broadphase
	std::unique_ptr<btBroadphaseInterface> OverlappingPairCache;

	// The default constraint solver
	std::unique_ptr<btSequentialImpulseConstraintSolver> Solver;

	// The physics for simulation
	std::unique_ptr<btDiscreteDynamicsWorld> DynamicWorld;
};

struct RPhysicsObjectContext
{
	RPhysicsObjectContext()
		: BoxHalfSize(-0.5f, -0.5f, -0.5f)
		, Mass(-1)
	{}

	std::unique_ptr<btCollisionShape> Shape;
	std::unique_ptr<btMotionState> MotionState;
	std::unique_ptr<btRigidBody> Body;
	RVec3 BoxHalfSize;
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
