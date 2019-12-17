//=============================================================================
// RNavigationSystem.cpp by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RNavigationSystem.h"

bool RNavigationSystem::Initialize()
{
	NavMeshDebugger.Initialize(&NavMeshGenerator, &NavMeshData);
	return true;
}

void RNavigationSystem::BuildNavMesh(const RScene* Scene)
{
	NavMeshGenerator.Build(Scene, NavMeshData);

	QueryStart = RVec3(1500, 50, 10);
	//QueryGoal = RVec3(-1500, 50, 10);
	//NavMeshData.QueryPath(QueryStart, QueryGoal, TestPath);
}

bool RNavigationSystem::QueryPath(const RVec3& Start, const RVec3& Goal, std::vector<RVec3>& OutPath)
{
	return NavMeshData.QueryPath(Start, Goal, OutPath);
}

void RNavigationSystem::DebugRender() const
{
	NavMeshGenerator.DebugRender();
	
	DebugDrawPathQuery();
	DebugDrawNavMesh();
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

void RNavigationSystem::DebugSetGoalPoint(const RVec3& Point)
{
	QueryGoal = Point;
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
	for (int i = 0; i < NavMeshData.GetNumEdges(); i++)
	{
		auto& EdgeData = NavMeshData.GetNavMeshEdgeData(i);

		GDebugRenderer.DrawLine(
			NavMeshData.GetNavMeshPointData(EdgeData.p0).WorldPosition,
			NavMeshData.GetNavMeshPointData(EdgeData.p1).WorldPosition,
			EdgeData.IsBorder ? RColor(0.0f, 1.0f, 0.0f) : RColor(0.0f, 0.4f, 0.0f)
		);
	}
}
