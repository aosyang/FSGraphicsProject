//=============================================================================
// RAStarPathfinder.cpp by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RAStarPathfinder.h"
#include "RNavMeshData.h"

RAStarSearchData::RAStarSearchData(const RNavMeshData* InNavMeshData)
	: NavMeshData(InNavMeshData)
{

}

void RAStarSearchData::ConditionalAddOpenNode(int ParentId, int PointId, float TotalCost, float Heuristics)
{
	AStarSearchNodeData NewSearchNodeData(ParentId, PointId, TotalCost, Heuristics);
	int SearchNodeIdx = GetSearchNodeIndexByPointId(PointId);

	// If a point id exists in the open list and has a lower cost, ignore the new node
	{
		auto Iter = std::find(OpenList.begin(), OpenList.end(), SearchNodeIdx);
		if (Iter != OpenList.end())
		{
			if (SearchNodes[*Iter].TotalEstimatedCost < NewSearchNodeData.TotalEstimatedCost)
			{
				return;
			}
		}
	}

	// If a point id exists in the closed list and has a lower cost, ignore the new node
	{
		auto Iter = std::find(ClosedList.begin(), ClosedList.end(), SearchNodeIdx);
		if (Iter != ClosedList.end())
		{
			if (SearchNodes[*Iter].TotalEstimatedCost < NewSearchNodeData.TotalEstimatedCost)
			{
				return;
			}
			else
			{
				ClosedList.erase(Iter);
			}
		}
	}

	if (IsNewPoint(PointId))
	{
		int NewIndex = (int)SearchNodes.size();
		SearchNodes.emplace(SearchNodes.end(), NewSearchNodeData);

		OpenList.push_back(NewIndex);
	}
	else
	{
		SearchNodes[SearchNodeIdx] = NewSearchNodeData;
	}
}

void RAStarSearchData::AddGoalCandidate(int ParentId, int PointId, float TotalCost)
{
	AStarSearchNodeData GoalCandidateData(ParentId, PointId, TotalCost, 0.0f);
	int SearchNodeIdx = GetSearchNodeIndexByPointId(PointId);

	if (IsNewPoint(PointId))
	{
		int NewIndex = (int)SearchNodes.size();
		SearchNodes.emplace(SearchNodes.end(), GoalCandidateData);

		GoalCandicates.push_back(NewIndex);
	}
	else
	{
		SearchNodes[SearchNodeIdx] = GoalCandidateData;
		GoalCandicates.push_back(SearchNodeIdx);
	}
}

int RAStarSearchData::PopOpenNodeWithMinimalCost()
{
	float MinCost = -1.0f;

	OpenListType::iterator MinCostIter = OpenList.end();

	for (auto Iter = OpenList.begin(); Iter != OpenList.end(); Iter++)
	{
		if (Iter == OpenList.begin() || SearchNodes[*Iter].CostFromStart < MinCost)
		{
			MinCost = SearchNodes[*Iter].CostFromStart;
			MinCostIter = Iter;
		}
	}

	if (MinCostIter != OpenList.end())
	{
		int Result = *MinCostIter;
		OpenList.erase(MinCostIter);
		return Result;
	}
	else
	{
		return -1;
	}
}

bool RAStarSearchData::HasOpenNodes() const
{
	return OpenList.size() != 0;
}

void RAStarSearchData::AddNodeToClosedList(int PointId)
{
	VerifyIsExistingPoint(PointId);
	ClosedList.push_back(PointId);
}

int RAStarSearchData::GetBestGoalCandicate() const
{
	int BestCandicate = -1;
	float MinCost = -1;

	// Find a goal candidate with minimal cost from start point
	for (const auto& CandidateIdx : GoalCandicates)
	{
		if (BestCandicate == -1 || SearchNodes[CandidateIdx].CostFromStart < MinCost)
		{
			BestCandicate = CandidateIdx;
			MinCost = SearchNodes[CandidateIdx].CostFromStart;
		}
	}

	return BestCandicate;
}

float RAStarSearchData::GetTotalCost(int SearchNodeIdx) const
{
	return SearchNodes[SearchNodeIdx].CostFromStart;
}

int RAStarSearchData::GetParent(int SearchNodeIdx) const
{
	return SearchNodes[SearchNodeIdx].ParentId;
}

int RAStarSearchData::GetSearchNodeIndexByPointId(int PointId) const
{
	int Index = 0;

	for (const auto& SearchNode : SearchNodes)
	{
		if (SearchNode.NavmeshPointId == PointId)
		{
			return Index;
		}

		Index++;
	}

	return -1;
}

const NavMeshPointData& RAStarSearchData::GetPointDataRefBySearchNode(int SearchNodeIdx) const
{
	int PointIdx = SearchNodes[SearchNodeIdx].NavmeshPointId;
	return NavMeshData->GetNavMeshPointData(PointIdx);
}

