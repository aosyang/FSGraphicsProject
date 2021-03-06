//=============================================================================
// RNavMeshCellDetector.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Core/RAabb.h"
#include "Core/CoreTypes.h"

class RSceneObject;

class INavMeshCellDetector
{
public:
	virtual ~INavMeshCellDetector() {}

	/// Test if space of the cell is overlapping with scene geometries
	virtual bool IsCellOccupied(const RAabb& CellBounds) const { return false; }

	/// Test if a cell is traversable
	virtual bool IsCellTraversable(const RAabb& CellBounds) const { return false; }
};

/// The default cell detector
class RDefaultNavMeshCellDetector : public INavMeshCellDetector
{
public:
	RDefaultNavMeshCellDetector(int InNumSubdivides = 4);

	virtual bool IsCellOccupied(const RAabb& CellBounds) const override;
	virtual bool IsCellTraversable(const RAabb& CellBounds) const override;

private:
	std::vector<RSceneObject*> SceneObjects;
	int NumSubdivides;
};

/// A cell detector using physics system
class RPhysicsNavMeshCellDetector : public RDefaultNavMeshCellDetector
{
public:
	RPhysicsNavMeshCellDetector();

	virtual bool IsCellOccupied(const RAabb& CellBounds) const override;
	virtual bool IsCellTraversable(const RAabb& CellBounds) const override;

private:
	std::unique_ptr<class btPairCachingGhostObject> GhostObject;
	mutable std::unique_ptr<class btBoxShape> BoxShape;
};
