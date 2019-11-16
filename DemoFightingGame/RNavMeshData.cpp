//=============================================================================
// RNavMeshData.cpp by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RNavMeshData.h"

void NavMeshPointData::AddNeighbor(int NeighborId, const RVec3& NeighborPosition)
{
	assert(NumNeighbors < MaxNumNeighbors);

	Neighbors[NumNeighbors] = NeighborId;
	NeighborsDistance[NumNeighbors] = (NeighborPosition - WorldPosition).Magnitude();
	NumNeighbors++;
}

void RNavMeshData::AddTriangle(const RVec3& p0, const RVec3& p1, const RVec3& p2, int RegionId)
{
	// TODO: Does not handle region id now
	int idx0 = FindOrAddPoint(p0);
	int idx1 = FindOrAddPoint(p1);
	int idx2 = FindOrAddPoint(p2);

	MakeNeighbors(idx0, idx1);
	MakeNeighbors(idx0, idx2);
	MakeNeighbors(idx1, idx2);

	NavMeshTriangles.emplace(NavMeshTriangles.end(), idx0, idx1, idx2);
}

bool RNavMeshData::QueryPath(const RVec3& Start, const RVec3& Goal, std::vector<RVec3>& OutPath)
{
	NavMeshProjectionResult StartResult = ProjectPointToNavmesh(Start);
	NavMeshProjectionResult GoalResult = ProjectPointToNavmesh(Goal);

	// Both the starting point and the end point need to be on the navmesh
	if (StartResult.Triangle == -1 || GoalResult.Triangle == -1)
	{
		return false;
	}

	// Both the starting point and the goal are in the same triangle. Simply connect them.
	if (StartResult.Triangle == GoalResult.Triangle)
	{
		std::vector<RVec3> Path;
		Path.push_back(StartResult.PositionOnNavmesh);
		Path.push_back(GoalResult.PositionOnNavmesh);
		OutPath = Path;

		return true;
	}

	OutPath = AStarPathfinder.Evaluate(this, StartResult, GoalResult);

	return OutPath.size() > 0;
}

int RNavMeshData::FindOrAddPoint(const RVec3& Point)
{
	int Index = 0;
	for (const auto& SearchPoint : NavMeshPoints)
	{
		if (SearchPoint.WorldPosition == Point)
		{
			return Index;
		}

		Index++;
	}

	NavMeshPoints.emplace(NavMeshPoints.end(), Point);
	return Index;
}

void RNavMeshData::MakeNeighbors(int PointId0, int PointId1)
{
	int PointIds[] = { PointId0, PointId1 };

	for (int i = 0; i < 2; i++)
	{
		int ThisPointId = PointIds[i];
		int OtherPointId = PointIds[1 - i];

		// Validate input indices
		assert(ThisPointId >= 0 && ThisPointId < NavMeshPoints.size());

		bool bHasExistConnection = false;
		NavMeshPointData& ThisPointData = NavMeshPoints[ThisPointId];
		for (int n = 0; n < ThisPointData.NumNeighbors; n++)
		{
			if (ThisPointData.Neighbors[n] == OtherPointId)
			{
				bHasExistConnection = true;
				break;
			}
		}

		if (!bHasExistConnection)
		{
			ThisPointData.AddNeighbor(OtherPointId, NavMeshPoints[OtherPointId].WorldPosition);
		}
	}
}

NavMeshProjectionResult RNavMeshData::ProjectPointToNavmesh(const RVec3& Point) const
{
	NavMeshProjectionResult Result;
	for (int Index = 0; Index < (int)NavMeshTriangles.size(); Index++)
	{
		const auto& Triangle = NavMeshTriangles[Index];

		int idx0 = Triangle.Points[0];
		int idx1 = Triangle.Points[1];
		int idx2 = Triangle.Points[2];

		float u, v, w;

		const RVec3 p0 = NavMeshPoints[idx0].WorldPosition;
		const RVec3 p1 = NavMeshPoints[idx1].WorldPosition;
		const RVec3 p2 = NavMeshPoints[idx2].WorldPosition;

		// Project the point to navmesh alone y-axis by evaluating its 2D barycentric parameters
		RMath::Barycentric2D_XZ(Point,
			p0, p1, p2,
			u, v, w);

		if (u >= 0 && v >= 0 && w >= 0)
		{
			Result.Triangle = Index;

			// Project the point to navmesh by modifying y from the input
			RVec3 ProjectedPoint = Point;
			ProjectedPoint.SetY((p0 * u + p1 * v + p2 * w).Y());
			Result.PositionOnNavmesh = ProjectedPoint;

			// TODO: Handle multi-layered navmesh in the future

			break;
		}
	}

	return Result;
}
