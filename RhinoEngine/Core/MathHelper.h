//=============================================================================
// MathHelper.h by Shiyang Ao, 2016 All Rights Reserved.
//
// Math helper functions
//=============================================================================

#pragma once

#include <winerror.h>	// FORCEINLINE

#include <stdlib.h>
#include <float.h>

class RVec3;

#define FLT_EQUAL(a,b)			(fabsf((a)-(b))<FLT_EPSILON)
#define FLT_EQUAL_ZERO(a)		(fabsf(a)<FLT_EPSILON)
#define PI 3.14159265358979323846f
#define DEG_TO_RAD(deg)			(PI/180.0f*(deg))
#define RAD_TO_DEG(rad)			((rad)*180.0f/PI)

namespace RMath
{
	FORCEINLINE float Square(float a)
	{
		return a * a;
	}

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

	// Make a degree fall between -180 and 180
	float UnwindDegree(float Degree);

	FORCEINLINE float DegreeToRadian(float Degree)				{ return PI / 180.0f * Degree; }
	FORCEINLINE float RadianToDegree(float Radian)				{ return 180.0f / PI * Radian; }

	void Barycentric(const RVec3& p, const RVec3& a, const RVec3& b, const RVec3& c, float& u, float& v, float &w);

	// Barycentric with all positions considered on xz-plane
	void Barycentric2D_XZ(const RVec3& p, const RVec3& a, const RVec3& b, const RVec3& c, float& u, float& v, float &w);

	// Calculate squared distance from a point to a line segment
	float SqrDist_PointToLineSegment(const RVec3& p, const RVec3& a, const RVec3& b);

	// Get a closest point to given one on a line segment
	RVec3 GetClosestPointOnLineSegment(const RVec3& p, const RVec3& a, const RVec3& b);

	int WeightedDiceRoll(const std::vector<float>& Weights);
}

