//=============================================================================
// RCollision.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "../Core/RVector.h"

/// 3D plane
struct RPlane
{
	RVec3 normal;	// The normalized normal vector of the plane
	float offset;	// Distance from the origin to the plane

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
enum class EPlaneSpace : UINT8
{
	Front,
	Back,
	Intersecting,
};

namespace RCollision
{
	EPlaneSpace TestSphereToPlane(const RPlane& plane, const RSphere& sphere);
	EPlaneSpace TestAabbToPlane(const RPlane& plane, const RAabb& aabb);
	bool TestAabbInsideFrustum(const RFrustum& frustum, const RAabb& aabb);
	bool TestSphereWithCapsule(const RSphere& sphere, const RCapsule& capsule);
};

