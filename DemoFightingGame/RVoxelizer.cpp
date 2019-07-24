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
					RVec3 CellCenter = RVec3(
						SceneCenterPoint.X() + (-0.5f * (CellNumX - 1) + x) * CellDimension.X(),
						SceneCenterPoint.Y() + (-0.5f * (CellNumY - 1) + y) * CellDimension.Y(),
						SceneCenterPoint.Z() + (-0.5f * (CellNumZ - 1) + z) * CellDimension.Z()
					);

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
}

void RVoxelizer::Render()
{
	GDebugRenderer.DrawAabb(SceneBounds);

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
