//=============================================================================
// RVoxelizer.cpp by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RVoxelizer.h"
#include "Core/RAabb.h"
#include "RenderSystem/RDebugRenderer.h"


namespace
{
	// For debug rendering regions in different colors
	vector<RColor> DebugRegionColors;

	// For debug rendering the region map at various heightfield
	vector<map<OpenSpanKey, int>> DebugRegionMaps;

	struct EdgePoint
	{
		RVec3 Point;
		bool bIsMandatory;
	};
	typedef vector<EdgePoint> EdgePoints;
	vector<EdgePoints> DebugRegionEdgePoints;

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
	vector<RVec3> SimplifyEdges(const vector<RVec3>& Edges)
	{
		assert(Edges.size() >= 2);

		if (Edges.size() == 2)
		{
			return vector<RVec3>{ Edges[0] };
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
			return vector<RVec3>{ Edges[0] };
		}
		else
		{
			// Taking the point with a max distance to the line segment as a middle point,
			// split the edges into two sublists and perform the algorithm on each of them.
			auto Left = SimplifyEdges(vector<RVec3>(Edges.begin(), Edges.begin() + MaxPointIdx + 1));
			auto Right = SimplifyEdges(vector<RVec3>(Edges.begin() + MaxPointIdx, Edges.end()));

			vector<RVec3> Result;
			Result.reserve(Left.size() + Right.size());
			Result.insert(Result.end(), Left.begin(), Left.end());
			Result.insert(Result.end(), Right.begin(), Right.end());

			return Result;
		}
	}
}

const OpenSpanKey OpenSpanKey::Invalid(-1, -1, -1);

const GridCoord RVoxelizer::NeighbourOffset[8] =
{
	// Direct neighbour grids
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


RVoxelizer::RVoxelizer()
	: CellDimension(100.0f, 20.0f, 100.0f)
	, MinTraversableHeight(200.0f)
	, MaxStepHeight(50.0f)
{
	MinTraversableNumCells = (int)ceil(MinTraversableHeight / CellDimension.Y());
	MaxStepNumCells = (int)floor(MaxStepHeight / CellDimension.Y());
}

void RVoxelizer::Initialize(RScene* Scene)
{
	RLog("Initalizing voxelizer from static level meshes.\n");

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
		Heightfield.resize(CellNumX * CellNumZ);

		GenerateHeightfieldColumns(SceneObjects);
		GenerateOpenSpanNeighbourData();
		GenerateDistanceField();
		GenerateRegions();
		GenerateRegionContours();
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
}

void RVoxelizer::Render()
{
	GDebugRenderer.DrawAabb(SceneBounds);

	static int DebugDistanceField = MaxDistanceField;

	// '[' key
	if (RInput.GetBufferedKeyState(VK_OEM_4) == EBufferedKeyState::Pressed)
	{
		DebugDistanceField++;
		if (DebugDistanceField > MaxDistanceField)
		{
			DebugDistanceField = 0;
		}

		RLog("Drawing debug distance field %d\n", DebugDistanceField);
	}

	// ']' key
	if (RInput.GetBufferedKeyState(VK_OEM_6) == EBufferedKeyState::Pressed)
	{
		DebugDistanceField--;
		if (DebugDistanceField < 0)
		{
			DebugDistanceField = MaxDistanceField;
		}

		RLog("Drawing debug distance field %d\n", DebugDistanceField);
	}

	{
		static int DebugDrawRegionId = 0;

		if (RInput.GetBufferedKeyState(VK_OEM_PLUS) == EBufferedKeyState::Pressed)
		{
			DebugDrawRegionId++;
			if (DebugDrawRegionId >= DebugRegionEdgePoints.size())
			{
				DebugDrawRegionId = 0;
			}
		}

		if (RInput.GetBufferedKeyState(VK_OEM_MINUS) == EBufferedKeyState::Pressed)
		{
			DebugDrawRegionId--;
			if (DebugDrawRegionId < 0)
			{
				DebugDrawRegionId = (int)DebugRegionEdgePoints.size() - 1;
			}
		}

		const auto& RegionEdges = DebugRegionEdgePoints[DebugDrawRegionId];
		for (int i = 0; i < RegionEdges.size(); i++)
		{
			const RVec3& p0 = RegionEdges[i].Point;
			const RVec3& p1 = RegionEdges[(i + 1) % RegionEdges.size()].Point;
			GDebugRenderer.DrawLine(p0, p1, RColor::Cyan);

			if (RegionEdges[i].bIsMandatory)
			{
				GDebugRenderer.DrawSphere(p0, 20.0f, RColor::Cyan, 8);
			}
			else
			{
				GDebugRenderer.DrawSphere(p0, 10.0f, RColor::Green, 8);
			}
		}
	}

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

			for (int i = 0; i < NUM_NEIGHBOUR_SPANS; i++)
			{
				int NeighbourLinkIndex = OpenSpan.NeighbourLink[i];
				if (NeighbourLinkIndex != -1)
				{
					const int n_x = Column.x + NeighbourOffset[i].x;
					const int n_z = Column.z + NeighbourOffset[i].z;

					if (n_x >= 0 && n_x < CellNumX &&
						n_z >= 0 && n_z < CellNumZ)
					{
						int NeighbourIndex = n_x * CellNumZ + n_z;
						const HeightfieldOpenSpan& NeighbourOpenSpan = Heightfield[NeighbourIndex].OpenSpans[NeighbourLinkIndex];
						int NeighbourStart = NeighbourOpenSpan.CellRowStart;

						RVec3 Start = GetCellCenter(Column.x, OpenSpan.CellRowStart, Column.z);
						RVec3 End = GetCellCenter(n_x, NeighbourStart, n_z);

						GDebugRenderer.DrawLine(Start, End, RColor(1.0f, 0.5f, 0.5f));
					}
				}
			}

			SpanIndex++;
		}
	}
}

