//=============================================================================
// RNavMeshCellDetector.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RNavMeshCellDetector.h"

#include "Scene/RSceneManager.h"
#include "Scene/RSMeshObject.h"

#include "Physics/RPhysicsEngine.h"
#include "Physics/RPhysicsPrivate.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"

RDefaultNavMeshCellDetector::RDefaultNavMeshCellDetector(int InNumSubdivides /*= 4*/)
	: NumSubdivides(InNumSubdivides)
{
	SceneObjects = GSceneManager.DefaultScene()->EnumerateSceneObjects();
}

bool RDefaultNavMeshCellDetector::IsCellOccupied(const RAabb& CellBounds)
{
	for (auto& SceneObj : SceneObjects)
	{
		const RAabb& ObjBounds = SceneObj->GetAabb();
		if (ObjBounds.TestIntersectionWithAabb(CellBounds))
		{
			// Mesh objects may contain per-element bounding box. In that case we're running overlapping test against mesh elements.
			if (RSMeshObject* MeshObj = SceneObj->CastTo<RSMeshObject>())
			{
				for (int i = 0; i < MeshObj->GetMeshElementCount(); i++)
				{
					const RAabb& ElementBounds = MeshObj->GetMeshElementAabb(i);
					const RAabb ElementWorldBounds = ElementBounds.GetTransformedAabb(MeshObj->GetTransformMatrix());

					if (ElementWorldBounds.TestIntersectionWithAabb(CellBounds))
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

bool RDefaultNavMeshCellDetector::IsCellTraversable(const RAabb& CellBounds)
{
	float StepX = (CellBounds.pMax.X() - CellBounds.pMin.X()) / NumSubdivides;
	float StepZ = (CellBounds.pMax.Z() - CellBounds.pMin.Z()) / NumSubdivides;
	float Height = CellBounds.pMax.Y() - CellBounds.pMin.Y();

	for (int z = 0; z < NumSubdivides; z++)
	{
		for (int x = 0; x < NumSubdivides; x++)
		{
			RVec3 Min(
				CellBounds.pMin.X() + x * StepX,
				CellBounds.pMin.Y() - Height,
				CellBounds.pMin.Z() + z * StepZ);

			RVec3 Max(
				CellBounds.pMin.X() + (x + 1) * StepX,
				CellBounds.pMin.Y(),
				CellBounds.pMin.Z() + (z + 1) * StepZ);

			RAabb Bounds(Min, Max);
			if (!IsCellOccupied(Bounds))
			{
				return false;
			}
		}
	}

	return true;
}

struct CellContactResultCallback : public btCollisionWorld::ContactResultCallback
{
	CellContactResultCallback()
		: bHasContacts(false)
	{}

	virtual btScalar addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1) override
	{
		bHasContacts = cp.getDistance() < 0.05f;
		return 1.0f;
	}

	bool bHasContacts;
};

RPhysicsNavMeshCellDetector::RPhysicsNavMeshCellDetector()
	: RDefaultNavMeshCellDetector(8)
	, GhostObject(std::make_unique<btPairCachingGhostObject>())
	, BoxShape(std::make_unique<btBoxShape>(btVector3(1, 1, 1)))
{
	GhostObject->setCollisionShape(BoxShape.get());
}

bool RPhysicsNavMeshCellDetector::IsCellOccupied(const RAabb& CellBounds)
{
	if (GhostObject->getCollisionShape())
	{
		GhostObject->setCollisionShape(nullptr);
	}

	BoxShape = std::make_unique<btBoxShape>(RVec3TobtVec3(CellBounds.GetLocalDimension() / 2));
	GhostObject->setCollisionShape(BoxShape.get());

	btTransform CellTransform;
	CellTransform.setIdentity();
	CellTransform.setOrigin(RVec3TobtVec3(CellBounds.GetCenter()));
	GhostObject->setWorldTransform(CellTransform);

	CellContactResultCallback ContactResult;

	GPhysicsEngine.GetContext()->DynamicWorld->contactTest(GhostObject.get(), ContactResult);
	return ContactResult.bHasContacts;
}

bool RPhysicsNavMeshCellDetector::IsCellTraversable(const RAabb& CellBounds)
{
	return RDefaultNavMeshCellDetector::IsCellTraversable(CellBounds);
}