void RAStarSearchData::DumpToLog() const
{
	RLog("=== Begin Dump Search Data ===\n")
	RLog("Open list:\n");
	for (int SearchNodeIdx : OpenList)
	{
		int NavMeshPointIdx = SearchNodes[SearchNodeIdx].NavmeshPointId;
		const NavMeshPointData& PointData = NavMeshData->GetNavMeshPointData(NavMeshPointIdx);
		RLog(" Point %d, parent: %d, cost from start: %.2f, pos: %s\n",
			 SearchNodeIdx,
			 SearchNodes[SearchNodeIdx].ParentId,
			 GetTotalCost(SearchNodeIdx),
			 PointData.WorldPosition.ToString().c_str());
	}
	RLog("=== End Dump Search Data ===\n")
}

bool RAStarSearchData::IsNewPoint(int PointId) const
{
	for (const auto& SearchNode : SearchNodes)
	{
		if (SearchNode.NavmeshPointId == PointId)
		{
			return false;
		}
	}

	return true;
}

void RAStarSearchData::VerifyIsExistingPoint(int PointId) const
{
	assert(PointId >= 0 && PointId < (int)SearchNodes.size());
}

std::vector<RVec3> RAStarPathfinder::Evaluate(const RNavMeshData* NavMeshData, const NavMeshProjectionResult& Start, const NavMeshProjectionResult& Goal)
{
	RAStarSearchData SearchData(NavMeshData);

	const NavMeshTriangleData& StartTriangle = NavMeshData->NavMeshTriangles[Start.Triangle];
	const NavMeshTriangleData& GoalTriangle = NavMeshData->NavMeshTriangles[Goal.Triangle];

	// Initialize the open list with three vertices of the triangle the start point lies inside.
	for (int i = 0; i < 3; i++)
	{
		const NavMeshPointData& PointData = NavMeshData->NavMeshPoints[StartTriangle.Points[i]];
		float Distance = (PointData.WorldPosition - Start.PositionOnNavmesh).Magnitude();
		float Heuristics = EvaluateHeuristics(PointData.WorldPosition, Goal.PositionOnNavmesh);

		SearchData.ConditionalAddOpenNode(-1, StartTriangle.Points[i], Distance, Heuristics);
	}

	std::vector<int> GoalCandidates;
	int BestCandidate = -1;

	while (SearchData.HasOpenNodes())
	{
		//SearchData.DumpToLog();

		// Find a node with minimal total cost
		int SearchNodeIdx = SearchData.PopOpenNodeWithMinimalCost();
		const NavMeshPointData& Point = SearchData.GetPointDataRefBySearchNode(SearchNodeIdx);
		float TotalCost = SearchData.GetTotalCost(SearchNodeIdx);
		bool bReachedGoalTriangle = false;

		for (int n = 0; n < Point.NumNeighbors; n++)
		{
			// Index to a neighbor point in the point list of navmesh 
			int NeighborIdx = Point.Neighbors[n];
			float DistanceToNeighbor = Point.NeighborsDistance[n];

			for (int p = 0; p < 3; p++)
			{
				if (NeighborIdx == GoalTriangle.Points[p])
				{
					// TODO: Has reached the goal, stop search.
					SearchData.AddGoalCandidate(SearchNodeIdx, NeighborIdx, TotalCost + DistanceToNeighbor);
					bReachedGoalTriangle = true;
				}
			}

			if (bReachedGoalTriangle)
			{
				BestCandidate = SearchData.GetBestGoalCandicate();
				break;
			}
			else
			{
				const NavMeshPointData& NeighborPoint = NavMeshData->GetNavMeshPointData(NeighborIdx);
				float Heuristics = EvaluateHeuristics(NeighborPoint.WorldPosition, Goal.PositionOnNavmesh);
				SearchData.ConditionalAddOpenNode(SearchNodeIdx, NeighborIdx, TotalCost + DistanceToNeighbor, Heuristics);
			}
		}

		if (bReachedGoalTriangle)
		{
			break;
		}

		SearchData.AddNodeToClosedList(SearchNodeIdx);
	}

	//SearchData.DumpToLog();
	std::vector<RVec3> PathResult;

	if (BestCandidate != -1)
	{
		PathResult.push_back(Goal.PositionOnNavmesh);

		int BackTraceIdx = BestCandidate;
		while (BackTraceIdx != -1)
		{
			auto& PointData = SearchData.GetPointDataRefBySearchNode(BackTraceIdx);
			PathResult.push_back(PointData.WorldPosition);
			BackTraceIdx = SearchData.GetParent(BackTraceIdx);
		}

		PathResult.push_back(Start.PositionOnNavmesh);

		std::reverse(PathResult.begin(), PathResult.end());
	}

	return PathResult;
}

float RAStarPathfinder::EvaluateHeuristics(const RVec3& Point, const RVec3& Goal) const
{
	return fabs(Goal.X() - Point.X()) + fabs(Goal.Y() - Point.Y()) + fabs(Goal.Z() - Point.Z());
}
