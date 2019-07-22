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

enum class ESpanType : UINT8
{
	Solid,
	Open,
};

struct HeightfieldSpan
{
	HeightfieldSpan()
		: bTraversable(false)
		, Type(ESpanType::Solid)
		, CellRowStart(0)
		, CellRowEnd(0)
	{}

	bool bTraversable;

	// Whether the span represents solid space or open space
	ESpanType Type;
	int CellRowStart;
	int CellRowEnd;
	RAabb Bounds;
};

struct HeightfieldData
{
	int x, z;
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

	int CellNumX;
	int CellNumY;
	int CellNumZ;

	vector<CellData>		Cells;
	vector<HeightfieldData>	Heightfield;
};