void RVoxelizer::GenerateHeightfieldColumns(const vector<RSceneObject*>& SceneObjects)
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
				CellBounds.pMax = CellCenter + CellDimension / 2.0f;
				CellBounds.pMin = CellCenter - CellDimension / 2.0f;

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

void RVoxelizer::GenerateOpenSpanNeighbourData()
{
	for (int x = 0; x < CellNumX; x++)
	{
		for (int z = 0; z < CellNumZ; z++)
		{
			int Index = x * CellNumZ + z;
			auto& Column = Heightfield[Index];

			for (auto& ThisOpenSpan : Column.OpenSpans)
			{
				int NumValidNeighbours = 0;
				for (int NeighbourLinkIndex = 0; NeighbourLinkIndex < 8; NeighbourLinkIndex++)
				{
					// Neighbour coordinates
					const int n_x = x + NeighbourOffset[NeighbourLinkIndex].x;
					const int n_z = z + NeighbourOffset[NeighbourLinkIndex].z;

					if (n_x >= 0 && n_x < CellNumX &&
						n_z >= 0 && n_z < CellNumZ)
					{
						int NeighbourIndex = n_x * CellNumZ + n_z;

						if (NeighbourLinkIndex < NUM_NEIGHBOUR_SPANS)
						{
							// Check direct neighbours and add valid ones to the neighbour link list
							int NeighbourOpenSpanIndex = 0;

							// Find linked neighbour open spans
							for (const auto& NeighbourOpenSpan : Heightfield[NeighbourIndex].OpenSpans)
							{
								if (IsValidNeighbourSpan(ThisOpenSpan, NeighbourOpenSpan))
								{
									ThisOpenSpan.NeighbourLink[NeighbourLinkIndex] = NeighbourOpenSpanIndex;
									NumValidNeighbours++;
									break;
								}

								NeighbourOpenSpanIndex++;
							}
						}
						else
						{
							// Diagonal neighbours are only used to check for border spans
							for (const auto& NeighbourOpenSpan : Heightfield[NeighbourIndex].OpenSpans)
							{
								if (IsValidNeighbourSpan(ThisOpenSpan, NeighbourOpenSpan))
								{
									NumValidNeighbours++;
									break;
								}
							}
						}
					}
				}

				// Span is a border if at least one of eight neighbours is not walkable from it
				ThisOpenSpan.bBorder = (NumValidNeighbours < 8);
			}
		}
	}
}

