//=============================================================================
// RVoxelizerDebugger.cpp by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RVoxelizerDebugger.h"

void RVoxelizerDebugger::DrawRegion(int RegionId)
{
	for (const auto& Line : RegionDebugLines[RegionId])
	{
		GDebugRenderer.DrawLine(Line.Start, Line.End, RColor::Green);
	}
}
