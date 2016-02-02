//=============================================================================
// MathHelper.h by Shiyang Ao, 2016 All Rights Reserved.
//
// Math helper functions
//=============================================================================
#ifndef _MATHHELPER_H
#define _MATHHELPER_H

namespace MathHelper
{
	// Returns random float in [0, 1).
	float RandF();

	// Returns random float in [a, b).
	float RandF(float a, float b);

	template<typename T>
	const T& Max(const T& a, const T& b) { return (a > b) ? a : b; }

	template<typename T>
	const T& Min(const T& a, const T& b) { return (a < b) ? a : b; }
}

#endif
