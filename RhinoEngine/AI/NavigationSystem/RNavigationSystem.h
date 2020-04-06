//=============================================================================
// RNavigationSystem.h by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "RNavMeshGenerator.h"
#include "RNavMeshData.h"
#include "RNavMeshDebugger.h"

#include "Core/RSingleton.h"

enum ENavMeshDebugFlag
{
	NavMeshDebug_DrawNavMesh	= 1 << 0,
	NavMeshDebug_DrawRegions	= 1 << 1,
	NavMeshDebug_DrawFunnel		= 1 << 2,
	NavMeshDebug_DrawSpans		= 1 << 3,
	NavMeshDebug_DrawSolidSpans	= 1 << 4,
	NavMeshDebug_DrawPathQuery	= 1 << 5,
};

class RNavigationSystem : public RSingleton<RNavigationSystem>
{
	friend class RSingleton<RNavigationSystem>;
public:
	bool Initialize();

	// Build navmesh data from a scene
	void BuildNavMesh(const RScene* Scene, INavMeshCellDetector& CellDetector = RDefaultNavMeshCellDetector());

	bool SerializeNavMesh(RSerializer& Serializer);

	// Query a path on navmesh
	bool QueryPath(const RVec3& Start, const RVec3& Goal, std::vector<RVec3>& OutPath);

	RNavMeshDebugger& GetDebugger();
	const RNavMeshDebugger& GetDebugger() const;

	void DebugRender(int DebugFlags) const;

	void DebugProjectPointToNavmesh(const RVec3& Point) const;
	void DebugSetPathQueryPoints(const RVec3& Start, const RVec3& Goal);

	static const RVec3 InvalidPosition;

private:
	void DebugDrawPathQuery() const;

	void DebugDrawNavMesh() const;

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
