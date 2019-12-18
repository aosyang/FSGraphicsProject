//=============================================================================
// RVoxelizer.cpp by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RNavMeshGenerator.h"
#include "Core/RAabb.h"
#include "RenderSystem/RDebugRenderer.h"

#include "Scene/RScene.h"
#include "RRegionData.h"
#include "RNavigationSystem.h"


namespace
{
	// For debug rendering regions in different colors
	std::vector<RColor> DebugRegionColors;

	// For debug rendering the region map at various heightfield
	std::vector<RegionMapType> DebugRegionMaps;

	// Calculate squared distance from a point to a line segment
	float CalculateSquaredDistanceOfPointToLineSegment(const RVec3& p, const RVec3& a, const RVec3& b)
	{
		assert(a - b != RVec3::Zero());

		RVec3 ap = p - a;
		RVec3 ab = b - a;

		float f = RVec3::Dot(ap, ab) / RVec3::Dot(ab, ab);
		if (f <= 0.0f)
		{
			return ap.SquaredMagitude();
		}
		else if (f >= 1.0f)
		{
			return (p - b).SquaredMagitude();
		}
		else
		{
			RVec3 q = a + ab * f;
			return (p - q).SquaredMagitude();
		}
	}

	// Simplify a list of edges with Douglas-Peucker algorithm
	// Note: the last point does go into return value for easier making up a loop
	std::vector<RVec3> SimplifyEdges(const std::vector<RVec3>& Edges)
	{
		assert(Edges.size() >= 2);

		if (Edges.size() == 2)
		{
			return std::vector<RVec3>{ Edges[0] };
		}

		const RVec3& Start = Edges[0];
		const RVec3& End = Edges[Edges.size() - 1];
		float MaxSqrDist = 0.0f;
		int MaxPointIdx = -1;

		for (int i = 1; i < (int)Edges.size() - 1; i++)
		{
			float SqrDist = CalculateSquaredDistanceOfPointToLineSegment(Edges[i], Start, End);
			if (SqrDist > MaxSqrDist)
			{
				MaxSqrDist = SqrDist;
				MaxPointIdx = i;
			}
		}

		static const float Threshold = 80.0f;
		if (MaxSqrDist <= Threshold * Threshold)
		{
			// All distances from the line segment to points are smaller than the threshold. The edge list can be simplified as one edge.
			return std::vector<RVec3>{ Edges[0] };
		}
		else
		{
			// Taking the point with a max distance to the line segment as a middle point,
			// split the edges into two sublists and perform the algorithm on each of them.
			auto Left = SimplifyEdges(std::vector<RVec3>(Edges.begin(), Edges.begin() + MaxPointIdx + 1));
			auto Right = SimplifyEdges(std::vector<RVec3>(Edges.begin() + MaxPointIdx, Edges.end()));

			std::vector<RVec3> Result;
			Result.reserve(Left.size() + Right.size());
			Result.insert(Result.end(), Left.begin(), Left.end());
			Result.insert(Result.end(), Right.begin(), Right.end());

			return Result;
		}
	}

	int FindShortestPartition(const EdgePointCollection &Edges)
	{
		int ShortestPartition = -1;
		float SqrMinDist = 0.0f;
		int NumEdgePoints = (int)Edges.size();

		for (int EdgePt0 = 0; EdgePt0 < NumEdgePoints; EdgePt0++)
		{
			int EdgePt1 = (EdgePt0 + 1) % NumEdgePoints;
			int EdgePt2 = (EdgePt0 + 2) % NumEdgePoints;

			RVec3 Edge0 = Edges[EdgePt1].Point - Edges[EdgePt0].Point;
			RVec3 Edge1 = Edges[EdgePt2].Point - Edges[EdgePt1].Point;

			// Ignore vertical difference for partitioning
			Edge0.SetY(0);
			Edge1.SetY(0);

			RVec3 cp_result = RVec3::Cross(Edge1, Edge0);
			//RLog("Edge 0: %s, Edge 1: %s, ty: %f\n", Edge0.ToString().c_str(), Edge1.ToString().c_str(), cp_result.Y());

			// Skip edges lying outside of the polygon
			if (cp_result.Y() >= 0.0f)
			{
				continue;
			}

			float NewSqrDist = (Edges[EdgePt0].Point - Edges[EdgePt2].Point).SquaredMagitude();
			if (ShortestPartition == -1 || NewSqrDist < SqrMinDist)
			{
				ShortestPartition = EdgePt0;
				SqrMinDist = NewSqrDist;
			}
		}

		return ShortestPartition;
	}

	void IncreasePeriodicIndex(int& Index, int MaxIndex)
	{
		Index++;
		if (Index > MaxIndex)
		{
			Index = 0;
		}
	}

	void DecreasePeriodicIndex(int& Index, int MaxIndex)
	{
		Index--;
		if (Index < 0)
		{
			Index = MaxIndex;
		}
	}
}

const OpenSpanKey OpenSpanKey::Invalid(-1, -1, -1);

