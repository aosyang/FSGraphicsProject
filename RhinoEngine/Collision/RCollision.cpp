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
		else if (dist < -sphere.radius)
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

	bool TestSphereWithCapsule(const RSphere& sphere, const RCapsule& capsule)
	{
		RVec3 v = capsule.end - capsule.start;
		RVec3 vn = v.GetNormalizedVec3();
		RVec3 pt = sphere.center - capsule.start;
		float scale = pt.Dot(vn) / v.Dot(vn);
		RVec3 cloest_pt;

		if (scale < 0)
			scale = 0;
		else if (scale > 1)
			scale = 1;

		cloest_pt = capsule.start + v * scale;

		float sqrdRadius = sphere.radius + capsule.radius;
		sqrdRadius *= sqrdRadius;

		return (sphere.center - cloest_pt).SquaredMagitude() < sqrdRadius;
	}
};

void RFrustum::BuildPlanesFromCorners()
{
	planes[0] = RPlane(corners[FC_NBR], corners[FC_NTL], corners[FC_NBL]);
	planes[1] = RPlane(corners[FC_FBL], corners[FC_FTR], corners[FC_FBR]);
	planes[2] = RPlane(corners[FC_NBL], corners[FC_FTL], corners[FC_FBL]);
	planes[3] = RPlane(corners[FC_FBR], corners[FC_NTR], corners[FC_NBR]);
	planes[4] = RPlane(corners[FC_NTR], corners[FC_FTL], corners[FC_NTL]);
	planes[5] = RPlane(corners[FC_NBL], corners[FC_FBR], corners[FC_NBR]);
}
