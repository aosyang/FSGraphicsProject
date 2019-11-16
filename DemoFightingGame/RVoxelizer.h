//=============================================================================
// RVoxelizer.h by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "RHeightfieldData.h"
#include "RVoxelizerDataType.h"
#include "RNavMeshData.h"

#include "RVoxelizerDebugger.h"

#include <vector>
#include <set>

class RScene;
class RSceneObject;

// 2D coordinate on x-z plane
struct GridCoord
{
	int x, z;
};

// Data for region edges
struct EdgePointData
{
	RVec3 Point;
	bool bIsMandatory;
};

typedef std::vector<EdgePointData> EdgePointCollection;


/// Scene voxelizer
class RVoxelizer
{
public:
	RVoxelizer();
	~RVoxelizer();

	void Initialize(RScene* Scene);

	void Render();

	void DebugProjectPointToNavmesh(const RVec3& Point) const;
	void DebugSetGoalPoint(const RVec3& Point);

	HeightfieldOpenSpan& GetOpenSpanByKey(const OpenSpanKey& Key);
	const HeightfieldOpenSpan& GetOpenSpanByKey(const OpenSpanKey& Key) const;

	OpenSpanKey GetNeighbourSpanByIndex(const OpenSpanKey& Key, int OffsetIndex) const;

	// Coordinates for all offsets of neightbours
	static const GridCoord	NeighbourOffset[];

private:
	void GenerateHeightfieldColumns(const std::vector<RSceneObject*>& SceneObjects);

	// Generate neighbour data after we have open spans for all columns 
	void GenerateOpenSpanNeighbourData();

	// Mark each area with its distance to a closest border span
	void GenerateDistanceField();

	// Divide all traversable areas into regions
	void GenerateRegions();

	// Make region contours from cells
	void GenerateRegionContours();

	void TriangulateRegions();

	// Going in one direction and find a span at the edge of the region of a given span
	OpenSpanKey FindRegionEdgeInDirection(const OpenSpanKey& Key, int DirectionIdx = 0) const;

	void AddEdge(const OpenSpanKey& Key, int DirectionIdx, int RegionId, bool bMendatoryPoint);

	RAabb CalculateBoundsForSpan(const HeightfieldSolidSpan& Span, int x, int z) const;

	// Returns if a character can navigate between given open spans
	bool IsValidNeighbourSpan(const HeightfieldOpenSpan& ThisOpenSpan, const HeightfieldOpenSpan& NeighbourOpenSpan) const;

	// Returns center location of a cell
	RVec3 GetCellCenter(int x, int y, int z) const;

private:
	void IncreaseDebugDistanceFieldLevel();
	void DecreaseDebugDistanceFieldLevel();

	int DebugDistanceField;

private:
	RAabb					SceneBounds;
	RVec3					SceneCenterPoint;
	RVec3					CellDimension;
	float					MinTraversableHeight;
	int						MinTraversableNumCells;

	/// The max distance a character can step up
	float					MaxStepHeight;
	int						MaxStepNumCells;

	int CellNumX;
	int CellNumY;
	int CellNumZ;

	RHeightfieldData		Heightfield;
	int MaxDistanceField;

	// Unique region ids
	std::set<int> UniqueRegionIds;

	// Edge points for each region. Array indices represent region ids.
	std::vector<EdgePointCollection> RegionEdgePoints;

	RNavMeshData NavMeshData;

	RVoxelizerDebugger Debugger;

	RVec3 QueryStart;
	RVec3 QueryGoal;
	std::vector<RVec3> TestPath;
};
