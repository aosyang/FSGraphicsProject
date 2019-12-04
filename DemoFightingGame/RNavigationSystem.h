//=============================================================================
// RNavigationSystem.h by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "RNavMeshGenerator.h"
#include "RNavMeshData.h"

class RNavigationSystem : public RSingleton<RNavigationSystem>
{
	friend class RSingleton<RNavigationSystem>;
public:
	void BuildNavMesh(const RScene* Scene);

	bool QueryPath(const RVec3& Start, const RVec3& Goal, std::vector<RVec3>& OutPath);

	void DebugRender() const;

	void DebugProjectPointToNavmesh(const RVec3& Point) const;
	void DebugSetGoalPoint(const RVec3& Point);

private:
	void DebugDrawPathQuery() const;

private:
	RNavMeshGenerator	NavMeshGenerator;
	RNavMeshData		NavMeshData;

	// Debug variables
	RVec3 QueryStart;
	RVec3 QueryGoal;
	std::vector<RVec3> TestPath;
};

#define GNavigationSystem RNavigationSystem::Instance()
