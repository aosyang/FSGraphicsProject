//=============================================================================
// RRegionData.cpp by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#include "RRegionData.h"

#include "Core/StdHelper.h"
#include "RNavMeshGenerator.h"

RRegionData::RRegionData(RNavMeshGenerator* InVoxelizer)
	: NavMeshGenerator(InVoxelizer)
{
}

int RRegionData::FindOrAddRegionId(const OpenSpanKey& Key)
{
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
}

void RRegionData::SetRegionId(const OpenSpanKey& Key, int RegionId)
{
	RegionMap[Key] = RegionId;
}

bool RRegionData::SetRegionIdFromAdjacency(const OpenSpanKey& Key, const std::vector<OpenSpanKey>* IgnoredNeighbors /*= nullptr*/)
{
	auto& Span = NavMeshGenerator->GetOpenSpanByKey(Key);
	int DiagonalRegionId = -1;
	for (int NeighborIdx = 0; NeighborIdx < 4; NeighborIdx++)
	{
		int NeighborSpanIndex = Span.NeighborLink[NeighborIdx];
		if (NeighborSpanIndex == -1)
		{
			continue;
		}

		int nx = Key.x + RNavMeshGenerator::NeighborOffset[NeighborIdx].x;
		int nz = Key.z + RNavMeshGenerator::NeighborOffset[NeighborIdx].z;

		OpenSpanKey NeighborKey(nx, nz, NeighborSpanIndex);
		if (IgnoredNeighbors && StdContains(*IgnoredNeighbors, NeighborKey))
		{
			continue;
		}

		int NeighborRegionId = FindOrAddRegionId(NeighborKey);
		if (NeighborRegionId != -1)
		{
			SetRegionId(Key, NeighborRegionId);
			return true;
		}

		if (DiagonalRegionId == -1)
		{
			// Diagonal searching pattern:
			// (x: current span; n: neighbor span; o: diagonal span)
			//
			// o n o         o   o
			//   x     and   n x n
			// o n o         o   o
			// 

			int OffsetIdx[2];
			if (NeighborIdx % 2 == 0)
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
				OpenSpanKey DiagonalNeighborKey = NavMeshGenerator->GetNeighborSpanByIndex(NeighborKey, OffsetIdx[i]);

				if (IgnoredNeighbors && StdContains(*IgnoredNeighbors, DiagonalNeighborKey))
				{
					continue;
				}

				if (DiagonalNeighborKey.IsValid())
				{
					DiagonalRegionId = FindOrAddRegionId(DiagonalNeighborKey);
					break;
				}
			}
		}
	}

	// Diagonal neighbors
	if (DiagonalRegionId != -1)
	{
		SetRegionId(Key, DiagonalRegionId);
		return true;
	}

	return false;
}

RegionMapType& RRegionData::GetRegionMapRef()
{
	return RegionMap;
}
