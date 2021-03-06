//=============================================================================
// RVoxelizer.h by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "RHeightfieldData.h"
#include "RVoxelizerDataType.h"
#include "RNavMeshData.h"
#include "RNavMeshCellDetector.h"

#include "UI/RProgressBar.h"

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


/// Navmesh generator
class RNavMeshGenerator
{
	friend class RNavMeshDebugger;
public:
	RNavMeshGenerator();
	~RNavMeshGenerator();

	// Build the navmesh for a scene
	void Build(const RScene* Scene, RNavMeshData& OutNavMeshData, const INavMeshCellDetector& CellDetector);

	// Draw debug geometries for the navmesh generator
	void DebugRender(int DebugFlags) const;

	HeightfieldOpenSpan& GetOpenSpanByKey(const OpenSpanKey& Key);
	const HeightfieldOpenSpan& GetOpenSpanByKey(const OpenSpanKey& Key) const;

	OpenSpanKey GetNeighborSpanByIndex(const OpenSpanKey& Key, int OffsetIndex) const;

	// Coordinates for all offsets of neightbours
	static const GridCoord	NeighborOffset[];

private:
	void GenerateHeightfieldColumns(const INavMeshCellDetector& CellDetector, RProgressBar& ProgressBar);

	// Test if a cell is colliding with any objects in the scene
	bool TestCellOverlappingWithScene(const RAabb& CellBounds, const std::vector<RSceneObject*>& SceneObjects);

	// Test if a cell is on top of solid ground for traversing
	bool TestCellTraversability(const RAabb& CellBounds, const std::vector<RSceneObject*>& SceneObjects, int NumSubdivides);

	// Generate neighbor data after we have open spans for all columns 
	void GenerateOpenSpanNeighborData();

	// Mark each area with its distance to a closest border span
	void GenerateDistanceField();

	// Divide all traversable areas into regions
	void GenerateRegions();

	// Make region contours from cells
	void GenerateRegionContours();

	void TriangulateRegions(RNavMeshData& OutNavMeshData);

	// Going in one direction and find a span at the edge of the region of a given span
	OpenSpanKey FindRegionEdgeInDirection(const OpenSpanKey& Key, int DirectionIdx = 0) const;

	// Add a new point as an edge point, indicating this point is shared by two or more regions.
	void AddEdgePoint(const OpenSpanKey& Key, int DirectionIdx, int RegionId, bool bMandatoryPoint);

	void SetEdgePointMandatory(const OpenSpanKey& Key, int DirectionIdx, int RegionId, bool bMandatoryPoint);

	RVec3 GetOffsetInDirection(int DirectionIdx) const;

	void VerifyIsEdgePointMandatory(int PointRegionId, const RVec3& Point, bool bMandatory) const;

	RAabb CalculateBoundsForSpan(const HeightfieldSolidSpan& Span, int x, int z) const;

	// Returns if a character can navigate between given open spans
	bool IsValidNeighborSpan(const HeightfieldOpenSpan& ThisOpenSpan, const HeightfieldOpenSpan& NeighborOpenSpan) const;

	// Returns center location of a cell
	RVec3 GetCellCenter(int x, int y, int z) const;

private:
	void DebugDrawSpans(int DebugFlags) const;

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
};
