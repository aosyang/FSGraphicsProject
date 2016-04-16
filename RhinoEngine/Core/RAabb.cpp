//=============================================================================
// RAabb.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"
#include "RAabb.h"

#include <float.h>

RAabb RAabb::Default;

RAabb::RAabb()
	:pMin(FLT_MAX, FLT_MAX, FLT_MAX),
	 pMax(-FLT_MAX, -FLT_MAX, -FLT_MAX)
{
}

bool RAabb::TestIntersectionWithAabb(const RAabb& aabb) const
{
	if (pMax.x <= aabb.pMin.x || pMin.x >= aabb.pMax.x)
		return false;
	if (pMax.y <= aabb.pMin.y || pMin.y >= aabb.pMax.y)
		return false;
	if (pMax.z <= aabb.pMin.z || pMin.z >= aabb.pMax.z)
		return false;

	return true;
}

RVec3 RAabb::TestDynamicCollisionWithAabb(const RVec3& moveVec, const RAabb& aabb) const
{
	static const float tolerance = 0.95f;

	RAabb sweptAabb;
	sweptAabb.Expand(pMin);
	sweptAabb.Expand(pMax);
	sweptAabb.Expand(pMin + moveVec);
	sweptAabb.Expand(pMax + moveVec);

	if (!sweptAabb.TestIntersectionWithAabb(aabb))
		return moveVec;

	float tx = 2, ty = 2, tz = 2;

	// Check initial colliding
	if (pMax.x > aabb.pMin.x && pMin.x < aabb.pMax.x)
	{
		tx = -1;
	}
	else
	{
		if (pMax.x <= aabb.pMin.x && pMax.x + moveVec.x > aabb.pMin.x)
			tx = (aabb.pMin.x - pMax.x) / moveVec.x;
		else if (pMin.x >= aabb.pMax.x && pMin.x + moveVec.x < aabb.pMax.x)
			tx = (aabb.pMax.x - pMin.x) / moveVec.x;
	}

	if (pMax.y > aabb.pMin.y && pMin.y < aabb.pMax.y)
	{
		ty = -1;
	}
	else
	{
		if (pMax.y <= aabb.pMin.y && pMax.y + moveVec.y > aabb.pMin.y)
			ty = (aabb.pMin.y - pMax.y) / moveVec.y;
		else if (pMin.y >= aabb.pMax.y && pMin.y + moveVec.y < aabb.pMax.y)
			ty = (aabb.pMax.y - pMin.y) / moveVec.y;
	}

	if (pMax.z > aabb.pMin.z && pMin.z < aabb.pMax.z)
	{
		tz = -1;
	}
	else
	{
		if (pMax.z <= aabb.pMin.z && pMax.z + moveVec.z > aabb.pMin.z)
			tz = (aabb.pMin.z - pMax.z) / moveVec.z;
		else if (pMin.z >= aabb.pMax.z && pMin.z + moveVec.z < aabb.pMax.z)
			tz = (aabb.pMax.z - pMin.z) / moveVec.z;
	}

	// No collision on any of three axises, moving vector is good
	if (tx > 1 || ty > 1 || tz > 1)
		return moveVec;

	RVec3 newVec = moveVec;
	if (tx >= 0 && tx >= ty && tx >= tz)
	{
		newVec.x *= tx * tolerance;
		return newVec;
	}
	else if (ty >= 0 && ty >= tx && ty >= tz)
	{
		newVec.y *= ty * tolerance;
		return newVec;
	}
	else if (tz >= 0 && tz >= tx && tz >= ty)
	{
		newVec.z *= tz * tolerance;
		return newVec;
	}

	return -moveVec;
}