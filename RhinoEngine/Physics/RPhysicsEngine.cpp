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
	Context->OverlappingPairCache = std::make_unique<btDbvtBroadphase>();
	Context->Solver = std::make_unique<btSequentialImpulseConstraintSolver>();
	Context->DynamicWorld = std::make_unique<btDiscreteDynamicsWorld>(Context->Dispatcher.get(), Context->OverlappingPairCache.get(), Context->Solver.get(), Context->CollisionConfiguration.get());

	Context->DynamicWorld->setGravity(btVector3(0, -1000, 0));

	return true;
}

void RPhysicsEngine::Shutdown()
{
}

void RPhysicsEngine::Simulate(float DeltaTime)
{
	Context->DynamicWorld->stepSimulation(DeltaTime, 10);
}
