//=============================================================================
// RPhysicsEngine.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "RPhysicsEngine.h"

#include "RPhysicsPrivate.h"

RPhysicsEngine::RPhysicsEngine()
	: Context(std::make_unique<RPhysicsEngineContext>())
{

}

RPhysicsEngine::~RPhysicsEngine() = default;

bool RPhysicsEngine::Initialize()
{
	Context->CollisionConfiguration = std::make_unique<btDefaultCollisionConfiguration>();
	Context->Dispatcher = std::make_unique<btCollisionDispatcher>(Context->CollisionConfiguration.get());
	Context->Broadphase = std::make_unique<btDbvtBroadphase>();
	Context->Solver = std::make_unique<btSequentialImpulseConstraintSolver>();
	Context->DynamicWorld = std::make_unique<btDiscreteDynamicsWorld>(Context->Dispatcher.get(), Context->Broadphase.get(), Context->Solver.get(), Context->CollisionConfiguration.get());

	// Set up pair callback for default collision behavior on character controllers
	Context->GhostPairCallback = std::make_unique<btGhostPairCallback>();
	Context->Broadphase->getOverlappingPairCache()->setInternalGhostPairCallback(Context->GhostPairCallback.get());

	Context->DynamicWorld->setGravity(btVector3(0, -1000, 0));

	return true;
}

void RPhysicsEngine::Shutdown()
{
	Context->Broadphase->getOverlappingPairCache()->setInternalGhostPairCallback(nullptr);
}

void RPhysicsEngine::Simulate(float DeltaTime)
{
	Context->DynamicWorld->stepSimulation(DeltaTime, 10, GetFixedTimeStep());
}

float RPhysicsEngine::GetFixedFrameRate()
{
	return 30.0f;
}

float RPhysicsEngine::GetFixedTimeStep()
{
	const float FixedTimeStep = 1.0f / GetFixedFrameRate();
	return FixedTimeStep;
}
