//=============================================================================
// RVoxelizer.h by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Scene/RScene.h"

struct CellData
{
	bool bIsSolid;
	RVec3 Center;
};

// A solid span represents spaces occupied by geometries
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

struct HeightfieldOpenSpan
{
	int CellRowStart;
	int CellRowEnd;

	// Index to a neighbour open span this spawn is linked to
	int NeighbourLink[4] = { -1, -1, -1, -1 };
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
	RAabb CreateBoundsForSpan(const HeightfieldSolidSpan& Span, int x, int z);

	RVec3 GetCellCenter(int x, int y, int z);

private:
	RAabb					SceneBounds;
	RVec3					SceneCenterPoint;
	RVec3					CellDimension;
	float					MinTraversableHeight;

	/// The max distance a character can step up
	float					MaxStepHeight;
	int						MaxStepNumCells;

	int CellNumX;
	int CellNumY;
	int CellNumZ;

	vector<HeightfieldData>	Heightfield;

	static RVec3			NeighbourOffset[4];
};