const GridCoord RNavMeshGenerator::NeighborOffset[8] =
{
	// Direct neighbor grids
	{-1,  0 },
	{ 0,  1 },
	{ 1,  0 },
	{ 0, -1 },

	// Diagonal grids
	{-1,  1 },
	{ 1,  1 },
	{ 1, -1 },
	{-1, -1 },
};


RNavMeshGenerator::RNavMeshGenerator()
	: DebugDistanceField(0)
	, CellDimension(50.0f, 10.0f, 50.0f)
	, MinTraversableHeight(200.0f)
	, MaxStepHeight(10.0f)
{
	MinTraversableNumCells = (int)ceil(MinTraversableHeight / CellDimension.Y());
	MaxStepNumCells = (int)floor(MaxStepHeight / CellDimension.Y());
}

RNavMeshGenerator::~RNavMeshGenerator()
{
	// Unbind all debug key functions
	RInput.UnbindKeyStateEvents(VK_OEM_4, EBufferedKeyState::Pressed);
	RInput.UnbindKeyStateEvents(VK_OEM_6, EBufferedKeyState::Pressed);
}

void RNavMeshGenerator::Build(const RScene* Scene, RNavMeshData& OutNavMeshData)
{
	RLog("Initalizing navmesh generator from static level meshes.\n");

	// Measure scene size
	SceneBounds = RAabb::Default;
	auto SceneObjects = Scene->EnumerateSceneObjects();
	for (auto& SceneObj : SceneObjects)
	{
		SceneBounds.Expand(SceneObj->GetAabb());
	}

	// Calculate numbers of cells in each dimension
	RVec3 SceneBoundSize = SceneBounds.GetLocalDimension();
	CellNumX = (int)ceilf(SceneBoundSize.X() / CellDimension.X());
	CellNumY = (int)ceilf(SceneBoundSize.Y() / CellDimension.Y());
	CellNumZ = (int)ceilf(SceneBoundSize.Z() / CellDimension.Z());

	RLog("Total cell dimensions - X: %d, Y: %d, Z: %d\n", CellNumX, CellNumY, CellNumZ);

	SceneCenterPoint = SceneBounds.GetCenter();

	// Create heightfield
	{
		// Allocate heightfield cells by x-z grid
		Heightfield.Resize(CellNumX, CellNumZ);

		GenerateHeightfieldColumns(SceneObjects);
		GenerateOpenSpanNeighborData();
		GenerateDistanceField();
		GenerateRegions();
		GenerateRegionContours();
		TriangulateRegions(OutNavMeshData);
	}

	// Generate randomized color for each region
	{
		DebugRegionColors.resize(50);
		for (auto& Color : DebugRegionColors)
		{
			Color = RColor(
				RMath::RandRangedF(0.5f, 1.0f),
				RMath::RandRangedF(0.5f, 1.0f),
				RMath::RandRangedF(0.5f, 1.0f)
			);
		}
	}

	// Debug key '['
	RInput.BindKeyStateEvent(VK_OEM_4, EBufferedKeyState::Pressed, this, &RNavMeshGenerator::IncreaseDebugDistanceFieldLevel);

	// Debug key ']'
	RInput.BindKeyStateEvent(VK_OEM_6, EBufferedKeyState::Pressed, this, &RNavMeshGenerator::DecreaseDebugDistanceFieldLevel);
}

void RNavMeshGenerator::DebugRender() const
{
	GDebugRenderer.DrawAabb(SceneBounds);

#if 0
	// Debug draw regions
	{
		static int DebugDrawRegionId = 0;

		if (RInput.GetBufferedKeyState(VK_OEM_PLUS) == EBufferedKeyState::Pressed)
		{
			IncreasePeriodicIndex(DebugDrawRegionId, (int)RegionEdgePoints.size() - 1);
		}

		if (RInput.GetBufferedKeyState(VK_OEM_MINUS) == EBufferedKeyState::Pressed)
		{
			DecreasePeriodicIndex(DebugDrawRegionId, (int)RegionEdgePoints.size() - 1);
		}

		GNavigationSystem.GetDebugger().DrawRegion(DebugDrawRegionId);
	}
#else
	// Debug draw funnel algorithm
	{
		static int DebugDrawFunnelId = 0;
		int MaxFunnelIndex = GNavigationSystem.GetDebugger().GetMaxStepIndex();

		if (RInput.GetBufferedKeyState(VK_OEM_PLUS) == EBufferedKeyState::Pressed)
		{
			IncreasePeriodicIndex(DebugDrawFunnelId, MaxFunnelIndex);
		}

		if (RInput.GetBufferedKeyState(VK_OEM_MINUS) == EBufferedKeyState::Pressed)
		{
			DecreasePeriodicIndex(DebugDrawFunnelId, MaxFunnelIndex);
		}

		GNavigationSystem.GetDebugger().DrawFunnel(DebugDrawFunnelId);
	}
#endif

	//DebugDrawSpans();
}

