//=============================================================================
// RCollision.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#ifndef _RCOLLISION_H
#define _RCOLLISION_H

#include "../Core/RVector.h"

struct RPlane
{
	RVec3 normal;
	float offset;

	RPlane() {}
	RPlane(const RVec3& va, const RVec3& vb, const RVec3& vc);
};

struct RSphere
{
	RVec3 center;
	float radius;
};

struct RCapsule
{
	RVec3 start;
	RVec3 end;
	float radius;
};

enum FrustumCorners
{
	FC_FTL,
	FC_FTR,
	FC_FBL,
	FC_FBR,
	FC_NTL,
	FC_NTR,
	FC_NBL,
	FC_NBR,
};

struct RFrustum
{
	RPlane planes[6];
	RVec3 corners[8];
};

enum PlaneSpace
{
	PlaneSpace_Front,
	PlaneSpace_Back,
	PlaneSpace_Intersecting,
};

namespace RCollision
{
	PlaneSpace TestSphereToPlane(const RPlane& plane, const RSphere& sphere);
	PlaneSpace TestAabbToPlane(const RPlane& plane, const RAabb& aabb);
	bool TestAabbInsideFrustum(const RFrustum& frustum, const RAabb& aabb);
	bool TestSphereWithCapsule(const RSphere& sphere, const RCapsule& capsule);
};

#endif
