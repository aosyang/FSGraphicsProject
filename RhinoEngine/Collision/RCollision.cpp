//=============================================================================
// RCollision.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RCollision.h"

RPlane::RPlane(const RVec3& va, const RVec3& vb, const RVec3& vc)
{
	normal = (vb - va).Cross(vc - va);
	normal.Normalize();
	offset = normal.Dot(va);
}