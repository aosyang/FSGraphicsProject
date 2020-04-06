//=============================================================================
// RRay.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

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

RVec3 RRay::GetPointAtDistance(float PointDistance) const
{
	return Origin + Direction * PointDistance;
}

bool RRay::TestIntersectionWithAabb(const RAabb& aabb, float* t/*=nullptr*/) const
{
	if (!aabb.IsValid())
		return false;

	float tmin = -FLT_MAX, tmax = FLT_MAX;

	if (!FLT_EQUAL_ZERO(Direction.X()))
	{
		float tx1 = (aabb.pMin.X() - Origin.X()) / Direction.X();
		float tx2 = (aabb.pMax.X() - Origin.X()) / Direction.X();

		tmin = RMath::Max(tmin, RMath::Min(tx1, tx2));
		tmax = RMath::Min(tmax, RMath::Max(tx1, tx2));
	}

	if (!FLT_EQUAL_ZERO(Direction.Y()))
	{
		float ty1 = (aabb.pMin.Y() - Origin.Y()) / Direction.Y();
		float ty2 = (aabb.pMax.Y() - Origin.Y()) / Direction.Y();

		tmin = RMath::Max(tmin, RMath::Min(ty1, ty2));
		tmax = RMath::Min(tmax, RMath::Max(ty1, ty2));
	}

	if (!FLT_EQUAL_ZERO(Direction.Z()))
	{
		float tz1 = (aabb.pMin.Z() - Origin.Z()) / Direction.Z();
		float tz2 = (aabb.pMax.Z() - Origin.Z()) / Direction.Z();

		tmin = RMath::Max(tmin, RMath::Min(tz1, tz2));
		tmax = RMath::Min(tmax, RMath::Max(tz1, tz2));
	}

	if (tmax > tmin)
	{
		if (t)
			*t = tmin;
		return true;
	}

	return false;
}

bool RRay::TestIntersectionWithPlane(const RPlane& Plane, float* t /*= nullptr*/) const
{
	float d = RVec3::Dot(Plane.normal, Direction);

	// Note: in the case of checking (d > 0) or (d < 0), we only test intersection from one side of the plane
	if (fabs(d) > 0.0f)
	{
		RVec3 p0 = Plane.normal * Plane.offset;
		RVec3 p0l0 = p0 - Origin;
		float r = RVec3::Dot(p0l0, Plane.normal) / d;
		if (r >= 0.0f)
		{
			if (t)
			{
				*t = r;
			}

			return true;
		}

		return false;
	}

	return false;
}
