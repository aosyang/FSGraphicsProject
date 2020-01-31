//=============================================================================
// RNavMeshRegionSimplifier.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "RNavMeshGenerator.h"

class RNavMeshRegionSimplifier
{
public:
	void Execute(const std::set<int>& UniqueRegionIds, std::vector<EdgePointCollection>& InOutRegionEdgePoints);

private:
	int FindFirstMandatoryPoint(const EdgePointCollection &RegionEdges) const;

	// Simplify a list of edges with Douglas-Peucker algorithm
	// Return: Index array of edges being removed
	// Note: the last point does go into return value for easier making up a loop
	std::vector<RVec3> SimplifyEdges(const std::vector<RVec3>& Edges);

	// Calculate squared distance from a point to a line segment
	float CalculateSquaredDistanceOfPointToLineSegment(const RVec3& p, const RVec3& a, const RVec3& b);
};
