//=============================================================================
// RAStarPathfinder.h by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Core/CoreTypes.h"

class RNavMeshData;
struct NavMeshProjectionResult;
struct NavMeshPointData;
struct NavMeshEdgeData;

// Data structure for A-star node during searching
struct AStarSearchNodeData
{
	AStarSearchNodeData(int InParentId, int InEdgeId, float InCostFromStart, float InHeuristics)
		: ParentId(InParentId)
		, NavmeshEdgeId(InEdgeId)
		, CostFromStart(InCostFromStart)
		, HeuristicCostToGoal(InHeuristics)
	{
		TotalEstimatedCost = CostFromStart + HeuristicCostToGoal;
	}

	int ParentId;					// Id of parent search node data
	int NavmeshEdgeId;				// Id of edge from original navmesh
	float CostFromStart;			// Cost to this node from the starting position
	float HeuristicCostToGoal;
	float TotalEstimatedCost;
};

// Search data structure used by a-star algorithm
class RAStarSearchData
{
public:
	RAStarSearchData(const RNavMeshData* InNavMeshData);

	// Adds an edge to the open list if suitable
	void ConditionalAddOpenNode(int ParentId, int EdgeId, float TotalCost, float Heuristics);

	// Adds an edge to the closed list
	void AddNodeToClosedList(int EdgeId);

	// Adds node data for a possibly best path to the goal
	void AddGoalCandidate(int ParentId, int EdgeId, float TotalCost);

	// Pops an open node with minimal cost and returns its index in all search nodes
	int PopOpenNodeWithMinimalCost();

	// Checks if open list has any node data
	bool HasOpenNodes() const;

	// Returns a goal candidate with minimal cost
	int GetBestGoalCandicate() const;

	// Gets total cost from the starting point to a search node
	float GetTotalCost(int SearchNodeIdx) const;

	// Gets the parent node of a search node
	int GetParent(int SearchNodeIdx) const;

	// Gets id of a search node by the point index from navmesh
	int GetSearchNodeIndexByEdgeId(int EdgeId) const;

	int GetEdgeIdBySearchNode(int SearchNodeIdx) const;

	// Gets associated navmesh point data of a search node
	const NavMeshEdgeData& GetEdgeDataRefBySearchNode(int SearchNodeIdx) const;

	// Dumps useful search data information to log
	void DumpToLog() const;

private:
	// Checks if a point is new in all search nodes
	bool IsNewEdge(int EdgeId) const;

	// Asserts the existence of a point in search nodes
	void VerifyIsExistingPoint(int EdgeId) const;

private:
	// Pointer to navmesh data, for accessing points and triangles in the navmesh directly 
	const RNavMeshData* NavMeshData;

	std::vector<AStarSearchNodeData> SearchNodes;

	typedef std::list<int> OpenListType;
	OpenListType OpenList;

	std::vector<int> ClosedList;

	std::vector<int> GoalCandicates;
};

// A-star pathfinding algorithm for navmesh
class RAStarPathfinder
{
public:
	std::vector<RVec3> Evaluate(const RNavMeshData* NavMeshData, const NavMeshProjectionResult& Start, const NavMeshProjectionResult& Goal);

private:
	float EvaluateHeuristics(const RVec3& Point, const RVec3& Goal) const;
};
