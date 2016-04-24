//=============================================================================
// RCollision.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RCollision.h"

RPlane::RPlane(const RVec3& va, const RVec3& vb, const RVec3& vc)
{
	normal = (vb - va).Cross(vc - va);
	normal.Normalize();
	offset = normal.Dot(va);
}

namespace RCollision
{
	PlaneSpace TestSphereToPlane(const RPlane& plane, const RSphere& sphere)
	{
		float dist = sphere.center.Dot(plane.normal) - plane.offset;
		if (dist > sphere.radius)
			return PlaneSpace_Front;
		else if (dist < sphere.radius)
			return PlaneSpace_Back;
		else
			return PlaneSpace_Intersecting;
	}

	PlaneSpace TestAabbToPlane(const RPlane& plane, const RAabb& aabb)
	{
		RSphere s;
		s.center = (aabb.pMax + aabb.pMin) * 0.5f;
		RVec3 e = aabb.pMax - s.center;
		s.radius = e.x * fabs(plane.normal.x) + e.y * fabs(plane.normal.y) + e.z * fabs(plane.normal.z);

		return TestSphereToPlane(plane, s);
	}

	bool TestAabbInsideFrustum(const RFrustum& frustum, const RAabb& aabb)
	{
		for (int i = 0; i < 6; i++)
		{
			if (TestAabbToPlane(frustum.planes[i], aabb) == PlaneSpace_Back)
				return false;
		}

		return true;
	}
};
