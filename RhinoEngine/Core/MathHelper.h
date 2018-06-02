//=============================================================================
// MathHelper.h by Shiyang Ao, 2016 All Rights Reserved.
//
// Math helper functions
//=============================================================================
#ifndef _MATHHELPER_H
#define _MATHHELPER_H


#define FLT_EQUAL(a,b)			(fabsf((a)-(b))<FLT_EPSILON)
#define FLT_EQUAL_ZERO(a)		(fabsf(a)<FLT_EPSILON)
#define PI 3.1415926f
#define DEG_TO_RAD(deg)			(PI/180.0f*(deg))
#define RAD_TO_DEG(rad)			((rad)*180.0f/PI)

namespace Math
{
	// Returns random float in [0, 1].
	FORCEINLINE float RandF()
	{
		return (float)(rand()) / (float)RAND_MAX;
	}

	// Returns random float in [a, b].
	FORCEINLINE float RandRangedF(float a, float b)
	{
		return a + RandF()*(b - a);
	}

	// Returns random integer in [a, b]
	FORCEINLINE int RandRangedInt(int a, int b)
	{
		float f = RandF();
		if (f == 1.0f) f = 0.0f;
		int d = (int)(f * (b - a + 1));
		return a + d;
	}

	template<typename T>
	const T& Max(const T& a, const T& b)						{ return (a > b) ? a : b; }

	template<typename T>
	const T& Min(const T& a, const T& b)						{ return (a < b) ? a : b; }

	FORCEINLINE float Lerp(float a, float b, float t)			{ return a + (b - a) * t; }

	// Restrict a value to a given range [min, max] 
	FORCEINLINE float Clamp(float a, float min, float max)		{ return (a < min) ? min : (a > max) ? max : a; }
}

#endif
