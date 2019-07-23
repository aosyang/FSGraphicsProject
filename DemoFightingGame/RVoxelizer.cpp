//=============================================================================
// RVoxelizer.cpp by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RVoxelizer.h"
#include "Core/RAabb.h"
#include "RenderSystem/RDebugRenderer.h"

RVec3 RVoxelizer::NeighbourOffset[4] =
{
	RVec3(1, 0, 0),
	RVec3(0, 0, 1),
	RVec3(-1, 0, 0),
	RVec3(0, 0, -1),
};


RVoxelizer::RVoxelizer()
	: CellDimension(50.0f, 20.0f, 50.0f)
	, MinTraversableHeight(200.0f)
	, MaxStepHeight(50.0f)
{
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
							Column.OpenSpans.push_back(OpenSpan);
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
					for (int NeighbourLinkIndex = 0; NeighbourLinkIndex < 4; NeighbourLinkIndex++)
					{
						// Neighbour coordinates
						const int n_x = x + (int)NeighbourOffset[NeighbourLinkIndex].X();
						const int n_z = z + (int)NeighbourOffset[NeighbourLinkIndex].Z();

						if (n_x >= 0 && n_x < CellNumX &&
							n_z >= 0 && n_z < CellNumZ)
						{
							int NeighbourIndex = n_x * CellNumZ + n_z;
							int NeighbourOpenSpanIndex = 0;

							// Find linked neighbour open spans
							for (const auto& NeighbourOpenSpan : Heightfield[NeighbourIndex].OpenSpans)
							{
								if (abs(NeighbourOpenSpan.CellRowStart - ThisOpenSpan.CellRowStart) <= MaxStepNumCells &&
									NeighbourOpenSpan.CellRowEnd >= ThisOpenSpan.CellRowEnd)
								{
									ThisOpenSpan.NeighbourLink[NeighbourLinkIndex] = NeighbourOpenSpanIndex;
									break;
								}

								NeighbourOpenSpanIndex++;
							}
						}
					}
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
		for (const auto& Span : Column.SolidSpans)
		{
			GDebugRenderer.DrawAabb(Span.Bounds, Span.bTraversable ? RColor::Green : RColor::Red);
		}

		for (const auto& OpenSpan : Column.OpenSpans)
		{
			for (int i = 0; i < 4; i++)
			{
				int NeighbourLinkIndex = OpenSpan.NeighbourLink[i];
				if (NeighbourLinkIndex != -1)
				{
					const int n_x = Column.x + (int)NeighbourOffset[i].X();
					const int n_z = Column.z + (int)NeighbourOffset[i].Z();

					if (n_x >= 0 && n_x < CellNumX &&
						n_z >= 0 && n_z < CellNumZ)
					{
						int NeighbourIndex = n_x * CellNumZ + n_z;
						int NeighbourStart = Heightfield[NeighbourIndex].OpenSpans[NeighbourLinkIndex].CellRowStart;

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

RVec3 RVoxelizer::GetCellCenter(int x, int y, int z)
{
	return RVec3(
		SceneCenterPoint.X() + (-0.5f * (CellNumX - 1) + x) * CellDimension.X(),
		SceneCenterPoint.Y() + (-0.5f * (CellNumY - 1) + y) * CellDimension.Y(),
		SceneCenterPoint.Z() + (-0.5f * (CellNumZ - 1) + z) * CellDimension.Z()
	);
}
