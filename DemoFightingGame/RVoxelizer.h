//=============================================================================
// RVoxelizer.h by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Scene/RScene.h"

#define NUM_NEIGHBOUR_SPANS 4

// 2D coordinate on x-z plane
struct GridCoord
{
	int x, z;
};

// A span represents solid spaces occupied by any geometries
struct HeightfieldSolidSpan
{
	HeightfieldSolidSpan()
		: bTraversable(false)
		, CellRowStart(0)
		, CellRowEnd(0)
	{}

	bool bTraversable;
	int CellRowStart;
	int CellRowEnd;
	RAabb Bounds;
};

// A span represents a traversable area
struct HeightfieldOpenSpan
{
	HeightfieldOpenSpan()
		: DistanceField(-1.0f)
	{
		fill_n(NeighbourLink, NUM_NEIGHBOUR_SPANS, -1);
	}

	int CellRowStart;
	int CellRowEnd;

	// Whether the span is a border span (which does not have 8 valid neighbour spans including diagonal ones)
	bool bBorder;

	// Index to a neighbour open span this spawn is linked to
	int NeighbourLink[NUM_NEIGHBOUR_SPANS];

	float DistanceField;
};

struct HeightfieldData
{
	// Grid position
	int x, z;

	// A list of solid spans in this column
	vector<HeightfieldSolidSpan> SolidSpans;

	// A list of open spans in this column
	vector<HeightfieldOpenSpan> OpenSpans;
};

class RVoxelizer
{
public:
	RVoxelizer();

	void Initialize(RScene* Scene);

	void Render();

private:
	void GenerateHeightfieldColumns(const vector<RSceneObject*>& SceneObjects);

	void GenerateOpenSpanNeighbourData();

	void GenerateDistanceField();

	RAabb CreateBoundsForSpan(const HeightfieldSolidSpan& Span, int x, int z);

	// Returns if a character can navigate between given open spans
	bool IsValidNeighbourSpan(const HeightfieldOpenSpan& ThisOpenSpan, const HeightfieldOpenSpan& NeighbourOpenSpan) const;

	// Returns center location of a cell
	RVec3 GetCellCenter(int x, int y, int z);

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

	vector<HeightfieldData>	Heightfield;
	float MaxDistanceField;

	static const GridCoord	NeighbourOffset[];
};
