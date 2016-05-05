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

bool RAabb::IsValid() const
{
	return pMax.x >= pMin.x && pMax.y >= pMin.y && pMax.z >= pMin.z;
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
	if (pMax.x <= point.x || pMin.x >= point.x)
		return false;
	if (pMax.y <= point.y || pMin.y >= point.y)
		return false;
	if (pMax.z <= point.z || pMin.z >= point.z)
		return false;

	return true;
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
	static const float tolerance = 0.001f;

	if (!GetSweptAabb(moveVec).TestIntersectionWithAabb(aabb))
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

#define FLT_SGN(x) (x > 0.0f) ? 1.0f : ((x < 0.0f) ? -1.0f : 0.0f);

	RVec3 newVec = moveVec;
	if (tx >= 0 && tx >= ty && tx >= tz)
	{
		float s = FLT_SGN(newVec.x);
		newVec.x = s * max(fabs(newVec.x * tx) - tolerance, 0.0f);
		return newVec;
	}
	else if (ty >= 0 && ty >= tx && ty >= tz)
	{
		float s = FLT_SGN(newVec.y);
		newVec.y = s * max(fabs(newVec.y * ty) - tolerance, 0.0f);
		return newVec;
	}
	else if (tz >= 0 && tz >= tx && tz >= ty)
	{
		float s = FLT_SGN(newVec.z);
		newVec.z = s * max(fabs(newVec.z * tz) - tolerance, 0.0f);
		return newVec;
	}

	return -moveVec;
}