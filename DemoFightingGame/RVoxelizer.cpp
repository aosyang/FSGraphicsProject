//=============================================================================
// RVoxelizer.cpp by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RVoxelizer.h"
#include "Core/RAabb.h"
#include "RenderSystem/RDebugRenderer.h"

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
	: CellDimension(50.0f, 20.0f, 50.0f)
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

		// Generate neighbour data after we have open spans for all columns 
		GenerateOpenSpanNeighbourData();

		GenerateDistanceField();
	}
}

void RVoxelizer::Render()
{
	GDebugRenderer.DrawAabb(SceneBounds);

	static float DebugDistanceField = MaxDistanceField;
	DebugDistanceField -= 0.1f;
	if (DebugDistanceField < 0.0f)
	{
		DebugDistanceField = MaxDistanceField;
	}

	for (const auto& Column : Heightfield)
	{
		// Draw solid spans
		//for (const auto& Span : Column.SolidSpans)
		//{
		//	GDebugRenderer.DrawAabb(Span.Bounds, Span.bTraversable ? RColor::Green : RColor::Red);
		//}

		// Draw traversable areas
		for (const auto& OpenSpan : Column.OpenSpans)
		{
			if (OpenSpan.DistanceField == ceilf(DebugDistanceField))
			{
				GDebugRenderer.DrawSphere(GetCellCenter(Column.x, OpenSpan.CellRowStart, Column.z), 10.0f, RColor::Green, 3);
			}

			if (OpenSpan.bBorder)
			{
				// Draw borders
				//{
				//	RVec3 Start = GetCellCenter(Column.x, OpenSpan.CellRowStart, Column.z);
				//	RVec3 End = GetCellCenter(Column.x, RMath::Min(OpenSpan.CellRowEnd, CellNumY - 1), Column.z);
				//	RColor Color(0.5, 0.5, 1.0f);

				//	GDebugRenderer.DrawSphere(Start, 10.0f, Color, 3);
				//	GDebugRenderer.DrawSphere(End, 10.0f, Color, 3);
				//	GDebugRenderer.DrawLine(Start, End, Color);
				//}

				continue;
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
						if (NeighbourOpenSpan.bBorder)
						{
							continue;
						}

						int NeighbourStart = NeighbourOpenSpan.CellRowStart;

						RVec3 Start = GetCellCenter(Column.x, OpenSpan.CellRowStart, Column.z);
						RVec3 End = GetCellCenter(n_x, NeighbourStart, n_z);

						GDebugRenderer.DrawLine(Start, End, RColor(1.0f, 0.5f, 0.5f));
					}
				}
			}
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

				RAabb CellBound;
				CellBound.pMax = CellCenter + CellDimension / 2.0f;
				CellBound.pMin = CellCenter - CellDimension / 2.0f;

				bool bIsSolidCell = false;

				// Has any overlaps with scene meshes?
				for (auto& SceneObj : SceneObjects)
				{
					if (SceneObj->GetAabb().TestIntersectionWithAabb(CellBound))
					{
						bIsSolidCell = true;
						break;
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

template<class T>
inline void HashCombine(size_t& HashValue, const T& Element)
{
	hash<T> Hash;
	HashValue ^= Hash(Element) + 0x9e3779b9 + (HashValue << 6) + (HashValue >> 2);
}

void RVoxelizer::GenerateDistanceField()
{
	class OpenSpanKey
	{
	public:
		OpenSpanKey(int InX, int InZ, int InSpanIndex)
			: x(InX), z(InZ), span_idx(InSpanIndex)
		{
			HashValue = CalcHash();
		}

		bool operator<(const OpenSpanKey& Rhs) const
		{
			return HashValue < Rhs.HashValue;
		}

		int x, z, span_idx;

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

	// TODO: preallocate data with total open span count
	queue<OpenSpanKey> PendingSpans;

	// The result of each span and its distance field
	map<OpenSpanKey, float> SpanDistanceMap;

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
					SpanDistanceMap[NewKey] = 0.0f;
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
				float NewDistance = SpanDistanceMap.find(ThisSpanKey)->second + 1.0f;

				auto Iter = SpanDistanceMap.find(NewKey);

				// If this neighbour is a new span, added it to the queue
				if (Iter == SpanDistanceMap.end())
				{
					PendingSpans.emplace(NewKey);
					SpanDistanceMap[NewKey] = NewDistance;
				}
				else
				{
					float OldDistance = SpanDistanceMap[NewKey];
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

	MaxDistanceField = 0.0f;

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

RVec3 RVoxelizer::GetCellCenter(int x, int y, int z)
{
	assert(x >= 0 && x < CellNumX && y >= 0 && y < CellNumY && z >= 0 && z < CellNumZ);
	return RVec3(
		SceneCenterPoint.X() + (-0.5f * (CellNumX - 1) + x) * CellDimension.X(),
		SceneCenterPoint.Y() + (-0.5f * (CellNumY - 1) + y) * CellDimension.Y(),
		SceneCenterPoint.Z() + (-0.5f * (CellNumZ - 1) + z) * CellDimension.Z()
	);
}
