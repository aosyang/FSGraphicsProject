//=============================================================================
// RNavMeshData.h by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Core/CoreTypes.h"
#include "RAStarPathfinder.h"

// Data for navmesh points
struct NavMeshPointData
{
	static const int MaxNumNeighbors = 16;

	NavMeshPointData(const RVec3& InWorldPosition)
		: WorldPosition(InWorldPosition)
		, NumNeighbors(0)
	{
		memset(Neighbors, -1, sizeof(Neighbors));

		for (int i = 0; i < MaxNumNeighbors; i++)
		{
			NeighborsDistance[i] = -1;
		}
	}

	void AddNeighbor(int NeighborId, const RVec3& NeighborPosition);

	RVec3 WorldPosition;
	int Neighbors[MaxNumNeighbors];
	float NeighborsDistance[MaxNumNeighbors];
	int NumNeighbors;
};

struct NavMeshTriangleData
{
	NavMeshTriangleData(int InPt0, int InPt1, int InPt2)
		: Points{ InPt0, InPt1, InPt2 }
	{
	}

	int Points[3];
};

struct NavMeshProjectionResult
{
	NavMeshProjectionResult()
		: Triangle(-1)
	{
	}

	int Triangle;
	RVec3 PositionOnNavmesh;
};

class RNavMeshData
{
	friend class RAStarPathfinder;
public:
	// Add a triangle to the collection of navmesh convex
	void AddTriangle(const RVec3& p0, const RVec3& p1, const RVec3& p2, int RegionId);

	bool QueryPath(const RVec3& Start, const RVec3& Goal, std::vector<RVec3>& OutPath);

	NavMeshProjectionResult ProjectPointToNavmesh(const RVec3& Point) const;

	NavMeshPointData& GetNavMeshPointData(int Index);
	const NavMeshPointData& GetNavMeshPointData(int Index) const;

private:
	// Find the index of a point in navmesh point list. If the point does not exist it will be appended to the list.
	int FindOrAddPoint(const RVec3& Point);
	
	// Make two points neighbors of each other
	void MakeNeighbors(int PointId0, int PointId1);

public:
	std::vector<NavMeshPointData> NavMeshPoints;

	std::vector<NavMeshTriangleData> NavMeshTriangles;

	RAStarPathfinder AStarPathfinder;
};

FORCEINLINE NavMeshPointData& RNavMeshData::GetNavMeshPointData(int Index)
{
	return NavMeshPoints[Index];
}

FORCEINLINE const NavMeshPointData& RNavMeshData::GetNavMeshPointData(int Index) const
{
	return NavMeshPoints[Index];
}
