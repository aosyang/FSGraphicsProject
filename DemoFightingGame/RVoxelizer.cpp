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

	//// Create voxelized cells
	//{
	//	Cells.resize(CellNumX * CellNumY * CellNumZ);

	//	for (int x = 0; x < CellNumX; x++)
	//	{
	//		for (int z = 0; z < CellNumZ; z++)
	//		{
	//			for (int y = 0; y < CellNumY; y++)
	//			{
	//				int Index = x * CellNumY * CellNumZ + z * CellNumY + y;
	//				auto& Cell = Cells[Index];

	//				Cell.Center = RVec3(
	//					SceneCenterPoint.X() + (-0.5f * (CellNumX - 1) + x) * CellDimension.X(),
	//					SceneCenterPoint.Y() + (-0.5f * (CellNumY - 1) + y) * CellDimension.Y(),
	//					SceneCenterPoint.Z() + (-0.5f * (CellNumZ - 1) + z) * CellDimension.Z()
	//				);

	//				RAabb CellBound;
	//				CellBound.pMax = Cell.Center + CellDimension / 2.0f;
	//				CellBound.pMin = Cell.Center - CellDimension / 2.0f;

	//				Cell.bIsSolid = false;

	//				// Overlap checking with scene meshes
	//				for (auto& SceneObj : SceneObjects)
	//				{
	//					if (SceneObj->GetAabb().TestIntersectionWithAabb(CellBound))
	//					{
	//						Cell.bIsSolid = true;
	//						break;
	//					}
	//				}
	//			}
	//		}
	//	}
	//}

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

				// Ignore cells before first solid span
				bool bFoundSolid = false;

				HeightfieldSpan Span;
				ESpanType CurrentSpanType = ESpanType::Solid;

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

					bool bIsSolid = false;

					// Has any overlaps with scene meshes?
					for (auto& SceneObj : SceneObjects)
					{
						if (SceneObj->GetAabb().TestIntersectionWithAabb(CellBound))
						{
							bIsSolid = true;
							break;
						}
					}

					if (bIsSolid)
					{
						// First solid span?
						if (!bFoundSolid)
						{
							bFoundSolid = true;

							Span.CellRowStart = y;
							Span.Type = ESpanType::Solid;
						}
						else
						{
							// A new solid spawn. Finish last open span
							if (Span.Type == ESpanType::Open)
							{
								// An open span is never considered traversable
								Span.bTraversable = false;
								Span.CellRowEnd = y;
								Column.Spans.push_back(Span);

								// Start a new solid span
								Span.Type = ESpanType::Solid;
								Span.CellRowStart = y;
							}
						}
					}
					else // !bIsSolid
					{
						if (Span.Type == ESpanType::Solid)
						{
							// TODO: Detect whether solid surface is traversable by check slopes
							Span.bTraversable = true;	// True for now
							Span.CellRowEnd = y - 1;
							Column.Spans.push_back(Span);

							// Start a new open span
							Span.Type = ESpanType::Open;
							Span.CellRowStart = y - 1;
						}
					}
				}

				// Has started any spans?
				if (bFoundSolid)
				{
					Span.CellRowEnd = CellNumY - 1;
					Column.Spans.push_back(Span);
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
	//for (const auto& Cell : Cells)
	//{
	//	if (Cell.bIsSolid)
	//	{
	//		RAabb Bound;
	//		Bound.pMax = Cell.Center + CellDimension / 2.0f;
	//		Bound.pMin = Cell.Center - CellDimension / 2.0f;

	//		//Bound.pMax = Cell.Center + RVec3(10.0f, 10.0f, 10.0f);
	//		//Bound.pMin = Cell.Center - RVec3(10.0f, 10.0f, 10.0f);

	//		GDebugRenderer.DrawAabb(Bound);

	//		//GDebugRenderer.DrawSphere(Cell.Center, 5.0f, 6);
	//	}
	//}

	for (const auto& Column : Heightfield)
	{
		for (const auto& Span : Column.Spans)
		{
			if (Span.Type == ESpanType::Solid)
			{
				GDebugRenderer.DrawAabb(Span.Bounds, RColor::Red);
			}
			//else
			//{
			//	GDebugRenderer.DrawAabb(Span.Bounds, RColor::Cyan);
			//}
		}
	}
}

RAabb RVoxelizer::CreateBoundsForSpan(const HeightfieldSpan& Span, int x, int z)
{
	RAabb Result;
	RVec3 SceneCenterPoint = SceneBounds.GetCenter();
	float Scale = (Span.Type == ESpanType::Solid) ? 1.0f : 0.9f;

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
