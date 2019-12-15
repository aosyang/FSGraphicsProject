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

void RAStarSearchData::ConditionalAddOpenNode(int ParentId, int EdgeId, float TotalCost, float Heuristics)
{
	AStarSearchNodeData NewSearchNodeData(ParentId, EdgeId, TotalCost, Heuristics);
	int SearchNodeIdx = GetSearchNodeIndexByEdgeId(EdgeId);

	// If a node id exists in the open list and has a lower cost, ignore the new node
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

	// If a node id exists in the closed list and has a lower cost, ignore the new node
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

	if (IsNewEdge(EdgeId))
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

void RAStarSearchData::AddGoalCandidate(int ParentId, int EdgeId, float TotalCost)
{
	AStarSearchNodeData GoalCandidateData(ParentId, EdgeId, TotalCost, 0.0f);
	int SearchNodeIdx = GetSearchNodeIndexByEdgeId(EdgeId);

	if (IsNewEdge(EdgeId))
	{
		int NewIndex = (int)SearchNodes.size();
		SearchNodes.emplace(SearchNodes.end(), GoalCandidateData);

		GoalCandidates.push_back(NewIndex);
	}
	else
	{
		SearchNodes[SearchNodeIdx] = GoalCandidateData;
		GoalCandidates.push_back(SearchNodeIdx);
	}
}

int RAStarSearchData::PopOpenNodeWithMinimalCost()
{
	float MinCost = -1.0f;

	OpenListIterator MinCostIter = OpenList.end();

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

void RAStarSearchData::AddNodeToClosedList(int EdgeId)
{
	VerifyIsExistingPoint(EdgeId);
	ClosedList.push_back(EdgeId);
}

int RAStarSearchData::GetBestGoalCandicate() const
{
	int BestCandicate = -1;
	float MinCost = -1;

	// Find a goal candidate with minimal cost from start point
	for (const auto& CandidateIdx : GoalCandidates)
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

int RAStarSearchData::GetSearchNodeIndexByEdgeId(int EdgeId) const
{
	int Index = 0;

	for (const auto& SearchNode : SearchNodes)
	{
		if (SearchNode.NavmeshEdgeId == EdgeId)
		{
			return Index;
		}

		Index++;
	}

	return -1;
}

int RAStarSearchData::GetEdgeIdBySearchNode(int SearchNodeIdx) const
{
	return SearchNodes[SearchNodeIdx].NavmeshEdgeId;
}

const NavMeshEdgeData& RAStarSearchData::GetEdgeDataRefBySearchNode(int SearchNodeIdx) const
{
	int EdgeIdx = SearchNodes[SearchNodeIdx].NavmeshEdgeId;
	return NavMeshData->GetNavMeshEdgeData(EdgeIdx);
}

void RAStarSearchData::DumpToLog() const
{
	RLog("=== Begin Dump Search Data ===\n")
	RLog("Open list:\n");
	for (int SearchNodeIdx : OpenList)
	{
		int NavMeshEdgeIdx = SearchNodes[SearchNodeIdx].NavmeshEdgeId;
		RLog(" Point %d, parent: %d, cost from start: %.2f, pos: %s\n",
			 SearchNodeIdx,
			 SearchNodes[SearchNodeIdx].ParentId,
			 GetTotalCost(SearchNodeIdx),
			 NavMeshData->GetEdgeCenter(NavMeshEdgeIdx).ToString().c_str());
	}
	RLog("=== End Dump Search Data ===\n")
}

bool RAStarSearchData::IsNewEdge(int EdgeId) const
{
	for (const auto& SearchNode : SearchNodes)
	{
		if (SearchNode.NavmeshEdgeId == EdgeId)
		{
			return false;
		}
	}

	return true;
}

void RAStarSearchData::VerifyIsExistingPoint(int EdgeId) const
{
	assert(EdgeId >= 0 && EdgeId < (int)SearchNodes.size());
}

std::vector<NavPathNode> RAStarPathfinder::Evaluate(const RNavMeshData* NavMeshData, const NavMeshProjectionResult& Start, const NavMeshProjectionResult& Goal)
{
	RAStarSearchData SearchData(NavMeshData);

	const NavMeshTriangleData& StartTriangle = NavMeshData->NavMeshTriangles[Start.Triangle];
	const NavMeshTriangleData& GoalTriangle = NavMeshData->NavMeshTriangles[Goal.Triangle];

	// Initialize the open list with three vertices of the triangle the start point lies inside.
	for (int i = 0; i < 3; i++)
	{
		int EdgeId = NavMeshData->FindEdgeIndexForPointsChecked(StartTriangle.Points[i], StartTriangle.Points[(i + 1) % 3]);
		RVec3 EdgeCenter = NavMeshData->GetEdgeCenter(EdgeId);

		float Distance = RVec3::Distance(EdgeCenter, Start.PositionOnNavmesh);
		float Heuristics = EvaluateHeuristics(EdgeCenter, Goal.PositionOnNavmesh);

		SearchData.ConditionalAddOpenNode(-1, EdgeId, Distance, Heuristics);
	}

	std::vector<int> GoalCandidates;
	int BestCandidate = -1;

	while (SearchData.HasOpenNodes())
	{
		//SearchData.DumpToLog();

		// Find a node with minimal total cost
		int SearchNodeIdx = SearchData.PopOpenNodeWithMinimalCost();
		const NavMeshEdgeData& Edge = SearchData.GetEdgeDataRefBySearchNode(SearchNodeIdx);
		float TotalCost = SearchData.GetTotalCost(SearchNodeIdx);
		bool bReachedGoalTriangle = false;

		for (int n = 0; n < (int)Edge.Neighbors.size(); n++)
		{
			// Index to a neighbor point in the point list of navmesh 
			int NeighborEdgeIdx = Edge.Neighbors[n].NeighborIndex;
			float DistanceToNeighbor = Edge.Neighbors[n].Distance;

			//
			for (int p = 0; p < 3; p++)
			{
				int EdgePoint0 = GoalTriangle.Points[p];
				int EdgePoint1 = GoalTriangle.Points[(p + 1) % 3];
				int EdgeIdx = NavMeshData->FindEdgeIndexForPointsChecked(EdgePoint0, EdgePoint1);

				if (NeighborEdgeIdx == EdgeIdx)
				{
					// TODO: Has reached the goal, stop search.
					SearchData.AddGoalCandidate(SearchNodeIdx, NeighborEdgeIdx, TotalCost + DistanceToNeighbor);
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
				RVec3 EdgeCenter = NavMeshData->GetEdgeCenter(NeighborEdgeIdx);
				float Heuristics = EvaluateHeuristics(EdgeCenter, Goal.PositionOnNavmesh);
				SearchData.ConditionalAddOpenNode(SearchNodeIdx, NeighborEdgeIdx, TotalCost + DistanceToNeighbor, Heuristics);
			}
		}

		if (bReachedGoalTriangle)
		{
			break;
		}

		SearchData.AddNodeToClosedList(SearchNodeIdx);
	}

	//SearchData.DumpToLog();
	std::vector<NavPathNode> PathResult;

	if (BestCandidate != -1)
	{
		PathResult.emplace(PathResult.end(), Goal.PositionOnNavmesh, -1);

		int BackTraceIdx = BestCandidate;
		while (BackTraceIdx != -1)
		{
			int EdgeId = SearchData.GetEdgeIdBySearchNode(BackTraceIdx);

			PathResult.emplace(PathResult.end(), NavMeshData->GetEdgeCenter(EdgeId), EdgeId);
			BackTraceIdx = SearchData.GetParent(BackTraceIdx);
		}

		PathResult.emplace(PathResult.end(), Start.PositionOnNavmesh, -1);

		// We're doing a back-tracking so we need to reverse the result
		std::reverse(PathResult.begin(), PathResult.end());
	}

	return PathResult;
}

float RAStarPathfinder::EvaluateHeuristics(const RVec3& Point, const RVec3& Goal) const
{
	return fabs(Goal.X() - Point.X()) + fabs(Goal.Y() - Point.Y()) + fabs(Goal.Z() - Point.Z());
}
