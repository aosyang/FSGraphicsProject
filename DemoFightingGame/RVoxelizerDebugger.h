//=============================================================================
// RVoxelizerDebugger.h by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Core/CoreTypes.h"

struct DebugLineData
{
	DebugLineData(const RVec3& InStart, const RVec3& InEnd)
		: Start(InStart)
		, End(InEnd)
	{}

	RVec3 Start;
	RVec3 End;
};

class RVoxelizerDebugger
{
public:
	void DrawRegion(int RegionId);

	void AddRegionEdge(int RegionId, const RVec3& Start, const RVec3& End);

private:
	std::vector<std::vector<DebugLineData>> RegionDebugLines;
};


FORCEINLINE void RVoxelizerDebugger::AddRegionEdge(int RegionId, const RVec3& Start, const RVec3& End)
{
	if (RegionDebugLines.size() < RegionId + 1)
	{
		RegionDebugLines.resize(RegionId + 1);
	}

	auto& PersistentDebugLines = RegionDebugLines[RegionId];
	PersistentDebugLines.emplace(PersistentDebugLines.end(), Start, End);
}
