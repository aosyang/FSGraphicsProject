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
	: pMin(FLT_MAX, FLT_MAX, FLT_MAX)
	, pMax(-FLT_MAX, -FLT_MAX, -FLT_MAX)
{
}

RAabb::RAabb(const RAabb& Other)
	: pMin(Other.pMin)
	, pMax(Other.pMax)
{
}

void RAabb::Reset()
{
	pMin = RVec3(FLT_MAX, FLT_MAX, FLT_MAX);
	pMax = RVec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
}

bool RAabb::IsValid() const
{
	return pMax.X() >= pMin.X() && pMax.Y() >= pMin.Y() && pMax.Z() >= pMin.Z();
}

RAabb RAabb::GetTransformedAabb(const RMatrix4& m) const
{
	RAabb aabb;
	aabb.pMin = aabb.pMax = m.GetTranslation();

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			float e = m.m[j][i] * ((float*)&pMin)[j];
			float f = m.m[j][i] * ((float*)&pMax)[j];
			if (e < f)
			{
				((float*)&aabb.pMin)[i] += e;
				((float*)&aabb.pMax)[i] += f;
			}
			else
			{
				((float*)&aabb.pMin)[i] += f;
				((float*)&aabb.pMax)[i] += e;
			}
		}
	}

	return aabb;
}

RAabb RAabb::GetSweptAabb(const RVec3& moveVec) const
{
	RAabb sweptAabb;
	sweptAabb.pMin = pMin;
	sweptAabb.pMax = pMax;
	sweptAabb.Expand(pMin + moveVec);
	sweptAabb.Expand(pMax + moveVec);

	return sweptAabb;
}

bool RAabb::TestPointInsideAabb(const RVec3& point) const
{
	if (pMax.X() <= point.X() || pMin.X() >= point.X())
		return false;
	if (pMax.Y() <= point.Y() || pMin.Y() >= point.Y())
		return false;
	if (pMax.Z() <= point.Z() || pMin.Z() >= point.Z())
		return false;

	return true;
}

bool RAabb::TestIntersectionWithAabb(const RAabb& aabb) const
{
	if (pMax.X() <= aabb.pMin.X() || pMin.X() >= aabb.pMax.X())
		return false;
	if (pMax.Y() <= aabb.pMin.Y() || pMin.Y() >= aabb.pMax.Y())
		return false;
	if (pMax.Z() <= aabb.pMin.Z() || pMin.Z() >= aabb.pMax.Z())
		return false;

	return true;
}

RVec3 RAabb::TestDynamicCollisionWithAabb(const RVec3& moveVec, const RAabb& aabb) const
{
	static const float tolerance = 0.001f;

	if (!GetSweptAabb(moveVec).TestIntersectionWithAabb(aabb))
		return moveVec;

	float tx = 2, ty = 2, tz = 2;

	// Check initial colliding
	if (pMax.X() > aabb.pMin.X() && pMin.X() < aabb.pMax.X())
	{
		tx = -1;
	}
	else
	{
		if (pMax.X() <= aabb.pMin.X() && pMax.X() + moveVec.X() > aabb.pMin.X())
			tx = (aabb.pMin.X() - pMax.X()) / moveVec.X();
		else if (pMin.X() >= aabb.pMax.X() && pMin.X() + moveVec.X() < aabb.pMax.X())
			tx = (aabb.pMax.X() - pMin.X()) / moveVec.X();
	}

	if (pMax.Y() > aabb.pMin.Y() && pMin.Y() < aabb.pMax.Y())
	{
		ty = -1;
	}
	else
	{
		if (pMax.Y() <= aabb.pMin.Y() && pMax.Y() + moveVec.Y() > aabb.pMin.Y())
			ty = (aabb.pMin.Y() - pMax.Y()) / moveVec.Y();
		else if (pMin.Y() >= aabb.pMax.Y() && pMin.Y() + moveVec.Y() < aabb.pMax.Y())
			ty = (aabb.pMax.Y() - pMin.Y()) / moveVec.Y();
	}

	if (pMax.Z() > aabb.pMin.Z() && pMin.Z() < aabb.pMax.Z())
	{
		tz = -1;
	}
	else
	{
		if (pMax.Z() <= aabb.pMin.Z() && pMax.Z() + moveVec.Z() > aabb.pMin.Z())
			tz = (aabb.pMin.Z() - pMax.Z()) / moveVec.Z();
		else if (pMin.Z() >= aabb.pMax.Z() && pMin.Z() + moveVec.Z() < aabb.pMax.Z())
			tz = (aabb.pMax.Z() - pMin.Z()) / moveVec.Z();
	}

	// No collision on any of three axises, moving vector is good
	if (tx > 1 || ty > 1 || tz > 1)
		return moveVec;

#define FLT_SGN(x) (x > 0.0f) ? 1.0f : ((x < 0.0f) ? -1.0f : 0.0f);

	RVec3 newVec = moveVec;
	if (tx >= 0 && tx >= ty && tx >= tz)
	{
		float s = FLT_SGN(newVec.X());
		newVec.SetX(s * max(fabs(newVec.X() * tx) - tolerance, 0.0f));
		return newVec;
	}
	else if (ty >= 0 && ty >= tx && ty >= tz)
	{
		float s = FLT_SGN(newVec.Y());
		newVec.SetY(s * max(fabs(newVec.Y() * ty) - tolerance, 0.0f));
		return newVec;
	}
	else if (tz >= 0 && tz >= tx && tz >= ty)
	{
		float s = FLT_SGN(newVec.Z());
		newVec.SetZ(s * max(fabs(newVec.Z() * tz) - tolerance, 0.0f));
		return newVec;
	}

	return -moveVec;
}