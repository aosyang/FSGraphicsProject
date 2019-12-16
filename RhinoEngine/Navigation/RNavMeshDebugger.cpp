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
	if (RegionId >= 0 && RegionId < (int)RegionEdgeLines.size())
	{
		const auto& RegionEdges = NavMeshGen->RegionEdgePoints[RegionId];

		// Debug draw edges of each region
		for (int i = 0; i < RegionEdges.size(); i++)
		{
			const RVec3& p0 = RegionEdges[i].Point;
			const RVec3& p1 = RegionEdges[(i + 1) % RegionEdges.size()].Point;
			GDebugRenderer.DrawLine(p0, p1, RColor::Cyan);

			if (RegionEdges[i].bIsMandatory)
			{
				GDebugRenderer.DrawSphere(p0, 20.0f, RColor::Cyan, 8);
			}
			else
			{
				GDebugRenderer.DrawSphere(p0, 10.0f, RColor::Green, 8);
			}
		}

		for (const auto& Line : RegionEdgeLines[RegionId])
		{
			GDebugRenderer.DrawLine(Line.Start, Line.End, RColor::Green);
		}
	}

	for (const auto& Line : PersistentLines)
	{
		GDebugRenderer.DrawLine(Line.Start, Line.End, RColor::Red);
	}
}

void RNavMeshDebugger::AddFunnelStep(int Step, const RVec3& InStart, const RVec3& InCurrent, const RVec3& InLeft, const RVec3& InRight)
{
	DebugFunnel.emplace(DebugFunnel.end(), Step, InStart, InCurrent, InLeft, InRight);
}

void RNavMeshDebugger::ClearFunnelResult()
{
	DebugFunnel.clear();
}

void RNavMeshDebugger::DrawFunnel(int Step) const
{
	for (auto& Iter : DebugFunnel)
	{
		if (Iter.Step == Step)
		{
			static const RVec3 Offset(0.0f, 1.0f, 0.0f);

			GDebugRenderer.DrawSphere(Iter.Start + Offset, 10.0f, RColor::Cyan, 8);
			GDebugRenderer.DrawSphere(Iter.Left + Offset, 10.0f, RColor::Red, 8);
			GDebugRenderer.DrawSphere(Iter.Right + Offset, 10.0f, RColor::Green, 8);
			GDebugRenderer.DrawSphere(Iter.Current + Offset, 10.0f, RColor::Blue, 8);
			
			GDebugRenderer.DrawLine(Iter.Start + Offset, Iter.Left + Offset, RColor::Red);
			GDebugRenderer.DrawLine(Iter.Start + Offset, Iter.Right + Offset, RColor::Green);
			GDebugRenderer.DrawLine(Iter.Start + Offset, Iter.Current + Offset, RColor::Blue);

			break;
		}
	}
}

int RNavMeshDebugger::GetMaxStepIndex() const
{
	int Index = -1;
	for (auto& Iter : DebugFunnel)
	{
		if (Iter.Step > Index)
		{
			Index = Iter.Step;
		}
	}

	return Index;
}