void RNavMeshGenerator::GenerateHeightfieldColumns(const std::vector<RSceneObject*>& SceneObjects)
{
	for (int x = 0; x < CellNumX; x++)
	{
		for (int z = 0; z < CellNumZ; z++)
		{
			int Index = x * CellNumZ + z;
			auto& Column = Heightfield[Index];
			Column.x = x;
			Column.z = z;

			HeightfieldSolidSpan Span;
			bool bIsLastCellSolid = false;

			// Search for all spans in a column, bottom-up
			for (int y = 0; y < CellNumY; y++)
			{
				// Find center of a cell
				RVec3 CellCenter = GetCellCenter(x, y, z);

				RAabb CellBounds;
				RVec3 CollisionDimention = CellDimension * RVec3(1.0f, 0.5f, 1.0f);
				CellBounds.pMax = CellCenter + CollisionDimention;
				CellBounds.pMin = CellCenter - CollisionDimention;

				bool bIsSolidCell = false;

				// Has any overlaps with scene meshes?
				for (auto& SceneObj : SceneObjects)
				{
					const RAabb& ObjBounds = SceneObj->GetAabb();
					if (ObjBounds.TestIntersectionWithAabb(CellBounds))
					{
						// Mesh objects may contain per-element bounding box. In that case we're running overlapping test against mesh elements.
						if (RSMeshObject* MeshObj = SceneObj->CastTo<RSMeshObject>())
						{
							for (int i = 0; i < MeshObj->GetMeshElementCount() && !bIsSolidCell; i++)
							{
								const RAabb& ElementBounds = MeshObj->GetMeshElementAabb(i);
								const RAabb ElementWorldBounds = ElementBounds.GetTransformedAabb(MeshObj->GetTransformMatrix());

								if (ElementWorldBounds.TestIntersectionWithAabb(CellBounds))
								{
									bIsSolidCell = true;
								}
							}
						}
						else
						{
							bIsSolidCell = true;
						}

						if (bIsSolidCell)
						{
							break;
						}
					}
				}

				if (bIsSolidCell)
				{
					// A new solid spawn. Finish last open span
					if (!bIsLastCellSolid)
					{
						// Start a new solid span
						Span.CellRowStart = y;
					}
				}
				else // !bIsSolidCell
				{
					if (bIsLastCellSolid)
					{
						Span.CellRowEnd = y - 1;
						Column.SolidSpans.push_back(Span);
					}
				}

				bIsLastCellSolid = bIsSolidCell;
			}

			// Finish the last span
			if (bIsLastCellSolid)
			{
				Span.CellRowEnd = CellNumY - 1;
				Column.SolidSpans.push_back(Span);
			}

			// Evaluate traversable flag for each span
			for (size_t i = 0; i < Column.SolidSpans.size(); i++)
			{
				if (i < Column.SolidSpans.size() - 1)
				{
					// Measure empty spaces between spans for traversable
					HeightfieldSolidSpan& ThisSpan = Column.SolidSpans[i];
					HeightfieldSolidSpan& NextSpan = Column.SolidSpans[i + 1];
					int NumEmptyCells = NextSpan.CellRowStart - ThisSpan.CellRowEnd - 1;
					ThisSpan.bTraversable = (CellDimension.Y() * NumEmptyCells >= MinTraversableHeight);

					if (ThisSpan.bTraversable)
					{
						HeightfieldOpenSpan OpenSpan;
						OpenSpan.CellRowStart = ThisSpan.CellRowEnd + 1;
						OpenSpan.CellRowEnd = NextSpan.CellRowStart;
						Column.OpenSpans.push_back(OpenSpan);
					}
				}
				else
				{
					// Check if top spans are traversable by their distance to the up boundary
					HeightfieldSolidSpan& ThisSpan = Column.SolidSpans[i];
					int NumEmptyCells = CellNumY - ThisSpan.CellRowEnd - 1;
					ThisSpan.bTraversable = true; //(CellDimension.Y() * NumEmptyCells >= MinTraversableHeight);

					if (ThisSpan.bTraversable)
					{
						HeightfieldOpenSpan OpenSpan;
						OpenSpan.CellRowStart = ThisSpan.CellRowEnd + 1;
						OpenSpan.CellRowEnd = INT_MAX;

						if (OpenSpan.CellRowStart < CellNumY)
						{
							Column.OpenSpans.push_back(OpenSpan);
						}
					}
				}
			}

			// Create bounds for each span
			for (auto& IterSpan : Column.SolidSpans)
			{
				IterSpan.Bounds = CalculateBoundsForSpan(IterSpan, x, z);
			}
		}
	}
}

