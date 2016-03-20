//=============================================================================
// RAabb.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"
#include "RAabb.h"

#include <float.h>

RAabb RAabb::Default;

RAabb::RAabb()
	:pMin(FLT_MAX, FLT_MAX, FLT_MAX),
	 pMax(-FLT_MAX, -FLT_MAX, -FLT_MAX)
{
}
