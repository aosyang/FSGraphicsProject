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
}


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
				IterSpan.Bounds = CreateBoundsForSpan(IterSpan, x, z);
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
	auto SetRegionIdFromAdjacency = [this, &RegionMap, &GetRegionId, &SetRegionId](const OpenSpanKey& Key) -> bool {
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

				if (NeighbourIdx % 2 == 0)
				{
					OpenSpanKey DiagonalNeighbourKey(-1, -1, -1);
					if (GetNeighbourSpan(NeighbourKey, 1, DiagonalNeighbourKey))
					{
						DiagonalRegionId = GetRegionId(DiagonalNeighbourKey);
					}

					if (DiagonalRegionId == -1)
					{
						if (GetNeighbourSpan(NeighbourKey, 3, DiagonalNeighbourKey))
						{
							DiagonalRegionId = GetRegionId(DiagonalNeighbourKey);
						}
					}
				}
				else
				{
					OpenSpanKey DiagonalNeighbourKey(-1, -1, -1);
					if (GetNeighbourSpan(NeighbourKey, 0, DiagonalNeighbourKey))
					{
						DiagonalRegionId = GetRegionId(DiagonalNeighbourKey);
					}

					if (DiagonalRegionId == -1)
					{
						if (GetNeighbourSpan(NeighbourKey, 2, DiagonalNeighbourKey))
						{
							DiagonalRegionId = GetRegionId(DiagonalNeighbourKey);
						}
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

		// Make isolated spans into new regions
		while (IsolatedSpans.size() > 0)
		{
			int NewRegion = NumRegions++;
			RLog("Region id: %d\n", NewRegion);

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
	
	// Assign final region ids to all spans
	for (auto Iter : RegionMap)
	{
		auto& Span = GetOpenSpanByKey(Iter.first);
		Span.RegionId = Iter.second;
	}
}

RAabb RVoxelizer::CreateBoundsForSpan(const HeightfieldSolidSpan& Span, int x, int z)
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

bool RVoxelizer::GetNeighbourSpan(const OpenSpanKey& Key, int OffsetIndex, OpenSpanKey& OutSpan) const
{
	auto& Span = GetOpenSpanByKey(Key);

	assert(OffsetIndex >= 0 && OffsetIndex < NUM_NEIGHBOUR_SPANS);
	int NeighbourSpanIdx = Span.NeighbourLink[OffsetIndex];
	if (NeighbourSpanIdx != -1)
	{
		OutSpan = OpenSpanKey(
			Key.x + NeighbourOffset[OffsetIndex].x,
			Key.z + NeighbourOffset[OffsetIndex].z,
			NeighbourSpanIdx
		);
		return true;
	}
	return false;
}

RVec3 RVoxelizer::GetCellCenter(int x, int y, int z)
{
	assert(x >= 0 && x < CellNumX && y >= 0 && y < CellNumY && z >= 0 && z < CellNumZ);
	return RVec3(
		SceneCenterPoint.X() + (-0.5f * (CellNumX - 1) + x) * CellDimension.X(),
		SceneCenterPoint.Y() + (-0.5f * (CellNumY - 1) + y) * CellDimension.Y(),
		SceneCenterPoint.Z() + (-0.5f * (CellNumZ - 1) + z) * CellDimension.Z()
	);
}
