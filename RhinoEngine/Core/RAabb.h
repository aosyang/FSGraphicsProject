//=============================================================================
// RAabb.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "RVector.h"
#include "RMatrix.h"

// Axis-aligned bounding box
class RAabb
{
public:
	RVec3 pMin;
	RVec3 pMax;

	RAabb();
	RAabb(const RVec3& InMin, const RVec3& InMax);
	RAabb(const RAabb& Other);

	inline void Expand(const RVec3& p)
	{
		if (p.X() < pMin.X()) pMin.SetX(p.X());
		if (p.Y() < pMin.Y()) pMin.SetY(p.Y());
		if (p.Z() < pMin.Z()) pMin.SetZ(p.Z());
		if (p.X() > pMax.X()) pMax.SetX(p.X());
		if (p.Y() > pMax.Y()) pMax.SetY(p.Y());
		if (p.Z() > pMax.Z()) pMax.SetZ(p.Z());
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

	RAabb& operator=(RAabb&& rhs)
	{
		this->pMin = rhs.pMin;
		this->pMax = rhs.pMax;
		rhs.Reset();

		return *this;
	}

	inline void Expand(const RAabb& aabb)
	{
		Expand(aabb.pMin);
		Expand(aabb.pMax);
	}

	inline void ExpandBySphere(const RVec3& center, float radius)
	{
		if (center.X() - radius < pMin.X()) pMin.SetX(center.X() - radius);
		if (center.Y() - radius < pMin.Y()) pMin.SetY(center.Y() - radius);
		if (center.Z() - radius < pMin.Z()) pMin.SetZ(center.Z() - radius);
		if (center.X() + radius > pMax.X()) pMax.SetX(center.X() + radius);
		if (center.Y() + radius > pMax.Y()) pMax.SetY(center.Y() + radius);
		if (center.Z() + radius > pMax.Z()) pMax.SetZ(center.Z() + radius);
	}

	void Reset();

	bool IsValid() const;

	/// Get the center point of bounding box
	RVec3 GetCenter() const;

	/// Get the size of bounding box in local space
	RVec3 GetLocalDimension() const;

	RAabb GetTransformedAabb(const RMatrix4& m) const;
	RAabb GetSweptAabb(const RVec3& moveVec) const;

	bool TestPointInsideAabb(const RVec3& point) const;
	bool TestIntersectionWithAabb(const RAabb& aabb) const;

	// Check movement collision with another aabb.
	// Returns corrected movement vector result.
	RVec3 TestDynamicCollisionWithAabb(const RVec3& moveVec, const RAabb& aabb) const;

	static RAabb Default;
};

FORCEINLINE RVec3 RAabb::GetCenter() const
{
	return (pMax + pMin) / 2.0f;
}

FORCEINLINE RVec3 RAabb::GetLocalDimension() const
{
	return pMax - pMin;
}
