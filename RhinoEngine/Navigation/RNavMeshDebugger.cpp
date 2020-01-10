//=============================================================================
// RVoxelizerDebugger.cpp by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RNavMeshDebugger.h"
#include "RNavMeshGenerator.h"

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

void RNavMeshDebugger::AddFunnelStep(int Step, const RVec3& InStart, const RVec3& InCurrent, int EdgeId, const RVec3& InLeft, const RVec3& InRight)
{
	DebugFunnel.emplace(DebugFunnel.end(), Step, InStart, InCurrent, EdgeId, InLeft, InRight);
}

void RNavMeshDebugger::ClearFunnelResult()
{
	DebugFunnel.clear();
}

void RNavMeshDebugger::DrawFunnel(int Index) const
{
	if (Index >= 0 && Index < (int)DebugFunnel.size())
	{
		const auto& Iter = DebugFunnel[Index];
		static const RVec3 Offset(0.0f, 10.0f, 0.0f);

		GDebugRenderer.DrawSphere(Iter.Start + Offset, 10.0f, RColor::Cyan, 8);
		GDebugRenderer.DrawSphere(Iter.Left + Offset, 10.0f, RColor::Red, 8);
		GDebugRenderer.DrawSphere(Iter.Right + Offset, 10.0f, RColor::Green, 8);
		GDebugRenderer.DrawSphere(Iter.Current + Offset, 10.0f, RColor(0.5f, 0.5f, 1.0f), 8);

		GDebugRenderer.DrawLine(Iter.Start + Offset, Iter.Left + Offset, RColor::Red);
		GDebugRenderer.DrawLine(Iter.Start + Offset, Iter.Right + Offset, RColor::Green);
		GDebugRenderer.DrawLine(Iter.Start + Offset, Iter.Current + Offset, RColor(0.5f, 0.5f, 1.0f));

		if (Iter.EdgeId != -1)
		{
			const auto& EdgeData = NavMeshData->GetNavMeshEdgeData(Iter.EdgeId);
			RVec3 p0 = NavMeshData->GetNavMeshPointData(EdgeData.p0).WorldPosition;
			RVec3 p1 = NavMeshData->GetNavMeshPointData(EdgeData.p1).WorldPosition;
			GDebugRenderer.DrawLine(Iter.Start + Offset, p0 + Offset, RColor::Yellow);
			GDebugRenderer.DrawLine(Iter.Start + Offset, p1 + Offset, RColor::Yellow);
			GDebugRenderer.DrawSphere(p0 + Offset, 10.0f, RColor::Yellow, 8);
			GDebugRenderer.DrawSphere(p1 + Offset, 10.0f, RColor::Yellow, 8);
		}
	}
}

void RNavMeshDebugger::DrawFunnelByStep(int Step) const
{
	auto Iter = std::find_if(DebugFunnel.begin(), DebugFunnel.end(), [Step](const DebugFunnelData& Data)
	{
		return Data.EdgeId == Step; 
	});

	if (Iter != DebugFunnel.end())
	{
		DrawFunnel(int(Iter - DebugFunnel.begin()));
	}
}

int RNavMeshDebugger::GetMaxFunnelStepIndex() const
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

int RNavMeshDebugger::GetMaxFunnelSteps() const
{
	return (int)DebugFunnel.size();
}
