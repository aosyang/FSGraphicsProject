//=============================================================================
// RNavigationSystem.h by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "RNavMeshGenerator.h"
#include "RNavMeshData.h"
#include "RNavMeshDebugger.h"

class RNavigationSystem : public RSingleton<RNavigationSystem>
{
	friend class RSingleton<RNavigationSystem>;
public:
	bool Initialize();

	// Build navmesh data from a scene
	void BuildNavMesh(const RScene* Scene);

	// Query a path on navmesh
	bool QueryPath(const RVec3& Start, const RVec3& Goal, std::vector<RVec3>& OutPath);

	RNavMeshDebugger& GetDebugger();
	const RNavMeshDebugger& GetDebugger() const;

	void DebugRender() const;

	void DebugProjectPointToNavmesh(const RVec3& Point) const;
	void DebugSetGoalPoint(const RVec3& Point);

private:
	void DebugDrawPathQuery() const;

private:
	RNavMeshGenerator	NavMeshGenerator;
	RNavMeshData		NavMeshData;

	RNavMeshDebugger	NavMeshDebugger;

	// Debug variables
	RVec3 QueryStart;
	RVec3 QueryGoal;
	std::vector<RVec3> TestPath;
};

#define GNavigationSystem RNavigationSystem::Instance()

FORCEINLINE RNavMeshDebugger& RNavigationSystem::GetDebugger()
{
	return NavMeshDebugger;
}

FORCEINLINE const RNavMeshDebugger& RNavigationSystem::GetDebugger() const
{
	return NavMeshDebugger;
}