void RNavMeshGenerator::GenerateOpenSpanNeighborData()
{
	for (int x = 0; x < CellNumX; x++)
	{
		for (int z = 0; z < CellNumZ; z++)
		{
			int Index = x * CellNumZ + z;
			auto& Column = Heightfield[Index];

			for (auto& ThisOpenSpan : Column.OpenSpans)
			{
				int NumValidNeighbors = 0;
				for (int NeighborLinkIndex = 0; NeighborLinkIndex < 8; NeighborLinkIndex++)
				{
					// Neighbor coordinates
					const int n_x = x + NeighborOffset[NeighborLinkIndex].x;
					const int n_z = z + NeighborOffset[NeighborLinkIndex].z;

					if (n_x >= 0 && n_x < CellNumX &&
						n_z >= 0 && n_z < CellNumZ)
					{
						int NeighborIndex = n_x * CellNumZ + n_z;

						if (NeighborLinkIndex < NUM_NEIGHBOR_SPANS)
						{
							// Check direct neighbors and add valid ones to the neighbor link list
							int NeighborOpenSpanIndex = 0;

							// Find linked neighbor open spans
							for (const auto& NeighborOpenSpan : Heightfield[NeighborIndex].OpenSpans)
							{
								if (IsValidNeighborSpan(ThisOpenSpan, NeighborOpenSpan))
								{
									ThisOpenSpan.NeighborLink[NeighborLinkIndex] = NeighborOpenSpanIndex;
									NumValidNeighbors++;
									break;
								}

								NeighborOpenSpanIndex++;
							}
						}
						else
						{
							// Diagonal neighbors are only used to check for border spans
							for (const auto& NeighborOpenSpan : Heightfield[NeighborIndex].OpenSpans)
							{
								if (IsValidNeighborSpan(ThisOpenSpan, NeighborOpenSpan))
								{
									NumValidNeighbors++;
									break;
								}
							}
						}
					}
				}

				// Span is a border if at least one of eight neighbors is not walkable from it
				ThisOpenSpan.bBorder = (NumValidNeighbors < 8);
			}
		}
	}
}

void RNavMeshGenerator::GenerateDistanceField()
{
	// TODO: preallocate data with total open span count
	std::queue<OpenSpanKey> PendingSpans;

	// The result of each span and its distance field
	std::map<OpenSpanKey, int> SpanDistanceMap;

	// Collect all border open spans from heightfield
	for (int x = 0; x < CellNumX; x++)
	{
		for (int z = 0; z < CellNumZ; z++)
		{
			int Index = x * CellNumZ + z;
			auto& Column = Heightfield[Index];

			// Index of the open span in its column
			int IndexOpenSpan = 0;
			for (auto& ThisOpenSpan : Column.OpenSpans)
			{
				if (ThisOpenSpan.bBorder)
				{
					OpenSpanKey NewKey(x, z, IndexOpenSpan);
					PendingSpans.emplace(NewKey);
					SpanDistanceMap[NewKey] = 0;
				}
				IndexOpenSpan++;
			}
		}
	}

	int NumSpansProcessed = 0;

	// Loop through all neighbor spans and add them to the end of the container
	while (PendingSpans.size() != 0)
	{
		// Get one element from the queue in the front
		const OpenSpanKey& ThisSpanKey = PendingSpans.front();
		//RLog("Span: %d, %d\n", ThisSpanData.x, ThisSpanData.z);

		int ColumnIndex = ThisSpanKey.x * CellNumZ + ThisSpanKey.z;
		auto& Column = Heightfield[ColumnIndex];

		for (int i = 0; i < 4; i++)
		{
			int NeighborSpanIndex = Column.OpenSpans[ThisSpanKey.span_idx].NeighborLink[i];
			if (NeighborSpanIndex != -1)
			{
				// New span data with the distance field
				OpenSpanKey NewKey(
					ThisSpanKey.x + NeighborOffset[i].x,
					ThisSpanKey.z + NeighborOffset[i].z,
					NeighborSpanIndex);

				// Neighbor distance = current distance + 1
				int NewDistance = SpanDistanceMap.find(ThisSpanKey)->second + 1;

				auto Iter = SpanDistanceMap.find(NewKey);

				// If this neighbor is a new span, added it to the queue
				if (Iter == SpanDistanceMap.end())
				{
					PendingSpans.emplace(NewKey);
					SpanDistanceMap[NewKey] = NewDistance;
				}
				else
				{
					int OldDistance = SpanDistanceMap[NewKey];
					if (NewDistance < OldDistance)
					{
						SpanDistanceMap[NewKey] = NewDistance;
					}
				}
			}
		}

		PendingSpans.pop();

		// Output progress
		//{
		//	NumSpansProcessed++;
		//	if (NumSpansProcessed % 10 == 0)
		//	{
		//		RLog("Generating distance field. Remaining: %zu, FinishSpans: %zu\n", PendingSpans.size(), SpanDistanceMap.size());
		//	}
		//}
	}

	MaxDistanceField = 0;

	// Copy distance values to open span array
	for (const auto& Span : SpanDistanceMap)
	{
		//RLog("Span: %d, %d, %.2f\n", Span.first.x, Span.first.z, Span.second);

		int ColumnIndex = Span.first.x * CellNumZ + Span.first.z;
		auto& Column = Heightfield[ColumnIndex];

		int SpanIndex = Span.first.span_idx;
		int DistanceField = Span.second;
		Column.OpenSpans[SpanIndex].DistanceField = DistanceField;

		if (DistanceField > MaxDistanceField)
		{
			MaxDistanceField = DistanceField;
		}
	}

	DebugDistanceField = MaxDistanceField;
}

