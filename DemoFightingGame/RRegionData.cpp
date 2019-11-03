//=============================================================================
// RRegionData.cpp by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#include "RRegionData.h"

#include "Core/StdHelper.h"
#include "RVoxelizer.h"

RRegionData::RRegionData(RVoxelizer* InVoxelizer)
	: Voxelizer(InVoxelizer)
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

bool RRegionData::SetRegionIdFromAdjacency(const OpenSpanKey& Key, const std::vector<OpenSpanKey>* IgnoredNeighbours /*= nullptr*/)
{
	auto& Span = Voxelizer->GetOpenSpanByKey(Key);
	int DiagonalRegionId = -1;
	for (int NeighbourIdx = 0; NeighbourIdx < 4; NeighbourIdx++)
	{
		int NeighbourSpanIndex = Span.NeighbourLink[NeighbourIdx];
		if (NeighbourSpanIndex == -1)
		{
			continue;
		}

		int nx = Key.x + RVoxelizer::NeighbourOffset[NeighbourIdx].x;
		int nz = Key.z + RVoxelizer::NeighbourOffset[NeighbourIdx].z;

		OpenSpanKey NeighbourKey(nx, nz, NeighbourSpanIndex);
		if (IgnoredNeighbours && StdContains(*IgnoredNeighbours, NeighbourKey))
		{
			continue;
		}

		int NeighbourRegionId = FindOrAddRegionId(NeighbourKey);
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
				OpenSpanKey DiagonalNeighbourKey = Voxelizer->GetNeighbourSpanByIndex(NeighbourKey, OffsetIdx[i]);

				if (IgnoredNeighbours && StdContains(*IgnoredNeighbours, DiagonalNeighbourKey))
				{
					continue;
				}

				if (DiagonalNeighbourKey.IsValid())
				{
					DiagonalRegionId = FindOrAddRegionId(DiagonalNeighbourKey);
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
}

RegionMapType& RRegionData::GetRegionMapRef()
{
	return RegionMap;
}
