//=============================================================================
// RQuat.h by Shiyang Ao, 2017 All Rights Reserved.
//
// Quaternion class
//=============================================================================
#pragma once

class RMatrix3;

class RQuat
{
public:
	float w, x, y, z;

	RQuat() {}
	RQuat(float _w, float _x, float _y, float _z)
		: w(_w), x(_x), y(_y), z(_z)
	{
	}

	RQuat(const RQuat& rhs)
		: w(rhs.w), x(rhs.x), y(rhs.y), z(rhs.z)
	{
	}

	FORCEINLINE RQuat& operator=(const RQuat& rhs)			{ w = rhs.w; x = rhs.x; y = rhs.y; z = rhs.z; return *this; }

	FORCEINLINE RQuat operator+(const RQuat& rhs) const		{ return RQuat(w + rhs.w, x + rhs.x, y + rhs.y, z + rhs.z); }
	FORCEINLINE RQuat operator-(const RQuat& rhs) const		{ return RQuat(w - rhs.w, x - rhs.x, y - rhs.y, z - rhs.z); }
	FORCEINLINE RQuat operator*(const RQuat& rhs) const
	{
		return RQuat(
			w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z,
			w * rhs.w + x * rhs.x + y * rhs.y - z * rhs.z,
			w * rhs.w - x * rhs.x + y * rhs.y + z * rhs.z,
			w * rhs.w + x * rhs.x - y * rhs.y + z * rhs.z
		);
	}

	FORCEINLINE RQuat operator*(float val) const			{ return RQuat(w * val, x * val, y * val, z * val); }
	FORCEINLINE RQuat operator/(float val) const			{ float div_val = 1.0f / val; return RQuat(w * div_val, x * div_val, y * div_val, z * div_val); }

	FORCEINLINE RQuat& operator+=(const RQuat& rhs)			{ w += rhs.w; x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
	FORCEINLINE RQuat& operator*=(const RQuat& rhs)			{ *this = *this * rhs; return *this; }
	FORCEINLINE RQuat operator*=(float val)					{ w *= val; x *= val; y *= val; z *= val; return *this; }
	FORCEINLINE RQuat operator/=(float val)					{ float div_val = 1.0f / val; w *= div_val; x *= div_val; y *= val; z *= div_val; return *this; }
	FORCEINLINE RQuat operator-() const						{ return RQuat(w, -x, -y, -z); }

	FORCEINLINE RVec3 operator*(const RVec3& vec) const		{ RVec3 vq(x, y, z); return vq * 2.0f * RVec3::Dot(vq, vec) + vec * (w * w - RVec3::Dot(vq, vq)) + RVec3::Cross(vq, vec) * 2.0f * w; }

	FORCEINLINE void Conjugate()							{ x = -x; y = -y; z = -z; }
	FORCEINLINE float Norm() const							{ return sqrtf(w * w + x * x + y * y + z * z); }
	FORCEINLINE void Normalize()							{ *this /= Norm(); }

	RMatrix3 GetRotationMatrix() const;

	static RQuat Euler(float axis_x, float axis_y, float axis_z);

	static RQuat IDENTITY;
};