void RNavMeshGenerator::GenerateRegions()
{
	int NumRegions = 0;
	RRegionData RegionData(this);

	DebugRegionMaps.resize(MaxDistanceField + 1);

	for (int DistanceFieldIdx = MaxDistanceField; DistanceFieldIdx >= 0; DistanceFieldIdx--)
	{
		// New spans that do not directly connect to any existing regions
		std::vector<OpenSpanKey> IsolatedSpans;

		for (int x = 0; x < CellNumX; x++)
		{
			for (int z = 0; z < CellNumZ; z++)
			{
				int Index = x * CellNumZ + z;
				auto& Column = Heightfield[Index];

				int SpanIndex = 0;

				// Index of the open span in its column
				for (auto& ThisOpenSpan : Column.OpenSpans)
				{
					if (ThisOpenSpan.DistanceField == DistanceFieldIdx)
					{
						// 1. At least one neighbor is from a previous distance field: Set region id to neighbor's region id
						// 2. Neighbor has no direct connection to previous spans. Possibly:
						//    - Span is connecting to one or more existing regions through other spans
						//    - Span is forming a new region (if no connections are made to existing regions)

						OpenSpanKey ThisKey(x, z, SpanIndex);
						int ThisRegionId = RegionData.FindOrAddRegionId(ThisKey);

						for (int NeighborIdx = 0; NeighborIdx < 4; NeighborIdx++)
						{
							int NeighborSpanIndex = ThisOpenSpan.NeighborLink[NeighborIdx];
							if (NeighborSpanIndex == -1)
							{
								continue;
							}

							OpenSpanKey NeighborKey(
								x + NeighborOffset[NeighborIdx].x,
								z + NeighborOffset[NeighborIdx].z,
								NeighborSpanIndex
							);

							int NeighborRegionId = RegionData.FindOrAddRegionId(NeighborKey);
							if (NeighborRegionId != -1)
							{
								// Only set region ids if a span is next to one from previous distance field.
								// Note: This avoids one region expanding too fast, taking up spaces next to other regions.
								if (GetOpenSpanByKey(NeighborKey).DistanceField > DistanceFieldIdx)
								{
									RegionData.SetRegionId(ThisKey, NeighborRegionId);
									ThisRegionId = NeighborRegionId;
								}
							}
						}

						if (ThisRegionId == -1)
						{
							// No connection to known regions so far, put it into a pending list
							IsolatedSpans.push_back(ThisKey);
						}

						RegionData.SetRegionId(ThisKey, ThisRegionId);
					}

					SpanIndex++;
				}
			}
		}

		// Before making new regions, let's try merging isolated spans into know regions
		if (IsolatedSpans.size() > 0)
		{
			int MergedSpans;
			do
			{
				MergedSpans = 0;
				std::vector<OpenSpanKey> IgnoredSpans;
				for (int i = (int)IsolatedSpans.size() - 1; i >= 0; i--)
				{
					OpenSpanKey& OtherSpan = IsolatedSpans[i];

					if (RegionData.SetRegionIdFromAdjacency(OtherSpan, &IgnoredSpans))
					{
						// Add this span to the ignore list so we limit the expansion to one span per iteration.
						// This will help evenly divide long expansion spans into two regions.
						IgnoredSpans.push_back(OtherSpan);

						IsolatedSpans.erase(IsolatedSpans.begin() + i);
						MergedSpans++;
					}
				}
			} while (MergedSpans > 0);
		}

		// Make isolated spans into new regions
		while (IsolatedSpans.size() > 0)
		{
			int NewRegion = NumRegions++;

			// Assign new region to the span
			OpenSpanKey Span = IsolatedSpans[0];
			RegionData.SetRegionId(Span, NewRegion);

			// Remove first span
			IsolatedSpans.erase(IsolatedSpans.begin());

			int MergedSpans;
			do 
			{
				MergedSpans = 0;
				for (int i = (int)IsolatedSpans.size() - 1; i >= 0; i--)
				{
					OpenSpanKey& OtherSpan = IsolatedSpans[i];

					if (RegionData.SetRegionIdFromAdjacency(OtherSpan))
					{
						IsolatedSpans.erase(IsolatedSpans.begin() + i);
						MergedSpans++;
					}
				}
			} while (MergedSpans > 0);
		}

		DebugRegionMaps[DistanceFieldIdx] = RegionData.GetRegionMapRef();
	}

	UniqueRegionIds.clear();
	
	// Assign final region ids to all spans
	for (auto Iter : RegionData.GetRegionMapRef())
	{
		auto& Span = GetOpenSpanByKey(Iter.first);
		Span.RegionId = Iter.second;
		UniqueRegionIds.insert(Span.RegionId);
	}
}

