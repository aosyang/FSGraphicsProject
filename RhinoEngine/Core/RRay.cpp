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
	: Origin(_origin), Direction(_dir.GetNormalized()), Distance(_dist)
{
}

RRay::RRay(const RVec3& _start, const RVec3& _end)
	: Origin(_start), Direction((_end - _start).GetNormalized()), Distance((_end - _start).Magnitude())
{
}

RRay RRay::Transform(const RMatrix4& mat) const
{
	return RRay((RVec4(Origin, 1.0f) * mat).ToVec3(), (RVec4(Direction, 0.0f) * mat).ToVec3(), Distance);
}

bool RRay::TestAabbIntersection(const RAabb& aabb, float* t/*=nullptr*/) const
{
	if (!aabb.IsValid())
		return false;

	float tmin = -FLT_MAX, tmax = FLT_MAX;

	if (!FLT_EQUAL_ZERO(Direction.X()))
	{
		float tx1 = (aabb.pMin.X() - Origin.X()) / Direction.X();
		float tx2 = (aabb.pMax.X() - Origin.X()) / Direction.X();

		tmin = max(tmin, min(tx1, tx2));
		tmax = min(tmax, max(tx1, tx2));
	}

	if (!FLT_EQUAL_ZERO(Direction.Y()))
	{
		float ty1 = (aabb.pMin.Y() - Origin.Y()) / Direction.Y();
		float ty2 = (aabb.pMax.Y() - Origin.Y()) / Direction.Y();

		tmin = max(tmin, min(ty1, ty2));
		tmax = min(tmax, max(ty1, ty2));
	}

	if (!FLT_EQUAL_ZERO(Direction.Z()))
	{
		float tz1 = (aabb.pMin.Z() - Origin.Z()) / Direction.Z();
		float tz2 = (aabb.pMax.Z() - Origin.Z()) / Direction.Z();

		tmin = max(tmin, min(tz1, tz2));
		tmax = min(tmax, max(tz1, tz2));
	}

	if (tmax > tmin)
	{
		if (t)
			*t = tmin;
		return true;
	}

	return false;
}
