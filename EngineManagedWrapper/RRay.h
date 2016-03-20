//=============================================================================
// RRay.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#ifndef _RRAY_H
#define _RRAY_H

#include "Core\RVector.h"

class RRay
{
public:
	RVec3 Origin;
	RVec3 Direction;
	float Distance;

	RRay();
	RRay(const RVec3& _origin, const RVec3& _dir, float _dist);
	RRay(const RVec3& _start, const RVec3& _end);
};

#endif