void RNavMeshGenerator::GenerateRegionContours()
{
	RegionEdgePoints.resize(UniqueRegionIds.size());

	for (const auto& RegionId : UniqueRegionIds)
	{
		// First, we need to find a span belong to a region
		int x, z, SpanIdx;
		bool bFoundSpanForRegion = false;

		for (x = 0; x < CellNumX; x++)
		{
			for (z = 0; z < CellNumZ; z++)
			{
				int Index = x * CellNumZ + z;
				const auto& Column = Heightfield[Index];

				for (SpanIdx = 0; SpanIdx < Column.OpenSpans.size(); SpanIdx++)
				{ 
					const HeightfieldOpenSpan& Span = Column.OpenSpans[SpanIdx];

					if (Span.RegionId == RegionId)
					{
						bFoundSpanForRegion = true;
						break;
					}
				}

				if (bFoundSpanForRegion)
				{
					break;
				}
			}

			if (bFoundSpanForRegion)
			{
				break;
			}
		}

		assert(bFoundSpanForRegion);
		OpenSpanKey Key(x, z, SpanIdx);

		int StartDirection = 0;
		OpenSpanKey StartKey = FindRegionEdgeInDirection(Key, StartDirection);

		OpenSpanKey CurrentKey(StartKey);
		int CurrentDirection = StartDirection;

		int LastNeighborRegionId = INT_MAX;

		RLog("Begin edge searching for region %d\n", RegionId);
		do
		{
			const HeightfieldOpenSpan& CurrentSpan = GetOpenSpanByKey(CurrentKey);
			if (CurrentSpan.NeighborLink[CurrentDirection] == -1)
			{
				// A mandatory point is a point shared by three or more regions including null regions
				bool bMandatoryPoint = (LastNeighborRegionId != INT_MAX && LastNeighborRegionId != -1);

				// No more neighbors in this direction, turn 90 to the right
				AddEdgePoint(CurrentKey, CurrentDirection, RegionId, bMandatoryPoint);
				LastNeighborRegionId = -1;
				CurrentDirection++;
				while (CurrentDirection >= 4) { CurrentDirection -= 4; }
			}
			else
			{
				// Is facing an edge?
				OpenSpanKey NextKey(
					CurrentKey.x + NeighborOffset[CurrentDirection].x,
					CurrentKey.z + NeighborOffset[CurrentDirection].z,
					CurrentSpan.NeighborLink[CurrentDirection]
				);

				const HeightfieldOpenSpan& NextSpan = GetOpenSpanByKey(NextKey);
				if (NextSpan.RegionId != CurrentSpan.RegionId)
				{
					// A mandatory point is a point shared by three or more regions including null regions
					bool bMandatoryPoint = (LastNeighborRegionId != INT_MAX && LastNeighborRegionId != NextSpan.RegionId);
					
					// Region edge, turn 90 to the right
					AddEdgePoint(CurrentKey, CurrentDirection, RegionId, bMandatoryPoint);
					LastNeighborRegionId = NextSpan.RegionId;
					CurrentDirection++;
					while (CurrentDirection >= 4) { CurrentDirection -= 4; }
				}
				else
				{
					// Move forward and turn 90 to the left
					CurrentKey = NextKey;
					CurrentDirection--;
					while (CurrentDirection < 0) { CurrentDirection += 4; }
				}
			}
		} while (CurrentKey != StartKey || CurrentDirection != StartDirection);

		// Simplify edges
		EdgePointCollection& RegionEdges = RegionEdgePoints[RegionId];

		// Find a first mandatory point
		int FirstIdx;
		for (FirstIdx = 0; FirstIdx < (int)RegionEdges.size(); FirstIdx++)
		{
			if (RegionEdges[FirstIdx].bIsMandatory)
			{
				break;
			}
		}
		
		if (FirstIdx != (int)RegionEdges.size())
		{
			int StartIdx = FirstIdx;
			int EndIdx = -1;
			std::vector<RVec3> SimplifiedPoints;

			do
			{
				std::vector<RVec3> Points;
				for (int i = 0; i <= (int)RegionEdges.size(); i++)
				{
					int Idx = (StartIdx + i) % RegionEdges.size();
					Points.push_back(RegionEdges[Idx].Point);
					if (i != 0 && RegionEdges[Idx].bIsMandatory)
					{
						EndIdx = Idx;
						break;
					}
				}

				assert(EndIdx != -1);

				std::vector<RVec3> Simplified = SimplifyEdges(Points);
				SimplifiedPoints.insert(SimplifiedPoints.end(), Simplified.begin(), Simplified.end());
				StartIdx = EndIdx;
			} while (StartIdx != FirstIdx);

			// For testing
			std::vector<EdgePointData> NewEdgePoints;
			for (const auto& p : SimplifiedPoints)
			{
				NewEdgePoints.push_back({ p, true });
			}

			RegionEdges = NewEdgePoints;
		}
		else
		{
			// When a region is not sharing any edges with other regions, it doesn't have a mandatory edge point.
			// We will simplify geometries by combining shorter line segments into a longer one. 
			int StartIndex = 0;
			do
			{
				const RVec3& p0 = RegionEdges[StartIndex].Point;
				const RVec3& p1 = RegionEdges[(StartIndex + 1) % RegionEdges.size()].Point;
				const RVec3& p2 = RegionEdges[(StartIndex + 2) % RegionEdges.size()].Point;

				if (FLT_EQUAL(RVec3::Dot((p1 - p0).GetNormalized(), (p2 - p0).GetNormalized()), 1.0f))
				{
					RegionEdges.erase(RegionEdges.begin() + StartIndex + 1);
				}
				else
				{
					StartIndex++;
				}
			} while (StartIndex < RegionEdges.size());
		}
	}
}


