//=============================================================================
// RRay.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "RVector.h"
#include "RAabb.h"

class RRay
{
public:
	RVec3 Origin;
	RVec3 Direction;
	float Distance;

	RRay();
	RRay(const RVec3& _origin, const RVec3& _dir, float _dist);
	RRay(const RVec3& _start, const RVec3& _end);

	RRay Transform(const RMatrix4& mat) const;
	bool TestAabbIntersection(const RAabb& aabb, float* t = nullptr) const;
};

