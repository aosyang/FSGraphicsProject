//=============================================================================
// RNavigationSystem.cpp by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RNavigationSystem.h"


const RVec3 RNavigationSystem::InvalidPosition(FLT_MAX, FLT_MAX, FLT_MAX);


bool RNavigationSystem::Initialize()
{
	NavMeshDebugger.Initialize(&NavMeshGenerator, &NavMeshData);
	return true;
}

void RNavigationSystem::BuildNavMesh(const RScene* Scene, INavMeshCellDetector& CellDetector /*= RDefaultNavMeshCellDetector()*/)
{
	NavMeshGenerator.Build(Scene, NavMeshData, CellDetector);

	//QueryStart = RVec3(1500, 50, 10);
	//QueryGoal = RVec3(-1500, 50, 10);
	//NavMeshData.QueryPath(QueryStart, QueryGoal, TestPath);
}

bool RNavigationSystem::QueryPath(const RVec3& Start, const RVec3& Goal, std::vector<RVec3>& OutPath)
{
	return NavMeshData.QueryPath(Start, Goal, OutPath);
}

void RNavigationSystem::DebugRender(int DebugFlags) const
{
	NavMeshGenerator.DebugRender(DebugFlags);
	
	if (DebugFlags & NavMeshDebug_DrawPathQuery)
	{
		DebugDrawPathQuery();
	}

	if (DebugFlags & NavMeshDebug_DrawNavMesh)
	{
		DebugDrawNavMesh();
	}
}

void RNavigationSystem::DebugProjectPointToNavmesh(const RVec3& Point) const
{
	NavMeshProjectionResult Result = NavMeshData.ProjectPointToNavmesh(Point);
	if (Result.IsValid())
	{
		const NavMeshTriangleData& Triangle = NavMeshData.GetNavMeshTriangleData(Result.Triangle);
		for (int i = 0; i < 3; i++)
		{
			RVec3 p0 = NavMeshData.GetNavMeshPointData(Triangle.Points[i]).WorldPosition;
			RVec3 p1 = NavMeshData.GetNavMeshPointData(Triangle.Points[(i + 1) % 3]).WorldPosition;
			GDebugRenderer.DrawLine(p0, p1);
		}
	}

	GDebugRenderer.DrawSphere(Point, 50);
}

void RNavigationSystem::DebugSetPathQueryPoints(const RVec3& Start, const RVec3& Goal)
{
	QueryStart = Start;
	QueryGoal = Goal;
	NavMeshData.QueryPath(QueryStart, QueryGoal, TestPath);
}

void RNavigationSystem::DebugDrawPathQuery() const
{
	static const RColor PathColor = RColor::Cyan;
	static const RColor EndPointColor = RColor::Yellow;

	for (int i = 0; i < (int)TestPath.size() - 1; i++)
	{
		GDebugRenderer.DrawLine(TestPath[i], TestPath[i + 1], PathColor);
		GDebugRenderer.DrawSphere(TestPath[i], 5.0f, PathColor);
		if (i == (int)TestPath.size() - 2)
		{
			GDebugRenderer.DrawSphere(TestPath[i + 1], 5.0f, PathColor);
		}
	}

	//GDebugRenderer.DrawSphere(QueryStart, 20.0f, EndPointColor);
	//GDebugRenderer.DrawSphere(QueryGoal, 20.0f, EndPointColor);
}

void RNavigationSystem::DebugDrawNavMesh() const
{
	for (int EdgeIdx = 0; EdgeIdx < NavMeshData.GetNumEdges(); EdgeIdx++)
	{
		auto& EdgeData = NavMeshData.GetNavMeshEdgeData(EdgeIdx);

		GDebugRenderer.DrawLine(
			NavMeshData.GetNavMeshPointData(EdgeData.p0).WorldPosition,
			NavMeshData.GetNavMeshPointData(EdgeData.p1).WorldPosition,
			EdgeData.IsBorder ? RColor(0.0f, 1.0f, 0.0f) : RColor(0.0f, 0.7f, 0.0f)
		);
	}

	for (int TriangleIdx = 0; TriangleIdx < NavMeshData.GetNumTriangles(); TriangleIdx++)
	{
		const auto& TriangleData = NavMeshData.GetNavMeshTriangleData(TriangleIdx);
		RVec3 Points[3];
		for (int PointIdx = 0; PointIdx < 3; PointIdx++)
		{
			const auto& PointData = NavMeshData.GetNavMeshPointData(TriangleData.Points[PointIdx]);
			Points[PointIdx] = PointData.WorldPosition;
		}

		GDebugRenderer.DrawTriangle(Points[0], Points[1], Points[2], RColor(0.0f, 1.0f, 0.0f, 0.6f));
	}
}
