//=============================================================================
// RVoxelizerDebugger.cpp by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RNavMeshGenDebugger.h"

void RNavMeshGenDebugger::DrawRegion(int RegionId)
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