void RNavMeshGenerator::TriangulateRegions(RNavMeshData& OutNavMeshData)
{
	int RegionId = 0;
	for (auto& TraverseEdges : RegionEdgePoints)
	{
		RLog("Triangulate polygons for region %d\n", RegionId);

		EdgePointCollection Edges = TraverseEdges;
		
		// Keep a list of edge point indices
		std::vector<int> EdgePointIndices;
		EdgePointIndices.resize(Edges.size());
		for (int i = 0; i < (int)EdgePointIndices.size(); i++)
		{
			EdgePointIndices[i] = i;
		}

		// Find a valid shortest partition

		while (1)
		{ 
			// Index of a point for a starting edge. The edge is represented as n and n + 2
			int PtIdx0 = FindShortestPartition(Edges);
			if (PtIdx0 == -1)
			{
				break;
			}
		
			int PtIdx1 = (PtIdx0 + 1) % Edges.size();
			int PtIdx2 = (PtIdx0 + 2) % Edges.size();

			const RVec3 p0 = Edges[PtIdx0].Point;
			const RVec3 p1 = Edges[PtIdx1].Point;
			const RVec3 p2 = Edges[PtIdx2].Point;

			OutNavMeshData.AddTriangle(p0, p1, p2, RegionId);

#if 1
			// Add edges to debugger for debug rendering
			{
				const RVec3 Offset(0.0f, 1.0f, 0.0f);
				
				GNavigationSystem.GetDebugger().AddRegionEdge(RegionId, p0 + Offset, p1 + Offset);
				GNavigationSystem.GetDebugger().AddRegionEdge(RegionId, p0 + Offset, p2 + Offset);
				GNavigationSystem.GetDebugger().AddRegionEdge(RegionId, p1 + Offset, p2 + Offset);
			}
#endif // 0

			// Removed a point after triangulating it with its direct neighbors
			Edges.erase(Edges.begin() + PtIdx1);
			EdgePointIndices.erase(EdgePointIndices.begin() + PtIdx1);
		}

		RegionId++;
	}
}

OpenSpanKey RNavMeshGenerator::FindRegionEdgeInDirection(const OpenSpanKey& Key, int DirectionIdx /*= 0*/) const
{
	const HeightfieldOpenSpan* RegionSpan = &GetOpenSpanByKey(Key);

	int CurrentSpanIdx = Key.span_idx;

	// Move in one direction until we find an edge
	int NextSpanIdx = RegionSpan->NeighborLink[DirectionIdx];
	int nx = Key.x, nz = Key.z;
	while (NextSpanIdx != -1)
	{
		OpenSpanKey CurrentKey(nx, nz, CurrentSpanIdx);

		nx += NeighborOffset[DirectionIdx].x;
		nz += NeighborOffset[DirectionIdx].z;
		OpenSpanKey NextKey(nx, nz, NextSpanIdx);
		const HeightfieldOpenSpan& NextSpan = GetOpenSpanByKey(NextKey);
		if (NextSpan.RegionId != RegionSpan->RegionId)
		{
			return CurrentKey;
		}

		RegionSpan = &NextSpan;
		CurrentSpanIdx = NextSpanIdx;
		NextSpanIdx = RegionSpan->NeighborLink[DirectionIdx];
	}

	return OpenSpanKey(nx, nz, CurrentSpanIdx);
}

void RNavMeshGenerator::AddEdgePoint(const OpenSpanKey& Key, int DirectionIdx, int RegionId, bool bMendatoryPoint)
{
	//RLog("Edge found! loc: (%d, %d), dir: %d - ", Key.x, Key.z, DirectionIdx);

	assert(RegionId >= 0 && RegionId < RegionEdgePoints.size());

	RVec3 CenterPoint = GetCellCenter(Key.x, GetOpenSpanByKey(Key).CellRowStart, Key.z);
	RVec3 Offset = CellDimension;
	switch (DirectionIdx)
	{
	case 0:
		Offset = Offset * RVec3(-.5f, 0, -.5f);
		break;
	case 1:
		Offset = Offset * RVec3(-.5f, 0, .5f);
		break;
	case 2:
		Offset = Offset * RVec3(.5f, 0, .5f);
		break;
	case 3:
		Offset = Offset * RVec3(.5f, 0, -.5f);
		break;
	}

	RVec3 EdgePoint = CenterPoint + Offset;
	RegionEdgePoints[RegionId].push_back({ EdgePoint, bMendatoryPoint });
	//RLog("Add edge point: %s\n", EdgePoint.ToString().c_str());
}

RAabb RNavMeshGenerator::CalculateBoundsForSpan(const HeightfieldSolidSpan& Span, int x, int z) const
{
	RAabb Result;
	float Scale = 1.0f;

	int y = Span.CellRowStart;
	RVec3 CellCenter = GetCellCenter(x, y, z);
	Result.pMin = CellCenter - CellDimension * 0.4f * Scale;

	y = Span.CellRowEnd;
	CellCenter = GetCellCenter(x, y, z);
	Result.pMax = CellCenter + CellDimension * 0.4f * Scale;
	return Result;
}

bool RNavMeshGenerator::IsValidNeighborSpan(const HeightfieldOpenSpan& ThisOpenSpan, const HeightfieldOpenSpan& NeighborOpenSpan) const
{
	return abs(NeighborOpenSpan.CellRowStart - ThisOpenSpan.CellRowStart) <= MaxStepNumCells &&
		   (NeighborOpenSpan.CellRowEnd - ThisOpenSpan.CellRowStart) > MinTraversableNumCells;
}

