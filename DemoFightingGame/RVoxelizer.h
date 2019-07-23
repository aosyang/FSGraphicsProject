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

struct HeightfieldSpan
{
	HeightfieldSpan()
		: bTraversable(false)
		, CellRowStart(0)
		, CellRowEnd(0)
	{}

	bool bTraversable;
	int CellRowStart;
	int CellRowEnd;
	RAabb Bounds;
};

struct HeightfieldData
{
	// Grid position
	int x, z;

	// A list of solid spans in this column
	vector<HeightfieldSpan> Spans;
};

class RVoxelizer
{
public:
	RVoxelizer();

	void Initialize(RScene* Scene);

	void Render();

private:
	RAabb CreateBoundsForSpan(const HeightfieldSpan& Span, int x, int z);

private:
	RAabb					SceneBounds;
	RVec3					CellDimension;
	float					MinTraversableHeight;

	int CellNumX;
	int CellNumY;
	int CellNumZ;

	vector<HeightfieldData>	Heightfield;
};