void RVoxelizer::GenerateDistanceField()
{
	// TODO: preallocate data with total open span count
	queue<OpenSpanKey> PendingSpans;

	// The result of each span and its distance field
	map<OpenSpanKey, int> SpanDistanceMap;

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

	// Loop through all neighbour spans and add them to the end of the container
	while (PendingSpans.size() != 0)
	{
		// Get one element from the queue in the front
		const OpenSpanKey& ThisSpanKey = PendingSpans.front();
		//RLog("Span: %d, %d\n", ThisSpanData.x, ThisSpanData.z);

		int ColumnIndex = ThisSpanKey.x * CellNumZ + ThisSpanKey.z;
		auto& Column = Heightfield[ColumnIndex];

		for (int i = 0; i < 4; i++)
		{
			int NeighbourSpanIndex = Column.OpenSpans[ThisSpanKey.span_idx].NeighbourLink[i];
			if (NeighbourSpanIndex != -1)
			{
				// New span data with the distance field
				OpenSpanKey NewKey(
					ThisSpanKey.x + NeighbourOffset[i].x,
					ThisSpanKey.z + NeighbourOffset[i].z,
					NeighbourSpanIndex);

				// Neighbour distance = current distance + 1
				int NewDistance = SpanDistanceMap.find(ThisSpanKey)->second + 1;

				auto Iter = SpanDistanceMap.find(NewKey);

				// If this neighbour is a new span, added it to the queue
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

		Column.OpenSpans[Span.first.span_idx].DistanceField = Span.second;

		if (Span.second > MaxDistanceField)
		{
			MaxDistanceField = Span.second;
		}
	}
}

void RVoxelizer::GenerateRegions()
{
	int NumRegions = 0;

	typedef map<OpenSpanKey, int> RegionMapType;
	RegionMapType RegionMap;

	// Get a region id for a span key. Create a default region id for the key if the id doesn't exist
	auto GetRegionId = [&RegionMap](const OpenSpanKey& Key) -> int {
		auto Iter = RegionMap.find(Key);
		if (Iter == RegionMap.end())
		{
			RegionMap[Key] = -1;
			return -1;
		}
		else
		{
			return Iter->second;
		}
	};

	auto SetRegionId = [&RegionMap](const OpenSpanKey& Key, int RegionId) -> void {
		RegionMap[Key] = RegionId;
	};

	// Find region id for a span from its adjacent spans
	auto SetRegionIdFromAdjacency = [this, &RegionMap, &GetRegionId, &SetRegionId](const OpenSpanKey& Key, const vector<OpenSpanKey>* IgnoredNeighbours = nullptr) -> bool {
		auto& Span = this->GetOpenSpanByKey(Key);
		int DiagonalRegionId = -1;
		for (int NeighbourIdx = 0; NeighbourIdx < 4; NeighbourIdx++)
		{
			int NeighbourSpanIndex = Span.NeighbourLink[NeighbourIdx];
			if (NeighbourSpanIndex == -1)
			{
				continue;
			}

			int nx = Key.x + NeighbourOffset[NeighbourIdx].x;
			int nz = Key.z + NeighbourOffset[NeighbourIdx].z;

			OpenSpanKey NeighbourKey(nx, nz, NeighbourSpanIndex);
			if (IgnoredNeighbours && StdContains(*IgnoredNeighbours, NeighbourKey))
			{
				continue;
			}

			int NeighbourRegionId = GetRegionId(NeighbourKey);
			if (NeighbourRegionId != -1)
			{
				SetRegionId(Key, NeighbourRegionId);
				return true;
			}

			if (DiagonalRegionId == -1)
			{
				// Diagonal searching pattern:
				// (x: current span; n: neighbour span; o: diagonal span)
				//
				// o n o         o   o
				//   x     and   n x n
				// o n o         o   o
				// 

				int OffsetIdx[2];
				if (NeighbourIdx % 2 == 0)
				{
					OffsetIdx[0] = 1;
					OffsetIdx[1] = 3;
				}
				else
				{
					OffsetIdx[0] = 0;
					OffsetIdx[1] = 2;
				}

				for (int i = 0; i < 2; i++)
				{
					OpenSpanKey DiagonalNeighbourKey = GetNeighbourSpanByIndex(NeighbourKey, OffsetIdx[i]);

					if (IgnoredNeighbours && StdContains(*IgnoredNeighbours, DiagonalNeighbourKey))
					{
						continue;
					}

					if (DiagonalNeighbourKey.IsValid())
					{
						DiagonalRegionId = GetRegionId(DiagonalNeighbourKey);
						break;
					}
				}
			}
		}

		// Diagonal neighbours
		if (DiagonalRegionId != -1)
		{
			SetRegionId(Key, DiagonalRegionId);
			return true;
		}

		return false;
	};

	DebugRegionMaps.resize(MaxDistanceField + 1);

	for (int DistanceFieldIdx = MaxDistanceField; DistanceFieldIdx >= 0; DistanceFieldIdx--)
	{
		// New spans that do not directly connect to any existing regions
		vector<OpenSpanKey> IsolatedSpans;

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
						// 1. At least one neighbour is from a previous distance field: Set region id to neighbour's region id
						// 2. Neighbour has no direct connection to previous spans. Possibly:
						//    - Span is connecting to one or more existing regions through other spans
						//    - Span is forming a new region (if no connections are made to existing regions)

						OpenSpanKey ThisKey(x, z, SpanIndex);
						int ThisRegionId = GetRegionId(ThisKey);

						for (int NeighbourIdx = 0; NeighbourIdx < 4; NeighbourIdx++)
						{
							int NeighbourSpanIndex = ThisOpenSpan.NeighbourLink[NeighbourIdx];
							if (NeighbourSpanIndex == -1)
							{
								continue;
							}

							OpenSpanKey NeighbourKey(
								x + NeighbourOffset[NeighbourIdx].x,
								z + NeighbourOffset[NeighbourIdx].z,
								NeighbourSpanIndex
							);

							int NeighbourRegionId = GetRegionId(NeighbourKey);
							if (NeighbourRegionId != -1)
							{
								// Only set region ids if a span is next to one from previous distance field.
								// Note: This avoids one region expanding too fast, taking up spaces next to other regions.
								if (GetOpenSpanByKey(NeighbourKey).DistanceField > DistanceFieldIdx)
								{
									SetRegionId(ThisKey, NeighbourRegionId);
									ThisRegionId = NeighbourRegionId;
								}
							}
						}

						if (ThisRegionId == -1)
						{
							// No connection to known regions so far, put it into a pending list
							IsolatedSpans.push_back(ThisKey);
						}

						SetRegionId(ThisKey, ThisRegionId);
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
				vector<OpenSpanKey> IgnoredSpans;
				for (int i = (int)IsolatedSpans.size() - 1; i >= 0; i--)
				{
					OpenSpanKey& OtherSpan = IsolatedSpans[i];

					if (SetRegionIdFromAdjacency(OtherSpan, &IgnoredSpans))
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
			SetRegionId(Span, NewRegion);

			// Remove first span
			IsolatedSpans.erase(IsolatedSpans.begin());

			int MergedSpans;
			do 
			{
				MergedSpans = 0;
				for (int i = (int)IsolatedSpans.size() - 1; i >= 0; i--)
				{
					OpenSpanKey& OtherSpan = IsolatedSpans[i];

					if (SetRegionIdFromAdjacency(OtherSpan))
					{
						IsolatedSpans.erase(IsolatedSpans.begin() + i);
						MergedSpans++;
					}
				}
			} while (MergedSpans > 0);
		}

		DebugRegionMaps[DistanceFieldIdx] = RegionMap;
	}

	UniqueRegionIds.clear();
	
	// Assign final region ids to all spans
	for (auto Iter : RegionMap)
	{
		auto& Span = GetOpenSpanByKey(Iter.first);
		Span.RegionId = Iter.second;
		UniqueRegionIds.insert(Span.RegionId);
	}
}

void RVoxelizer::GenerateRegionContours()
{
	DebugRegionEdgePoints.resize(UniqueRegionIds.size());

	for (const auto& RegionId : UniqueRegionIds)
	{
		// First, we need to find a span belong to a region
		int x, z, SpanIdx;
		bool bFoundSpanForRegion = false;

		for (x = 0; x < CellNumX && !bFoundSpanForRegion; x++)
		{
			for (z = 0; z < CellNumZ && !bFoundSpanForRegion; z++)
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
			}
		}

		assert(bFoundSpanForRegion);
		OpenSpanKey Key(x, z, SpanIdx);

		int StartDirection = 0;
		OpenSpanKey StartKey = FindRegionEdgeInDirection(Key, StartDirection);

		OpenSpanKey CurrentKey(StartKey);
		int CurrentDirection = StartDirection;

		CurrentDirection++;
		while (CurrentDirection >= 4) { CurrentDirection -= 4; }
		int LastNeighbourRegionId = INT_MAX;

		RLog("Begin edge searching for region %d\n", RegionId);
		do
		{
			const HeightfieldOpenSpan& CurrentSpan = GetOpenSpanByKey(CurrentKey);
			if (CurrentSpan.NeighbourLink[CurrentDirection] == -1)
			{
				// A mandatory point is a point shared by three or more regions including null regions
				bool bMandatoryPoint = (LastNeighbourRegionId != INT_MAX && LastNeighbourRegionId != -1);

				// No more neighbours in this direction, turn 90 to the right
				AddEdge(CurrentKey, CurrentDirection, RegionId, bMandatoryPoint);
				LastNeighbourRegionId = -1;
				CurrentDirection++;
				while (CurrentDirection >= 4) { CurrentDirection -= 4; }
			}
			else
			{
				// Is facing an edge?
				OpenSpanKey NextKey(
					CurrentKey.x + NeighbourOffset[CurrentDirection].x,
					CurrentKey.z + NeighbourOffset[CurrentDirection].z,
					CurrentSpan.NeighbourLink[CurrentDirection]
				);

				const HeightfieldOpenSpan& NextSpan = GetOpenSpanByKey(NextKey);
				if (NextSpan.RegionId != CurrentSpan.RegionId)
				{
					// A mandatory point is a point shared by three or more regions including null regions
					bool bMandatoryPoint = (LastNeighbourRegionId != INT_MAX && LastNeighbourRegionId != NextSpan.RegionId);
					
					// Region edge, turn 90 to the right
					AddEdge(CurrentKey, CurrentDirection, RegionId, bMandatoryPoint);
					LastNeighbourRegionId = NextSpan.RegionId;
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
		EdgePoints& RegionEdges = DebugRegionEdgePoints[RegionId];

		// Find a first mandatory point
		int FirstIdx;
		for (FirstIdx = 0; FirstIdx < (int)RegionEdges.size(); FirstIdx++)
		{
			if (RegionEdges[FirstIdx].bIsMandatory)
			{
				break;
			}
		}
		
		// Note: When a region is not sharing any edges with other regions, it doesn't have a mandatory edge point.
		//		 In this case we'll use the first point for both start and end index.

		if (FirstIdx != (int)RegionEdges.size())
		{
			int StartIdx = FirstIdx;
			int EndIdx = -1;
			vector<RVec3> SimplifiedPoints;

			do
			{
				vector<RVec3> Points;
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

				vector<RVec3> Simplified = SimplifyEdges(Points);
				SimplifiedPoints.insert(SimplifiedPoints.end(), Simplified.begin(), Simplified.end());
				StartIdx = EndIdx;
			} while (StartIdx != FirstIdx);

			// For testing
			vector<EdgePoint> NewEdgePoints;
			for (const auto& p : SimplifiedPoints)
			{
				NewEdgePoints.push_back({ p, true });
			}

			RegionEdges = NewEdgePoints;
		}
	}
}

OpenSpanKey RVoxelizer::FindRegionEdgeInDirection(const OpenSpanKey& Key, int DirectionIdx /*= 0*/) const
{
	const HeightfieldOpenSpan* RegionSpan = &GetOpenSpanByKey(Key);

	int CurrentSpanIdx = Key.span_idx;

	// Move in one direction until we find an edge
	int NextSpanIdx = RegionSpan->NeighbourLink[DirectionIdx];
	int nx = Key.x, nz = Key.z;
	while (NextSpanIdx != -1)
	{
		OpenSpanKey CurrentKey(nx, nz, CurrentSpanIdx);

		nx += NeighbourOffset[DirectionIdx].x;
		nz += NeighbourOffset[DirectionIdx].z;
		OpenSpanKey NextKey(nx, nz, NextSpanIdx);
		const HeightfieldOpenSpan& NextSpan = GetOpenSpanByKey(NextKey);
		if (NextSpan.RegionId != RegionSpan->RegionId)
		{
			return CurrentKey;
		}

		RegionSpan = &NextSpan;
		CurrentSpanIdx = NextSpanIdx;
		NextSpanIdx = RegionSpan->NeighbourLink[DirectionIdx];
	}

	return OpenSpanKey(nx, nz, CurrentSpanIdx);
}

void RVoxelizer::AddEdge(const OpenSpanKey& Key, int DirectionIdx, int RegionId, bool bMendatoryPoint)
{
	//RLog("Edge found! loc: (%d, %d), dir: %d\n", Key.x, Key.z, DirectionIdx);

	assert(RegionId >= 0 && RegionId < DebugRegionEdgePoints.size());

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
	DebugRegionEdgePoints[RegionId].push_back({ EdgePoint, bMendatoryPoint });
}

RAabb RVoxelizer::CalculateBoundsForSpan(const HeightfieldSolidSpan& Span, int x, int z) const
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

bool RVoxelizer::IsValidNeighbourSpan(const HeightfieldOpenSpan& ThisOpenSpan, const HeightfieldOpenSpan& NeighbourOpenSpan) const
{
	return abs(NeighbourOpenSpan.CellRowStart - ThisOpenSpan.CellRowStart) <= MaxStepNumCells &&
		   (NeighbourOpenSpan.CellRowEnd - ThisOpenSpan.CellRowStart) > MinTraversableNumCells;
}

HeightfieldOpenSpan& RVoxelizer::GetOpenSpanByKey(const OpenSpanKey& Key)
{
	assert(Key.x >= 0 && Key.x < CellNumX && Key.z >= 0 && Key.z < CellNumZ);
	int Index = Key.x * CellNumZ + Key.z;

	auto& OpenSpans = Heightfield[Index].OpenSpans;
	assert(Key.span_idx >= 0 && Key.span_idx < OpenSpans.size());

	return OpenSpans[Key.span_idx];
}

const HeightfieldOpenSpan& RVoxelizer::GetOpenSpanByKey(const OpenSpanKey& Key) const
{
	return const_cast<RVoxelizer*>(this)->GetOpenSpanByKey(Key);
}

OpenSpanKey RVoxelizer::GetNeighbourSpanByIndex(const OpenSpanKey& Key, int OffsetIndex) const
{
	auto& Span = GetOpenSpanByKey(Key);

	assert(OffsetIndex >= 0 && OffsetIndex < NUM_NEIGHBOUR_SPANS);
	int NeighbourSpanIdx = Span.NeighbourLink[OffsetIndex];
	if (NeighbourSpanIdx != -1)
	{
		return OpenSpanKey(
			Key.x + NeighbourOffset[OffsetIndex].x,
			Key.z + NeighbourOffset[OffsetIndex].z,
			NeighbourSpanIdx
		);
	}

	return OpenSpanKey::Invalid;
}

RVec3 RVoxelizer::GetCellCenter(int x, int y, int z) const
{
	assert(x >= 0 && x < CellNumX && y >= 0 && y < CellNumY && z >= 0 && z < CellNumZ);
	return RVec3(
		SceneCenterPoint.X() + (-0.5f * (CellNumX - 1) + x) * CellDimension.X(),
		SceneCenterPoint.Y() + (-0.5f * (CellNumY - 1) + y) * CellDimension.Y(),
		SceneCenterPoint.Z() + (-0.5f * (CellNumZ - 1) + z) * CellDimension.Z()
	);
}
