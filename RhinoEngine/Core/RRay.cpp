//=============================================================================
// RRay.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RRay.h"

RRay::RRay()
{
}

RRay::RRay(const RVec3& _origin, const RVec3& _dir, float _dist)
	: Origin(_origin), Direction(_dir.GetNormalizedVec3()), Distance(_dist)
{
}

RRay::RRay(const RVec3& _start, const RVec3& _end)
	: Origin(_start), Direction((_end - _start).GetNormalizedVec3()), Distance((_end - _start).Magnitude())
{
}

RRay RRay::Transform(const RMatrix4& mat) const
{
	return RRay((RVec4(Origin, 1.0f) * mat).ToVec3(), (RVec4(Direction, 0.0f) * mat).ToVec3(), Distance);
}

bool RRay::TestAabbIntersection(const RAabb& aabb) const
{
	if (!aabb.IsValid())
		return false;

	float tmin = -FLT_MAX, tmax = FLT_MAX;

	if (!FLT_EQUAL_ZERO(Direction.x))
	{
		float tx1 = (aabb.pMin.x - Origin.x) / Direction.x;
		float tx2 = (aabb.pMax.x - Origin.x) / Direction.x;

		tmin = max(tmin, min(tx1, tx2));
		tmax = min(tmax, max(tx1, tx2));
	}

	if (!FLT_EQUAL_ZERO(Direction.y))
	{
		float ty1 = (aabb.pMin.y - Origin.y) / Direction.y;
		float ty2 = (aabb.pMax.y - Origin.y) / Direction.y;

		tmin = max(tmin, min(ty1, ty2));
		tmax = min(tmax, max(ty1, ty2));
	}

	if (!FLT_EQUAL_ZERO(Direction.z))
	{
		float tz1 = (aabb.pMin.z - Origin.z) / Direction.z;
		float tz2 = (aabb.pMax.z - Origin.z) / Direction.z;

		tmin = max(tmin, min(tz1, tz2));
		tmax = min(tmax, max(tz1, tz2));
	}

	return tmax > tmin;
}
