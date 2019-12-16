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

struct DebugFunnelData
{
	DebugFunnelData(int InStep, const RVec3& InStart, const RVec3& InCurrent, const RVec3& InLeft, const RVec3& InRight)
		: Step(InStep)
		, Start(InStart)
		, Current(InCurrent)
		, Left(InLeft)
		, Right(InRight)
	{}

	int Step;
	RVec3 Start;
	RVec3 Current;
	RVec3 Left;
	RVec3 Right;
};

class RNavMeshDebugger
{
public:
	RNavMeshDebugger();

	bool Initialize(RNavMeshGenerator* InNavMeshGen, RNavMeshData* InNavMeshData);

	// Draw debug information for a given region
	void DrawRegion(int RegionId) const;

	void AddRegionEdge(int RegionId, const RVec3& Start, const RVec3& End);

	void AddFunnelStep(int Step, const RVec3& InStart, const RVec3& InCurrent, const RVec3& InLeft, const RVec3& InRight);

	void ClearFunnelResult();

	void DrawFunnel(int Step) const;

	int GetMaxStepIndex() const;

	void AddPersistentLine(const RVec3& Start, const RVec3& End);

private:
	std::vector<std::vector<DebugLineData>> RegionEdgeLines;
	std::vector<DebugLineData> PersistentLines;

	// Funnel debug info
	std::vector<DebugFunnelData> DebugFunnel;

	RNavMeshGenerator*	NavMeshGen;
	RNavMeshData*		NavMeshData;
};


FORCEINLINE void RNavMeshDebugger::AddRegionEdge(int RegionId, const RVec3& Start, const RVec3& End)
{
	if (RegionEdgeLines.size() < RegionId + 1)
	{
		RegionEdgeLines.resize(RegionId + 1);
	}

	auto& DebugLines = RegionEdgeLines[RegionId];
	DebugLines.emplace(DebugLines.end(), Start, End);
}

FORCEINLINE void RNavMeshDebugger::AddPersistentLine(const RVec3& Start, const RVec3& End)
{
	PersistentLines.emplace(PersistentLines.end(), Start, End);
}
