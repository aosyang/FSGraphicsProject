//=============================================================================
// RRegionData.h by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "RVoxelizerDataType.h"

#include <map>
#include <vector>

class RNavMeshGenerator;

// Type for mapping an open span to its region id
typedef std::map<OpenSpanKey, int> RegionMapType;

class RRegionData
{
public:
	RRegionData(RNavMeshGenerator* InNavMeshGenerator);

	// Get a region id for a span key. Create a default region id for the key if the id doesn't exist
	int FindOrAddRegionId(const OpenSpanKey& Key);

	// Assign an open span to a region
	void SetRegionId(const OpenSpanKey& Key, int RegionId);

	// Find region id for a span from its adjacent spans
	bool SetRegionIdFromAdjacency(const OpenSpanKey& Key, const std::vector<OpenSpanKey>* IgnoredNeighbors = nullptr);

	// Get a reference of region map
	RegionMapType& GetRegionMapRef();

private:
	RNavMeshGenerator* NavMeshGenerator;

	RegionMapType RegionMap;
};