HeightfieldOpenSpan& RNavMeshGenerator::GetOpenSpanByKey(const OpenSpanKey& Key)
{
	assert(Key.x >= 0 && Key.x < CellNumX && Key.z >= 0 && Key.z < CellNumZ);
	int Index = Key.x * CellNumZ + Key.z;

	auto& OpenSpans = Heightfield[Index].OpenSpans;
	assert(Key.span_idx >= 0 && Key.span_idx < OpenSpans.size());

	return OpenSpans[Key.span_idx];
}

const HeightfieldOpenSpan& RNavMeshGenerator::GetOpenSpanByKey(const OpenSpanKey& Key) const
{
	return const_cast<RNavMeshGenerator*>(this)->GetOpenSpanByKey(Key);
}

OpenSpanKey RNavMeshGenerator::GetNeighborSpanByIndex(const OpenSpanKey& Key, int OffsetIndex) const
{
	auto& Span = GetOpenSpanByKey(Key);

	assert(OffsetIndex >= 0 && OffsetIndex < NUM_NEIGHBOR_SPANS);
	int NeighborSpanIdx = Span.NeighborLink[OffsetIndex];
	if (NeighborSpanIdx != -1)
	{
		return OpenSpanKey(
			Key.x + NeighborOffset[OffsetIndex].x,
			Key.z + NeighborOffset[OffsetIndex].z,
			NeighborSpanIdx
		);
	}

	return OpenSpanKey::Invalid;
}

RVec3 RNavMeshGenerator::GetCellCenter(int x, int y, int z) const
{
	assert(x >= 0 && x < CellNumX && y >= 0 && y < CellNumY && z >= 0 && z < CellNumZ);
	return RVec3(
		SceneCenterPoint.X() + (-0.5f * (CellNumX - 1) + x) * CellDimension.X(),
		SceneCenterPoint.Y() + (-0.5f * (CellNumY - 1) + y) * CellDimension.Y(),
		SceneCenterPoint.Z() + (-0.5f * (CellNumZ - 1) + z) * CellDimension.Z()
	);
}

void RNavMeshGenerator::DebugDrawSpans() const
{
	const auto& RegionMap = DebugRegionMaps[DebugDistanceField];

	for (const auto& Column : Heightfield)
	{
		// Draw solid spans
		//for (const auto& Span : Column.SolidSpans)
		//{
		//	GDebugRenderer.DrawAabb(Span.Bounds, Span.bTraversable ? RColor::Green : RColor::Red);
		//}

		int SpanIndex = 0;

		// Draw traversable areas
		for (const auto& OpenSpan : Column.OpenSpans)
		{
			if (OpenSpan.DistanceField >= DebugDistanceField)
			{
				OpenSpanKey Key(Column.x, Column.z, SpanIndex);
				auto Iter = RegionMap.find(Key);
				if (Iter != RegionMap.end())
				{
					int RegionId = Iter->second;
					if (RegionId != -1)
					{
						RVec3 CellPosition = GetCellCenter(Column.x, OpenSpan.CellRowStart, Column.z);
						GDebugRenderer.DrawSphere(CellPosition, CellDimension.X() * 0.5f, DebugRegionColors[RegionId], 4);
					}
				}
			}

			for (int i = 0; i < NUM_NEIGHBOR_SPANS; i++)
			{
				int NeighborLinkIndex = OpenSpan.NeighborLink[i];
				if (NeighborLinkIndex != -1)
				{
					const int n_x = Column.x + NeighborOffset[i].x;
					const int n_z = Column.z + NeighborOffset[i].z;

					if (n_x >= 0 && n_x < CellNumX &&
						n_z >= 0 && n_z < CellNumZ)
					{
						int NeighborIndex = n_x * CellNumZ + n_z;
						const HeightfieldOpenSpan& NeighborOpenSpan = Heightfield[NeighborIndex].OpenSpans[NeighborLinkIndex];
						int NeighborStart = NeighborOpenSpan.CellRowStart;

						RVec3 Start = GetCellCenter(Column.x, OpenSpan.CellRowStart, Column.z);
						RVec3 End = GetCellCenter(n_x, NeighborStart, n_z);

						GDebugRenderer.DrawLine(Start, End, RColor(1.0f, 0.5f, 0.5f));
}
				}
			}

			SpanIndex++;
		}
	}
}

void RNavMeshGenerator::IncreaseDebugDistanceFieldLevel()
{
	DebugDistanceField++;
	if (DebugDistanceField > MaxDistanceField)
	{
		DebugDistanceField = 0;
	}

	RLog("Drawing debug distance field %d\n", DebugDistanceField);
}

void RNavMeshGenerator::DecreaseDebugDistanceFieldLevel()
{
	DebugDistanceField--;
	if (DebugDistanceField < 0)
	{
		DebugDistanceField = MaxDistanceField;
	}

	RLog("Drawing debug distance field %d\n", DebugDistanceField);
}
