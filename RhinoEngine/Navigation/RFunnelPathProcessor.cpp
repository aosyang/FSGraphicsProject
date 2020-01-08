//=============================================================================
// RFunnelPathProcessor.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RFunnelPathProcessor.h"

#include "RNavigationSystem.h"

namespace
{
	int DetermineSideOfPointToLine2D(const RVec3& p, const RVec3& l0, const RVec3& l1)
	{
		float d = RVec3::Cross2D(p - l0, l1 - l0);
		return d > 0 ? 1 : (d < 0 ? -1 : 0);
	}
}

RFunnelPathProcessor::RFunnelPathProcessor(const RNavMeshData* InNavMeshData, const std::vector<NavPathNode>& InPathData)
	: NavMeshData(InNavMeshData)
	, PathData(InPathData)
	, Start(RNavigationSystem::InvalidPosition)
	, Left(RNavigationSystem::InvalidPosition)
	, Right(RNavigationSystem::InvalidPosition)
{
	GenerateImmediateLeftAndRightForEdgePoints();
}

std::vector<NavPathNode> RFunnelPathProcessor::Execute()
{
	static bool bDebugOutput = false;
	static bool bDebugDrawFunnel = true;
	static bool bDebugDrawPath = false;

	RNavMeshDebugger& NavMeshDebugger = GNavigationSystem.GetDebugger();

	if (bDebugDrawFunnel)
	{
		NavMeshDebugger.ClearFunnelResult();
	}

	// Debug draw the original path
	if (bDebugDrawPath)
	{
		for (int PathSegmentIdx = 0; PathSegmentIdx < (int)PathData.size() - 1; PathSegmentIdx++)
		{
			if (PathSegmentIdx != 0)
			{
				GDebugRenderer.DrawSphere(PathData[PathSegmentIdx].Position, 5.0f, RColor::Yellow, 8);
			}

			GDebugRenderer.DrawLine(PathData[PathSegmentIdx].Position, PathData[PathSegmentIdx + 1].Position, RColor::Yellow);
			NavMeshData->DebugDrawEdge(PathData[PathSegmentIdx].EdgeId, RColor(0.75f, 0.75f, 0.75f));
		}
	}

	// The path is already simple enough. Stop
	if (PathData.size() <= 2)
	{
		return PathData;
	}

	std::vector<NavPathNode> Result;
	SetStartPoint(PathData[0].Position);
	Result.emplace(Result.end(), PathData[0]);

	static const int LEFT_EDGE_SIDE = 1;
	static const int RIGHT_EDGE_SIDE = -1;

	for (int PathSegmentIdx = 1; PathSegmentIdx < (int)PathData.size();)
	{
		int NewPathIndex = -1;
		const RVec3& PrevPosition = PathData[PathSegmentIdx - 1].Position;
		const RVec3& CurrentPosition = PathData[PathSegmentIdx].Position;

		int EdgeId = PathData[PathSegmentIdx].EdgeId;

		if (EdgeId == -1)
		{
			// Assuming this is the last point in the path
			assert(PathSegmentIdx == (int)PathData.size() - 1);

			// For the last point, we need to decide whether it's crossing the left point or the right point or it can be directly connected 
			// from the previous point.

			if (DetermineSideOfPointToLine2D(Right, Start, CurrentPosition) != RIGHT_EDGE_SIDE)
			{
				Result.emplace(Result.end(), Right, -1);
				NewPathIndex = SetStartPoint(Right);
			}

			if (DetermineSideOfPointToLine2D(Left, Start, CurrentPosition) != LEFT_EDGE_SIDE)
			{
				Result.emplace(Result.end(), Left, -1);
				NewPathIndex = SetStartPoint(Left);
			}

			if (bDebugOutput)
			{
				NavMeshDebugger.AddFunnelStep(PathSegmentIdx, Start, CurrentPosition, Left, Right);
			}

			if (NewPathIndex == -1)
			{
				Result.emplace(Result.end(), PathData[PathSegmentIdx].Position, -1);
				PathSegmentIdx++;
			}
			else
			{
				PathSegmentIdx = NewPathIndex;
			}

			continue;
		}

		const NavMeshEdgeData& Edge = NavMeshData->GetNavMeshEdgeData(EdgeId);

		// Find position for both endpoints of the edge
		RVec3 NewLeft, NewRight;
		int LeftEdgeSide = 0, RightEdgeSide = 0;
		bool FindResult = FindLeftAndRightForPathNode(PathSegmentIdx, NewLeft, NewRight, &LeftEdgeSide, &RightEdgeSide);
		assert(FindResult);

		if (PathSegmentIdx != 1)
		{
			if (Left != NewLeft)
			{
				// If new endpoint is narrower, adapt it as the new point
				if (DetermineSideOfPointToLine2D(Left, Start, NewLeft) != RightEdgeSide)
				{
					// Check if either left or right point has crossed each other
					if (DetermineSideOfPointToLine2D(NewLeft, Start, Right) != LeftEdgeSide)
					{
						Result.emplace(Result.end(), Right, -1);
						NewPathIndex = SetStartPoint(Right, &NewLeft, &NewRight);
					}
					else
					{
						Left = NewLeft;
					}
				}
			}
			else if (Right != NewRight)
			{
				if (DetermineSideOfPointToLine2D(Right, Start, NewRight) != LeftEdgeSide)
				{
					// Check if either left or right point has crossed each other
					if (DetermineSideOfPointToLine2D(NewRight, Start, Left) != RightEdgeSide)
					{
						Result.emplace(Result.end(), Left, -1);
						NewPathIndex = SetStartPoint(Left, &NewLeft, &NewRight);
					}
					else
					{
						Right = NewRight;
					}
				}
			}
		}

		if (bDebugOutput)
		{
			RLog("Step %d - Start: %s, Current: %s, Left: %s, Right: %s\n", PathSegmentIdx,
				Start.ToString().c_str(),
				CurrentPosition.ToString().c_str(),
				Left.ToString().c_str(),
				Right.ToString().c_str());
		}

		if (bDebugDrawFunnel)
		{
			NavMeshDebugger.AddFunnelStep(PathSegmentIdx, Start, CurrentPosition, Left, Right);
		}

		if (NewPathIndex != -1)
		{
			PathSegmentIdx = NewPathIndex;
		}
		else
		{
			PathSegmentIdx++;
		}
	}

	bDebugOutput = false;

	return Result;
}

