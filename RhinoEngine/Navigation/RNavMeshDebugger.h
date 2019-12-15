//=============================================================================
// RVoxelizerDebugger.h by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Core/CoreTypes.h"

class RNavMeshGenerator;
class RNavMeshData;

struct DebugLineData
{
	DebugLineData(const RVec3& InStart, const RVec3& InEnd)
		: Start(InStart)
		, End(InEnd)
	{}

	RVec3 Start;
	RVec3 End;
};

class RNavMeshDebugger
{
public:
	RNavMeshDebugger();

	bool Initialize(RNavMeshGenerator* InNavMeshGen, RNavMeshData* InNavMeshData);

	void DrawRegion(int RegionId) const;

	void AddRegionEdge(int RegionId, const RVec3& Start, const RVec3& End);

	void AddPersistentLine(const RVec3& Start, const RVec3& End);

private:
	std::vector<std::vector<DebugLineData>> RegionDebugLines;
	std::vector<DebugLineData> PersistentLines;

	RNavMeshGenerator*	NavMeshGen;
	RNavMeshData*		NavMeshData;
};


FORCEINLINE void RNavMeshDebugger::AddRegionEdge(int RegionId, const RVec3& Start, const RVec3& End)
{
	if (RegionDebugLines.size() < RegionId + 1)
	{
		RegionDebugLines.resize(RegionId + 1);
	}

	auto& PersistentDebugLines = RegionDebugLines[RegionId];
	PersistentDebugLines.emplace(PersistentDebugLines.end(), Start, End);
}

FORCEINLINE void RNavMeshDebugger::AddPersistentLine(const RVec3& Start, const RVec3& End)
{
	PersistentLines.emplace(PersistentLines.end(), Start, End);
}
