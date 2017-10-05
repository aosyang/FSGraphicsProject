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

/// Represents all eight corners of frustum
enum FrustumCorner
{
	FC_FTL,			// far top left
	FC_FTR,			// far top right
	FC_FBL,			// far bottom left
	FC_FBR,			// far bottom right
	FC_NTL,			// near top left
	FC_NTR,			// near top right
	FC_NBL,			// near bottom left
	FC_NBR,			// near bottom right
};

struct RFrustum
{
	RPlane planes[6];
	RVec3 corners[8];

	void BuildPlanesFromCorners();
};

/// Result of testing geometry against a plane
enum class PlaneSpace
{
	Front,
	Back,
	Intersecting,
};

namespace RCollision
{
	PlaneSpace TestSphereToPlane(const RPlane& plane, const RSphere& sphere);
	PlaneSpace TestAabbToPlane(const RPlane& plane, const RAabb& aabb);
	bool TestAabbInsideFrustum(const RFrustum& frustum, const RAabb& aabb);
	bool TestSphereWithCapsule(const RSphere& sphere, const RCapsule& capsule);
};

#endif