int RFunnelPathProcessor::SetStartPoint(const RVec3& NewStartPoint, RVec3* FallbackLeft /*= nullptr*/, RVec3* FallbackRight /*= nullptr*/)
{
	Start = NewStartPoint;

	for (const auto& Data : ImmediateNeighborData)
	{
		if (Data.Point == NewStartPoint)
		{
			Left = Data.Left;
			Right = Data.Right;
			return Data.EdgeId;
		}
	}

	if (NewStartPoint == PathData[0].Position)
	{
		FindLeftAndRightForPathNode(1, Left, Right);
	}
	else
	{
		if (FallbackLeft && FallbackRight)
		{
			Left = *FallbackLeft;
			Right = *FallbackRight;
		}
	}

	return -1;
}

void RFunnelPathProcessor::GenerateImmediateLeftAndRightForEdgePoints()
{
	RVec3 Left, Right;
	RVec3 NewLeft, NewRight;

	for (int PathSegmentIdx = 1; PathSegmentIdx < (int)PathData.size() - 1; PathSegmentIdx++)
	{
		FindLeftAndRightForPathNode(PathSegmentIdx, NewLeft, NewRight);

		if (PathSegmentIdx != 1)
		{
			if (Left == NewLeft)
			{
				ImmediateNeighborData.push_back(RImmediateNeighborData(Right, Left, NewRight, PathSegmentIdx));
			}
			else if (Right == NewRight)
			{
				ImmediateNeighborData.push_back(RImmediateNeighborData(Left, NewLeft, Right, PathSegmentIdx));
			}
			else
			{
				assert(false);
			}

		}

		Left = NewLeft;
		Right = NewRight;
	}
}

bool RFunnelPathProcessor::FindLeftAndRightForPathNode(int Index, RVec3& OutLeft, RVec3& OutRight, int* OutLeftSide /*= nullptr*/, int* OutRightSide /*= nullptr*/) const
{
	if (Index > 0 && Index < (int)PathData.size())
	{
		const RVec3& PrevPosition = PathData[Index - 1].Position;
		const RVec3& CurrentPosition = PathData[Index].Position;

		int EdgeId = PathData[Index].EdgeId;
		const NavMeshEdgeData& Edge = NavMeshData->GetNavMeshEdgeData(EdgeId);

		// Find position for both endpoints of the edge
		RVec3 NewLeft = NavMeshData->GetNavMeshPointData(Edge.p1).WorldPosition;
		RVec3 NewRight = NavMeshData->GetNavMeshPointData(Edge.p0).WorldPosition;

		int LeftEdgeSide = DetermineSideOfPointToLine2D(NewLeft, PrevPosition, CurrentPosition);
		int RightEdgeSide = DetermineSideOfPointToLine2D(NewRight, PrevPosition, CurrentPosition);

		// Since an edge does not guarantee the order of two endpoints, swap both endpoints if necessary
		if (RightEdgeSide > LeftEdgeSide)
		{
			std::swap(NewLeft, NewRight);
			std::swap(LeftEdgeSide, RightEdgeSide);
		}

		OutLeft = NewLeft;
		OutRight = NewRight;

		if (OutLeftSide) *OutLeftSide = LeftEdgeSide;
		if (OutRightSide) *OutRightSide = RightEdgeSide;

		return true;
	}

	return false;
}
