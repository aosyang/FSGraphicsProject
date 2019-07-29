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
		: DistanceField(-1)
		, RegionId(-1)
	{
		fill_n(NeighbourLink, NUM_NEIGHBOUR_SPANS, -1);
	}

	int CellRowStart;
	int CellRowEnd;

	// Whether the span is a border span (which does not have 8 valid neighbour spans including diagonal ones)
	bool bBorder;

	// Index to a neighbour open span this spawn is linked to
	int NeighbourLink[NUM_NEIGHBOUR_SPANS];

	int DistanceField;

	int RegionId;
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


template<class T>
inline void HashCombine(size_t& HashValue, const T& Element)
{
	hash<T> Hash;
	HashValue ^= Hash(Element) + 0x9e3779b9 + (HashValue << 6) + (HashValue >> 2);
}

/// Key for a traversable span. Used for searching a span at a given location
class OpenSpanKey
{
public:
	OpenSpanKey(int InX, int InZ, int InSpanIndex)
		: x(InX), z(InZ), span_idx(InSpanIndex)
	{
		HashValue = CalcHash();
	}

	OpenSpanKey(const OpenSpanKey& Rhs)
	{
		x = Rhs.x; z = Rhs.z; span_idx = Rhs.span_idx; HashValue = Rhs.HashValue;
	}

	OpenSpanKey& operator=(const OpenSpanKey& Rhs)
	{
		x = Rhs.x; z = Rhs.z; span_idx = Rhs.span_idx; HashValue = Rhs.HashValue;
		return *this;
	}

	bool operator==(const OpenSpanKey& Rhs) const
	{
		return HashValue == Rhs.HashValue;
	}

	bool operator!=(const OpenSpanKey& Rhs) const
	{
		return HashValue != Rhs.HashValue;
	}

	bool operator<(const OpenSpanKey& Rhs) const
	{
		return HashValue < Rhs.HashValue;
	}

	bool IsValid() const
	{
		return x >= 0;
	}

	int x, z, span_idx;
	const static OpenSpanKey Invalid;

private:
	size_t CalcHash() const
	{
		size_t Hash = 0;
		HashCombine(Hash, x);
		HashCombine(Hash, z);
		HashCombine(Hash, span_idx);
		return Hash;
	}

private:
	size_t HashValue;
};


/// Scene voxelizer
class RVoxelizer
{
public:
	RVoxelizer();

	void Initialize(RScene* Scene);

	void Render();

private:
	void GenerateHeightfieldColumns(const vector<RSceneObject*>& SceneObjects);

	// Generate neighbour data after we have open spans for all columns 
	void GenerateOpenSpanNeighbourData();

	// Mark each area with its distance to a closest border span
	void GenerateDistanceField();

	// Divide all traversable areas into regions
	void GenerateRegions();

	// Make region contours from cells
	void GenerateRegionContours();

	// Going in one direction and find a span at the edge of the region of a given span
	OpenSpanKey FindRegionEdgeInDirection(const OpenSpanKey& Key, int DirectionIdx = 0) const;

	void AddEdge(const OpenSpanKey& Key, int DirectionIdx, int RegionId, bool bMendatoryPoint);

	RAabb CalculateBoundsForSpan(const HeightfieldSolidSpan& Span, int x, int z) const;

	// Returns if a character can navigate between given open spans
	bool IsValidNeighbourSpan(const HeightfieldOpenSpan& ThisOpenSpan, const HeightfieldOpenSpan& NeighbourOpenSpan) const;

	HeightfieldOpenSpan& GetOpenSpanByKey(const OpenSpanKey& Key);
	const HeightfieldOpenSpan& GetOpenSpanByKey(const OpenSpanKey& Key) const;

	OpenSpanKey GetNeighbourSpanByIndex(const OpenSpanKey& Key, int OffsetIndex) const;

	// Returns center location of a cell
	RVec3 GetCellCenter(int x, int y, int z) const;

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
	int MaxDistanceField;

	// Unique region ids
	set<int> UniqueRegionIds;

	static const GridCoord	NeighbourOffset[];
};
