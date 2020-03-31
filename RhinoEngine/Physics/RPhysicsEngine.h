//=============================================================================
// RPhysicsEngine.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Core/RSingleton.h"
#include "Core/CoreTypes.h"

struct RPhysicsEngineContext;

class RPhysicsEngine : public RSingleton<RPhysicsEngine>
{
	friend class RSingleton<RPhysicsEngine>;
public:
	RPhysicsEngine();
	~RPhysicsEngine();

	bool Initialize();
	void Shutdown();

	/// Simulate the physics world
	void Simulate(float DeltaTime);

	/// Get context for physics engine
	RPhysicsEngineContext* GetContext() const;

	static float GetFixedFrameRate();
	static float GetFixedTimeStep();

private:
	std::unique_ptr<RPhysicsEngineContext>	Context;
};

FORCEINLINE RPhysicsEngineContext* RPhysicsEngine::GetContext() const
{
	return Context.get();
}

#define GPhysicsEngine RPhysicsEngine::Instance()
