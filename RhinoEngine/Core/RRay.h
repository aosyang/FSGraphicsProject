//=============================================================================
// RRay.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "RVector.h"
#include "RAabb.h"
#include "Collision/RCollision.h"

struct RPlane;

class RRay
{
public:
	RVec3 Origin;
	RVec3 Direction;		// Normalized direction of the ray
	float Distance;

	RRay();
	RRay(const RVec3& _origin, const RVec3& _dir, float _dist);
	RRay(const RVec3& _start, const RVec3& _end);

	RRay Transform(const RMatrix4& mat) const;

	RVec3 GetPointAtDistance(float PointDistance) const;

	// Test for ray intersection with an AABB.
	//		t: If intersects, fill the value with the time of intersection
	bool TestIntersectionWithAabb(const RAabb& aabb, float* t = nullptr) const;

	// Test for ray intersection with a plane
	bool TestIntersectionWithPlane(const RPlane& Plane, float* t = nullptr) const;
};

