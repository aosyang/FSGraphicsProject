//=============================================================================
// MathHelper.cpp by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#include "MathHelper.h"

#include "Core/CoreTypes.h"
#include "RLog.h"

#define DEBUG_CHECK_NAN 1

float RMath::UnwindDegree(float Degree)
{
	while (Degree > 180) { Degree -= 360; }
	while (Degree < -180) { Degree += 360; }
	return Degree;
}

void RMath::Barycentric(const RVec3& p, const RVec3& a, const RVec3& b, const RVec3& c, float& u, float& v, float &w)
{
	RVec3 v0 = b - a, v1 = c - a, v2 = p - a;
	float d00 = RVec3::Dot(v0, v0);
	float d01 = RVec3::Dot(v0, v1);
	float d11 = RVec3::Dot(v1, v1);
	float d20 = RVec3::Dot(v2, v0);
	float d21 = RVec3::Dot(v2, v1);
	float denom = d00 * d11 - d01 * d01;

	v = (d11 * d20 - d01 * d21) / denom;
	w = (d00 * d21 - d01 * d20) / denom;
	u = 1.0f - v - w;

#if DEBUG_CHECK_NAN
	if (isnan(u) || isnan(v) || isnan(w))
	{
		RLog("Error: Barycentric() - one or more of output values is NaN! (u = %f, v = %f, w = %f)\n", u, v, w);
		RLog("p:%s, a:%s, b:%s, c:%s\n",
			p.ToString().c_str(),
			a.ToString().c_str(),
			b.ToString().c_str(),
			c.ToString().c_str());
		DebugBreak();
	}
#endif	// DEBUG_CHECK_NAN}
}

void RMath::Barycentric2D_XZ(const RVec3& p, const RVec3& a, const RVec3& b, const RVec3& c, float& u, float& v, float &w)
{
	const RVec3 p0(p.X(), 0, p.Z());
	const RVec3 a0(a.X(), 0, a.Z());
	const RVec3 b0(b.X(), 0, b.Z());
	const RVec3 c0(c.X(), 0, c.Z());

	Barycentric(p0, a0, b0, c0, u, v, w);
}

float RMath::SqrDist_PointToLineSegment(const RVec3& p, const RVec3& a, const RVec3& b)
{
	return (p - GetClosestPointOnLineSegment(p, a, b)).SquaredMagitude();
}

RVec3 RMath::GetClosestPointOnLineSegment(const RVec3& p, const RVec3& a, const RVec3& b)
{
	assert(a != b);

	RVec3 ap = p - a;
	RVec3 ab = b - a;

	float f = RVec3::Dot(ap, ab) / RVec3::Dot(ab, ab);
	if (f <= 0.0f)
	{
		return a;
	}
	else if (f >= 1.0f)
	{
		return b;
	}
	else
	{
		return a + ab * f;
	}
}

int RMath::WeightedDiceRoll(const std::vector<float>& Weights)
{
	float Sum = 0.0f;
	for (int i = 0; i < (int)Weights.size(); i++)
	{
		Sum += Weights[i];
	}

	float Val = RMath::RandRangedF(0.0f, Sum);
	for (int i = 0; i < (int)Weights.size(); i++)
	{
		if (Val <= Weights[i])
		{
			return i;
		}

		Val -= Weights[i];
	}

	// Shouldn't be here
	assert(false);
	return -1;
}
