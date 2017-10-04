//=============================================================================
// RAabb.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#ifndef _RAABB_H
#define _RAABB_H

#include "RVector.h"

class RAabb
{
public:
	RVec3 pMin;
	RVec3 pMax;

	RAabb();
	inline void Expand(const RVec3& p)
	{
		if (p.x < pMin.x) pMin.x = p.x;
		if (p.y < pMin.y) pMin.y = p.y;
		if (p.z < pMin.z) pMin.z = p.z;
		if (p.x > pMax.x) pMax.x = p.x;
		if (p.y > pMax.y) pMax.y = p.y;
		if (p.z > pMax.z) pMax.z = p.z;
	}

	RAabb& operator=(const RAabb& rhs)
	{
		if (this != &rhs)
		{
			this->pMin = rhs.pMin;
			this->pMax = rhs.pMax;
		}

		return *this;
	}

	inline void Expand(const RAabb& aabb)
	{
		Expand(aabb.pMin);
		Expand(aabb.pMax);
	}

	inline void ExpandBySphere(const RVec3& center, float radius)
	{
		if (center.x - radius < pMin.x) pMin.x = center.x - radius;
		if (center.y - radius < pMin.y) pMin.y = center.y - radius;
		if (center.z - radius < pMin.z) pMin.z = center.z - radius;
		if (center.x + radius > pMax.x) pMax.x = center.x + radius;
		if (center.y + radius > pMax.y) pMax.y = center.y + radius;
		if (center.z + radius > pMax.z) pMax.z = center.z + radius;
	}

	bool IsValid() const;

	RAabb GetTransformedAabb(const RMatrix4& m) const;
	RAabb GetSweptAabb(const RVec3& moveVec) const;

	bool TestPointInsideAabb(const RVec3& point) const;
	bool TestIntersectionWithAabb(const RAabb& aabb) const;

	// Check movement collision with another aabb.
	// Returns corrected movement vector result.
	RVec3 TestDynamicCollisionWithAabb(const RVec3& moveVec, const RAabb& aabb) const;

	static RAabb Default;
};

#endif
