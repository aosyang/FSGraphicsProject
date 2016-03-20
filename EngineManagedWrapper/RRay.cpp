//=============================================================================
// RRay.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "stdafx.h"
#include "Rhino.h"
#include "RRay.h"

RRay::RRay()
{
}

RRay::RRay(const RVec3& _origin, const RVec3& _dir, float _dist)
	: Origin(_origin), Direction(_dir.GetNormalizedVec3()), Distance(_dist)
{
}

RRay::RRay(const RVec3& _start, const RVec3& _end)
	: Origin(_start), Direction((_end - _start).GetNormalizedVec3()), Distance((_end - _start).Magnitude())
{
}
