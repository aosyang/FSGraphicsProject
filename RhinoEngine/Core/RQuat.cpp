//=============================================================================
// RQuat.cpp by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RQuat.h"
#include "RMatrix.h"

RQuat RQuat::IDENTITY(1.0f, 0.0f, 0.0f, 0.0f);

RMatrix3 RQuat::GetRotationMatrix() const
{
	float xs2 = x * x * 2;
	float ys2 = y * y * 2;
	float zs2 = z * z * 2;
	float xy2 = x * y * 2;
	float xz2 = x * z * 2;
	float yz2 = y * z * 2;
	float wx2 = w * x * 2;
	float wy2 = w * y * 2;
	float wz2 = w * z * 2;

	return RMatrix3(
		1.0f - ys2 - zs2,	xy2 + wz2,			xz2 - wy2,
		xy2 - wz2,			1.0f - xs2 - zs2,	yz2 + wx2,
		xz2 + wy2,			yz2 - wx2,			1.0f - xs2 - ys2
	);
}

RQuat RQuat::Euler(float axis_x, float axis_y, float axis_z)
{
	float cr, cp, cy, sr, sp, sy, cpcy, spsy;

	cr = cosf(0.5f * axis_x);
	cp = cosf(0.5f * axis_y);
	cy = cosf(0.5f * axis_z);
	sr = sinf(0.5f * axis_x);
	sp = sinf(0.5f * axis_y);
	sy = sinf(0.5f * axis_z);
	cpcy = cp * cy;
	spsy = sp * sy;

	return RQuat(
		cr * cpcy + sr * spsy,
		sr * cpcy - cr * spsy,
		cr * sp * cy + sr * cp * sy,
		cr * cp * sy - sr * sp * cy
	);
}

RQuat RQuat::SlerpUnnormalized(const RQuat& a, const RQuat& b, float t)
{
	float dot = RQuat::Dot(a, b);

	if (fabsf(dot) >= 1.0f)
	{
		return a;
	}

	float Sgn = 1.0f;
	if (dot < 0.0f)
	{
		Sgn = -1.0f;
		dot = -dot;
	}

	dot = Math::Clamp(dot, -1, 1);
	float Theta = acosf(dot);

	float InvSinTheta = 1.0f / sinf(Theta);
	RQuat Result = a * (sinf((1.0f - t) * Theta) * InvSinTheta) + b * (sinf(t * Theta) * InvSinTheta * Sgn);

	return Result;
}
