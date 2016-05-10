//=============================================================================
// MathHelper.h by Shiyang Ao, 2016 All Rights Reserved.
//
// Math helper functions
//=============================================================================
#ifndef _MATHHELPER_H
#define _MATHHELPER_H


#define FLT_EQUAL(a,b)			(fabsf(a-b)<FLT_EPSILON)
#define FLT_EQUAL_ZERO(a)		(fabsf(a)<FLT_EPSILON)
#define PI 3.1415926f
#define DEG_TO_RAD(deg)			(PI/180.0f*(deg))
#define RAD_TO_DEG(rad)			((rad)*180.0f/PI)

namespace Math
{
	// Returns random float in [0, 1).
	float RandF();

	// Returns random float in [a, b).
	float RandF(float a, float b);

	template<typename T>
	const T& Max(const T& a, const T& b) { return (a > b) ? a : b; }

	template<typename T>
	const T& Min(const T& a, const T& b) { return (a < b) ? a : b; }

	inline float Lerp(float a, float b, float t) { return a + (b - a) * t; }
}

#endif
