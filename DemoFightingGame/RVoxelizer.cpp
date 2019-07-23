//=============================================================================
// RVoxelizer.cpp by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RVoxelizer.h"
#include "Core/RAabb.h"
#include "RenderSystem/RDebugRenderer.h"

RVoxelizer::RVoxelizer()
	: CellDimension(100.0f, 50.0f, 100.0f)
	, MinTraversableHeight(200.0f)
{

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

	RVec3 SceneCenterPoint = SceneBounds.GetCenter();

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

				HeightfieldSpan Span;
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
							Column.Spans.push_back(Span);
						}
					}

					bIsLastCellSolid = bIsSolidCell;
				}

				// Finish the last span
				if (bIsLastCellSolid)
				{
					Span.CellRowEnd = CellNumY - 1;
					Column.Spans.push_back(Span);
				}

				// Evaluate traversable flag for each span
				for (size_t i = 0; i < Column.Spans.size(); i++)
				{
					if (i < Column.Spans.size() - 1)
					{
						// Measure empty spaces between spans for traversable
						HeightfieldSpan& ThisSpan = Column.Spans[i];
						HeightfieldSpan& NextSpan = Column.Spans[i + 1];
						int NumEmptyCells = NextSpan.CellRowStart - ThisSpan.CellRowEnd - 1;
						ThisSpan.bTraversable = (CellDimension.Y() * NumEmptyCells >= MinTraversableHeight);
					}
					else
					{
						// Check if top spans are traversable by their distance to the up boundary
						HeightfieldSpan& ThisSpan = Column.Spans[i];
						int NumEmptyCells = CellNumY - ThisSpan.CellRowEnd - 1;
						ThisSpan.bTraversable = (CellDimension.Y() * NumEmptyCells >= MinTraversableHeight);
					}
				}

				// Create bounds for each span
				for (auto& IterSpan : Column.Spans)
				{
					IterSpan.Bounds = CreateBoundsForSpan(IterSpan, x, z);
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
		for (const auto& Span : Column.Spans)
		{
			GDebugRenderer.DrawAabb(Span.Bounds, Span.bTraversable ? RColor::Green : RColor::Red);
		}
	}
}

RAabb RVoxelizer::CreateBoundsForSpan(const HeightfieldSpan& Span, int x, int z)
{
	RAabb Result;
	RVec3 SceneCenterPoint = SceneBounds.GetCenter();
	float Scale = 1.0f;

	int y = Span.CellRowStart;
	RVec3 CellCenter = RVec3(
		SceneCenterPoint.X() + (-0.5f * (CellNumX - 1) + x) * CellDimension.X(),
		SceneCenterPoint.Y() + (-0.5f * (CellNumY - 1) + y) * CellDimension.Y(),
		SceneCenterPoint.Z() + (-0.5f * (CellNumZ - 1) + z) * CellDimension.Z()
	);
	Result.pMin = CellCenter - CellDimension * 0.4f * Scale;

	y = Span.CellRowEnd;
	CellCenter = RVec3(
		SceneCenterPoint.X() + (-0.5f * (CellNumX - 1) + x) * CellDimension.X(),
		SceneCenterPoint.Y() + (-0.5f * (CellNumY - 1) + y) * CellDimension.Y(),
		SceneCenterPoint.Z() + (-0.5f * (CellNumZ - 1) + z) * CellDimension.Z()
	);

	Result.pMax = CellCenter + CellDimension * 0.4f * Scale;
	return Result;
}
