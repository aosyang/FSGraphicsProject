//=============================================================================
// MathHelper.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#include "Rhino.h"

#include "MathHelper.h"

namespace MathHelper
{
	// Returns random float in [0, 1).
	float RandF()
	{
		return (float)(rand()) / (float)RAND_MAX;
	}

	// Returns random float in [a, b).
	float RandF(float a, float b)
	{
		return a + RandF()*(b - a);
	}
}