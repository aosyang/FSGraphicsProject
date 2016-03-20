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

	inline void Expand(const RAabb& aabb)
	{
		Expand(aabb.pMin);
		Expand(aabb.pMax);
	}

	static RAabb Default;
};

#endif
