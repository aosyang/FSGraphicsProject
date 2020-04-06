//=============================================================================
// RFunnelPathProcessor.h by Shiyang Ao, 2020 All Rights Reserved.
//
// A processor which simplifies path-finding results with funnel algorithm
//=============================================================================

#pragma once

#include "Core/CoreTypes.h"
#include "RAStarPathfinder.h"

struct RImmediateNeighborData
{
	RImmediateNeighborData(const RVec3& InPoint, const RVec3& InLeft, const RVec3& InRight, int InEdgeId)
		: Point(InPoint)
		, Left(InLeft)
		, Right(InRight)
		, EdgeId(InEdgeId)
	{}

	RVec3 Point;
	RVec3 Left;
	RVec3 Right;
	int EdgeId;
};

class RFunnelPathProcessor
{
public:
	RFunnelPathProcessor(const RNavMeshData* InNavMeshData, const std::vector<NavPathNode>& InPathData);

	std::vector<NavPathNode> Execute();

private:
	int SetStartPoint(const RVec3& NewStartPoint, RVec3* FallbackLeft = nullptr, RVec3* FallbackRight = nullptr);

	void GenerateImmediateLeftAndRightForEdgePoints();

	bool FindLeftAndRightForPathNode(int Index, RVec3& OutLeft, RVec3& OutRight, int* OutLeftSide = nullptr, int* OutRightSide = nullptr) const;

private:
	const RNavMeshData* NavMeshData;
	const std::vector<NavPathNode>& PathData;

	std::vector<RImmediateNeighborData> ImmediateNeighborData;

	RVec3 Start;
	RVec3 Left, Right;
};
