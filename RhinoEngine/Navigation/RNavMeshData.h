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

	// Indices in navmesh point list
	int Points[3];
};

struct NavMeshEdgeNeighborData
{
	NavMeshEdgeNeighborData(int InNeighborIndex, float InDistance)
		: NeighborIndex(InNeighborIndex)
		, Distance(InDistance)
	{
	}

	int NeighborIndex;
	float Distance;
};

struct NavMeshEdgeData
{
	NavMeshEdgeData(int _p0, int _p1)
		: p0(_p0)
		, p1(_p1)
		, IsBorder(true)
	{
	}

	bool operator==(const NavMeshEdgeData& Rhs) const
	{
		return (Rhs.p0 == p0 && Rhs.p1 == p1) || (Rhs.p0 == p1 && Rhs.p1 == p0);
	}

	// Indices of both points
	int p0, p1;

	// Whether the edge lies at the border of the navmesh
	bool IsBorder;

	std::vector<NavMeshEdgeNeighborData> Neighbors;
};

struct NavMeshProjectionResult
{
	NavMeshProjectionResult()
		: Triangle(-1)
	{
	}

	bool IsValid() const
	{
		return Triangle != -1;
	}

	int Triangle;
	RVec3 PositionOnNavmesh;
};

class RNavMeshData
{
	friend class RAStarPathfinder;
	friend class RNavMeshDebugger;

public:
	// Add a triangle to the collection of navmesh convex
	void AddTriangle(const RVec3& p0, const RVec3& p1, const RVec3& p2, int RegionId);

	// Query a path on navmesh between two points
	bool QueryPath(const RVec3& Start, const RVec3& Goal, std::vector<RVec3>& OutPath);

	// Project a point to navmesh
	NavMeshProjectionResult ProjectPointToNavmesh(const RVec3& Point, float MaxHeightDifference = 50.0f) const;

	NavMeshPointData& GetNavMeshPointData(int Index);
	const NavMeshPointData& GetNavMeshPointData(int Index) const;

	NavMeshTriangleData& GetNavMeshTriangleData(int Index);
	const NavMeshTriangleData& GetNavMeshTriangleData(int Index) const;

	NavMeshEdgeData& GetNavMeshEdgeData(int Index);
	const NavMeshEdgeData& GetNavMeshEdgeData(int Index) const;

	// Get total number of edges for navmesh
	int GetNumEdges() const;

	// Get the center position of a given edge
	RVec3 GetEdgeCenter(int EdgeId) const;

	int FindEdgeIndexForPointsChecked(int PointId0, int PointId1) const;

	// Debug draw an edge from navmesh by id
	void DebugDrawEdge(int EdgeId, const RColor& Color) const;

private:
	// Find the index of a point in navmesh point list. If the point does not exist it will be appended to the list.
	int FindOrAddPoint(const RVec3& Point);
	
	// Make two points neighbors of each other
	void MakePointNeighbors(int PointId0, int PointId1);

	// Make two edges neighbors of each other
	void MakeEdgeNeighbors(int EdgeId0, int EdgeId1);

	// Add a new edge or mark an existing edge as non-border
	// Return an index to the edge
	int AddOrUpdateEdge(int PointId0, int PointId1);

	// Optimize a path by the string pulling algorithm
	std::vector<NavPathNode> PerformStringPulling(const std::vector<NavPathNode>& InPathData) const;

	// Optimize a path by the funnel algorithm
	std::vector<NavPathNode> PerformFunnel(const std::vector<NavPathNode>& InPathData) const;

private:
	std::vector<NavMeshPointData> NavMeshPoints;

	// Triangles represented by three indices of navmesh points
	std::vector<NavMeshTriangleData> NavMeshTriangles;

	// Edges represented by two indices of navmesh points
	std::vector<NavMeshEdgeData> NavMeshEdges;

	// The A-star algorithm class
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

FORCEINLINE NavMeshTriangleData& RNavMeshData::GetNavMeshTriangleData(int Index)
{
	return NavMeshTriangles[Index];
}

FORCEINLINE const NavMeshTriangleData& RNavMeshData::GetNavMeshTriangleData(int Index) const
{
	return NavMeshTriangles[Index];
}

FORCEINLINE NavMeshEdgeData& RNavMeshData::GetNavMeshEdgeData(int Index)
{
	return NavMeshEdges[Index];
}

FORCEINLINE const NavMeshEdgeData& RNavMeshData::GetNavMeshEdgeData(int Index) const
{
	return NavMeshEdges[Index];
}

FORCEINLINE int RNavMeshData::GetNumEdges() const
{
	return (int)NavMeshEdges.size();
}
