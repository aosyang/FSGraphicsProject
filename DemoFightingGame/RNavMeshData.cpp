//=============================================================================
// RNavMeshData.cpp by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RNavMeshData.h"

namespace
{
	// Returns the clockwise of three points on the XZ plane
	int CCW2D_XZ(const RVec3& p0, const RVec3& p1, const RVec3& p2)
	{
		float d = RVec3::Cross(p1 - p0, p2 - p0).Y();
		return d > 0 ? 1 : (d < 0 ? -1 : 0);
	}

	// Checks if two line segments on XZ plane intersect
	bool CheckLineSegmentsIntersection2D_XZ(const RVec3& p0, const RVec3& p1, const RVec3& p2, const RVec3& p3)
	{
		// Project all points to XZ plane
		RVec3 a(p0); a.SetY(0);
		RVec3 b(p1); b.SetY(0);
		RVec3 c(p2); c.SetY(0);
		RVec3 d(p3); d.SetY(0);

		int d0 = CCW2D_XZ(a, c, d);
		int d1 = CCW2D_XZ(b, c, d);

		if (d0 == d1 || d0 * d1 == 0)
		{
			return false;
		}

		int d2 = CCW2D_XZ(a, b, c);
		int d3 = CCW2D_XZ(a, b, d);

		if (d2 == d3 || d2 * d3 == 0)
		{
			return false;
		}

		return true;
	}
}

void NavMeshPointData::AddNeighbor(int NeighborId, const RVec3& NeighborPosition)
{
	assert(NumNeighbors < MaxNumNeighbors);

	Neighbors[NumNeighbors] = NeighborId;
	NeighborsDistance[NumNeighbors] = RVec3::Distance(NeighborPosition, WorldPosition);
	NumNeighbors++;
}

void RNavMeshData::AddTriangle(const RVec3& p0, const RVec3& p1, const RVec3& p2, int RegionId)
{
	// Note: Region ids are NOT used now

	int idx0 = FindOrAddPoint(p0);
	int idx1 = FindOrAddPoint(p1);
	int idx2 = FindOrAddPoint(p2);

	MakePointNeighbors(idx0, idx1);
	MakePointNeighbors(idx0, idx2);
	MakePointNeighbors(idx1, idx2);

	int Edge0 = AddOrUpdateEdge(idx0, idx1);
	int Edge1 = AddOrUpdateEdge(idx0, idx2);
	int Edge2 = AddOrUpdateEdge(idx1, idx2);

	MakeEdgeNeighbors(Edge0, Edge1);
	MakeEdgeNeighbors(Edge0, Edge2);
	MakeEdgeNeighbors(Edge1, Edge2);

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
	//OutPath = StringPullPath(OutPath);

	return OutPath.size() > 0;
}

RVec3 RNavMeshData::GetEdgeCenter(int EdgeId) const
{
	const auto& EdgeData = NavMeshEdges[EdgeId];
	int p0 = EdgeData.p0;
	int p1 = EdgeData.p1;

	return (NavMeshPoints[p0].WorldPosition + NavMeshPoints[p1].WorldPosition) * 0.5f;
}

int RNavMeshData::FindEdgeIndexForPointsChecked(int PointId0, int PointId1) const
{
	NavMeshEdgeData SearchEdge(PointId0, PointId1);

	auto Iter = std::find(NavMeshEdges.begin(), NavMeshEdges.end(), SearchEdge);
	assert(Iter != NavMeshEdges.end());

	return int(Iter - NavMeshEdges.begin());
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

void RNavMeshData::MakePointNeighbors(int PointId0, int PointId1)
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

void RNavMeshData::MakeEdgeNeighbors(int EdgeId0, int EdgeId1)
{
	RVec3 EdgeCenter0 = GetEdgeCenter(EdgeId0);
	RVec3 EdgeCenter1 = GetEdgeCenter(EdgeId1);
	float Distance = RVec3::Distance(EdgeCenter0, EdgeCenter1);

	NavMeshEdges[EdgeId0].Neighbors.push_back(NavMeshEdgeNeighborData(EdgeId1, Distance));
	NavMeshEdges[EdgeId1].Neighbors.push_back(NavMeshEdgeNeighborData(EdgeId0, Distance));
}

int RNavMeshData::AddOrUpdateEdge(int PointId0, int PointId1)
{
	NavMeshEdgeData NewEdge(PointId0, PointId1);

	auto Iter = std::find(NavMeshEdges.begin(), NavMeshEdges.end(), NewEdge);
	if (Iter != NavMeshEdges.end())
	{
		// If an edge exists in the edge list, it must be shared with another triangle so let's unset the 'is border' flag.
		Iter->IsBorder = false;

		return int(Iter - NavMeshEdges.begin());
	}
	else
	{
		NavMeshEdges.emplace(NavMeshEdges.end(), NewEdge);
		return (int)NavMeshEdges.size() - 1;
	}
}

std::vector<RVec3> RNavMeshData::StringPullPath(const std::vector<RVec3>& Path)
{
	for (int i = 0; i < (int)Path.size() - 1; i++)
	{
		GDebugRenderer.DrawLine(Path[i], Path[i + 1], RColor::Yellow);
	}

	bool bFirstIntersection = true;

	// Can't string pull a path further if there are only two points in it
	if (Path.size() <= 2)
	{
		return Path;
	}

	std::vector<RVec3> Result;
	Result.push_back(Path[0]);

	int BeginIdx = 0;
	int EndIdx = 2;
	while (EndIdx < (int)Path.size())
	{
		bool bCrossingEdge = false;
		RVec3 PathA(Path[BeginIdx]);
		RVec3 PathB(Path[EndIdx]);

		for (const auto& Edge : NavMeshEdges)
		{
			if (!Edge.IsBorder)
			{
				continue;
			}

			RVec3 EdgeA(NavMeshPoints[Edge.p0].WorldPosition);
			RVec3 EdgeB(NavMeshPoints[Edge.p1].WorldPosition);

			// TODO: This check may involve unwanted edges from other layers
			if (CheckLineSegmentsIntersection2D_XZ(PathA, PathB, EdgeA, EdgeB))
			{
				if (bFirstIntersection)
				{
					GDebugRenderer.DrawLine(PathA, PathB, RColor::Green);
					GDebugRenderer.DrawLine(EdgeA, EdgeB, RColor::Red);

					bFirstIntersection = false;
				}

				bCrossingEdge = true;
				break;
			}
		}

		if (bCrossingEdge)
		{
			Result.push_back(Path[EndIdx - 1]);

			BeginIdx = EndIdx - 1;
			EndIdx = BeginIdx + 2;
		}
		else
		{
			EndIdx++;
		}
	}

	// Add the last point to the result path
	assert(EndIdx - 1 < (int)Path.size());
	Result.push_back(Path[EndIdx - 1]);

	return Result;
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
