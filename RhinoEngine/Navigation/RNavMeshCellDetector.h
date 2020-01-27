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

	/// Test if space of the cell is empty
	virtual bool IsCellVacant(const RAabb& CellBounds) { return false; }

	/// Test if a cell is traversable
	virtual bool IsCellTraversable(const RAabb& CellBounds) { return false; }
};

/// The default cell detector
class RDefaultNavMeshCellDetector : public INavMeshCellDetector
{
public:
	RDefaultNavMeshCellDetector(int InNumSubdivides = 4);

	virtual bool IsCellVacant(const RAabb& CellBounds) override;
	virtual bool IsCellTraversable(const RAabb& CellBounds) override;

private:
	std::vector<RSceneObject*> SceneObjects;
	int NumSubdivides;
};

/// A cell detector using physics system
class RPhysicsNavMeshCellDetector : public INavMeshCellDetector
{
public:
	RPhysicsNavMeshCellDetector();

	virtual bool IsCellVacant(const RAabb& CellBounds) override;
	virtual bool IsCellTraversable(const RAabb& CellBounds) override;

private:
	std::unique_ptr<class btPairCachingGhostObject> GhostObject;
	std::unique_ptr<class btBoxShape> BoxShape;
};
