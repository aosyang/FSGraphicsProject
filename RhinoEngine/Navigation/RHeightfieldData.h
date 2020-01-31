//=============================================================================
// RHeightfieldData.h by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Core/RAabb.h"

#include <algorithm>
#include <vector>

#define NUM_NEIGHBOR_SPANS 4

// A span represents solid spaces occupied by any geometries
struct HeightfieldSolidSpan
{
	HeightfieldSolidSpan()
		: bTraversable(true)
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
		: DistanceField(-1)
		, RegionId(-1)
	{
		std::fill_n(NeighborLink, NUM_NEIGHBOR_SPANS, -1);
	}

	int CellRowStart;
	int CellRowEnd;

	// Whether the span is a border span (which has less than 8 valid neighbor spans including diagonal ones)
	bool bBorder;

	// Index to a neighbor open span which current span is linked to
	int NeighborLink[NUM_NEIGHBOR_SPANS];

	int DistanceField;

	int RegionId;
};

struct HeightfieldCellData
{
	// The position of the cell in grids
	int x, z;

	// A list of solid spans in this column
	std::vector<HeightfieldSolidSpan> SolidSpans;

	// A list of open spans in this column
	std::vector<HeightfieldOpenSpan> OpenSpans;
};

// Heightfield data used by navmesh generators
class RHeightfieldData
{
public:
	RHeightfieldData();

	HeightfieldCellData& operator[](int Index);
	const HeightfieldCellData& operator[](int Index) const;

	// For range-based for loop
	auto begin()		{ return CellData.begin();	}
	auto end()			{ return CellData.end();	}
	auto begin() const	{ return CellData.begin();	}
	auto end() const	{ return CellData.end();	}

	void Resize(size_t NewSizeX, size_t NewSizeZ);

private:
	std::vector<HeightfieldCellData> CellData;

	size_t NumCellsX, NumCellsZ;
};

FORCEINLINE RHeightfieldData::RHeightfieldData()
	: NumCellsX(0)
	, NumCellsZ(0)
{
}

FORCEINLINE HeightfieldCellData& RHeightfieldData::operator[](int Index)
{
	return CellData[Index];
}

FORCEINLINE const HeightfieldCellData& RHeightfieldData::operator[](int Index) const
{
	return CellData[Index];
}

FORCEINLINE void RHeightfieldData::Resize(size_t NewSizeX, size_t NewSizeZ)
{
	NumCellsX = NewSizeX;
	NumCellsZ = NewSizeZ;
	CellData.resize(NewSizeX * NewSizeZ);
}
