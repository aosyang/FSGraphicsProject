//=============================================================================
// RCollision.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "../Core/RVector.h"

struct RPlane
{
	RVec3 normal;
	float offset;

	RPlane() {}
	RPlane(const RVec3& va, const RVec3& vb, const RVec3& vc);
};
