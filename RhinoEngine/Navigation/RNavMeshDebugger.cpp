//=============================================================================
// RVoxelizerDebugger.cpp by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RNavMeshDebugger.h"

RNavMeshDebugger::RNavMeshDebugger()
	: NavMeshGen(nullptr)
	, NavMeshData(nullptr)
{

}

bool RNavMeshDebugger::Initialize(RNavMeshGenerator* InNavMeshGen, RNavMeshData* InNavMeshData)
{
	NavMeshGen = InNavMeshGen;
	assert(NavMeshGen);

	NavMeshData = InNavMeshData;
	assert(NavMeshData);

	return true;
}

void RNavMeshDebugger::DrawRegion(int RegionId) const
{
	if (RegionId >= 0 && RegionId < (int)RegionDebugLines.size())
	{
		for (const auto& Line : RegionDebugLines[RegionId])
		{
			GDebugRenderer.DrawLine(Line.Start, Line.End, RColor::Green);
		}
	}

	for (const auto& Line : PersistentLines)
	{
		GDebugRenderer.DrawLine(Line.Start, Line.End, RColor::Red);
	}
}
