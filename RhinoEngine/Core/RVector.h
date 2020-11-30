//=============================================================================
// RVector.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "MathHelper.h"
#include <math.h>
#include <sstream>

#define USE_SIMD_MATH 0

#if (USE_SIMD_MATH == 1)
#include <stdint.h>
#include <xmmintrin.h>
#endif

class RVec2
{
public:
	float x, y;
	
	RVec2()
	{}

	RVec2(float _x, float _y)
		: x(_x), y(_y)
	{}

	RVec2(const float* v)
		: x(v[0]), y(v[1])
	{}

	RVec2(const RVec2& rhs)
		: x(rhs.x), y(rhs.y)
	{}

	RVec2& operator=(const RVec2& rhs)
	{
		x = rhs.x; y = rhs.y;
		return *this;
	}

	RVec2 operator-() const											{ return RVec2(-x, -y); }

	RVec2 operator+(const RVec2& rhs) const							{ return RVec2(x + rhs.x, y + rhs.y); }
	RVec2 operator-(const RVec2& rhs) const							{ return RVec2(x - rhs.x, y - rhs.y); }

	RVec2 operator*(float val) const								{ return RVec2(x * val, y * val); }
	RVec2 operator/(float val) const								{ return RVec2(x / val, y / val); }

	RVec2& operator+=(const RVec2& rhs)								{ x += rhs.x; y += rhs.y; return *this; }
	RVec2& operator-=(const RVec2& rhs)								{ x -= rhs.x; y -= rhs.y; return *this; }
	RVec2& operator*=(float val)									{ x *= val; y *= val; return *this; }
	RVec2& operator/=(float val)									{ x /= val; y /= val; return *this; }

	// Get length of vector
	float Magnitude() const
	{
		return sqrtf(x * x + y * y);
	}

	// Make unit vector
	void Normalize()
	{
		float mag = Magnitude();
		if (!FLT_EQUAL_ZERO(mag))
		{
			x /= mag;
			y /= mag;
		}
	}

	// Dot product
	float Dot(const RVec2& rhs) const
	{
		return x * rhs.x + y * rhs.y;
	}

	// Cross product
	float Cross(const RVec2& rhs) const
	{
		return x * rhs.y - y * rhs.x;
	}

	static RVec2 Zero()
	{
		return RVec2(0.0f, 0.0f);
	}

	static RVec2 Lerp(const RVec2& a, const RVec2& b, float t)
	{
		return RVec2(RMath::Lerp(a.x, b.x, t),
					 RMath::Lerp(a.y, b.y, t));
	}
};

#if (USE_SIMD_MATH == 0)

class RVec3
{
public:
	FORCEINLINE RVec3()
		: x(0.0f), y(0.0f), z(0.0f) {}

	FORCEINLINE RVec3(float _x, float _y, float _z)
		: x(_x), y(_y), z(_z) {}

	FORCEINLINE RVec3(const float* v)
		: x(v[0]), y(v[1]), z(v[2]) {}

	FORCEINLINE RVec3(const RVec3& rhs)
		: x(rhs.x), y(rhs.y), z(rhs.z) {}

	FORCEINLINE RVec3& operator=(const RVec3& rhs)
	{ x = rhs.x; y = rhs.y; z = rhs.z; return *this; }

	FORCEINLINE float X() const { return x; }
	FORCEINLINE float Y() const { return y; }
	FORCEINLINE float Z() const { return z; }

	FORCEINLINE void SetX(float _x) { x = _x; }
	FORCEINLINE void SetY(float _y) { y = _y; }
	FORCEINLINE void SetZ(float _z) { z = _z; }

	FORCEINLINE RVec3 operator-() const
	{ return RVec3(-x, -y, -z); }

	FORCEINLINE RVec3 operator+(const RVec3& rhs) const							{ return RVec3(x + rhs.x, y + rhs.y, z + rhs.z); }
	FORCEINLINE RVec3 operator-(const RVec3& rhs) const							{ return RVec3(x - rhs.x, y - rhs.y, z - rhs.z); }
	FORCEINLINE RVec3 operator*(const RVec3& rhs) const							{ return RVec3(x * rhs.x, y * rhs.y, z * rhs.z); }
	FORCEINLINE RVec3 operator/(const RVec3& rhs) const							{ return RVec3(x / rhs.x, y / rhs.y, z / rhs.z); }

	FORCEINLINE RVec3 operator*(float val) const								{ return RVec3(x * val, y * val, z * val); }
	FORCEINLINE RVec3 operator/(float val) const								{ return RVec3(x / val, y / val, z / val); }

	FORCEINLINE RVec3& operator+=(const RVec3& rhs)								{ x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
	FORCEINLINE RVec3& operator-=(const RVec3& rhs)								{ x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
	FORCEINLINE RVec3& operator*=(float val)									{ x *= val; y *= val; z *= val; return *this; }
	FORCEINLINE RVec3& operator/=(float val)									{ x /= val; y /= val; z /= val; return *this; }

	FORCEINLINE bool operator!=(const RVec3& rhs) const							{ return x != rhs.x || y != rhs.y || z != rhs.z; }
	FORCEINLINE bool operator==(const RVec3& rhs) const							{ return Equals(rhs); }

	FORCEINLINE bool Equals(const RVec3& Other, float Tolerance = FLT_EPSILON) const
	{
		return (fabs(X() - Other.X()) < Tolerance) &&
			   (fabs(Y() - Other.Y()) < Tolerance) &&
			   (fabs(Z() - Other.Z()) < Tolerance);
	}

	FORCEINLINE float SquaredMagitude() const
	{
		return x * x + y * y + z * z;
	}

	FORCEINLINE float SquaredMagitude2D() const
	{
		return x * x + z * z;
	}

	// Get length of vector
	FORCEINLINE float Magnitude() const
	{
		return sqrtf(x * x + y * y + z * z);
	}

	FORCEINLINE float Magnitude2D() const
	{
		return sqrtf(x * x + z * z);
	}

	// Make unit vector
	FORCEINLINE void Normalize()
	{
		float mag = Magnitude();
		if (!FLT_EQUAL_ZERO(mag))
		{
			x /= mag;
			y /= mag;
			z /= mag;
		}
	}

	FORCEINLINE bool HasNan() const
	{
		return isnan(x) || isnan(y) || isnan(z);
	}

	FORCEINLINE bool HasFloatMax() const
	{
		return fabs(x) == FLT_MAX || fabs(y) == FLT_MAX || fabs(z) == FLT_MAX;
	}

	FORCEINLINE bool IsValid() const
	{
		return !HasNan() && !HasFloatMax();
	}

	// All components of vector are zero?
	FORCEINLINE bool IsZero(float Tolerance = FLT_EPSILON) const
	{
		return fabs(x) < Tolerance && fabs(y) < Tolerance && fabs(z) < Tolerance;
	}

	FORCEINLINE std::string ToString() const
	{
		std::ostringstream StringStream;
		StringStream << "(" << x << ", " << y << ", " << z << ")";
		return StringStream.str();
	}

	RVec3 GetNormalized() const
	{
		float mag = Magnitude();
		RVec3 n = *this;
		if (!FLT_EQUAL_ZERO(mag))
		{
			float one_over_mag = 1.0f / mag;
			n.x *= one_over_mag;
			n.y *= one_over_mag;
			n.z *= one_over_mag;
		}
		return n;
	}

	RVec3 GetNormalized2D() const
	{
		RVec3 Result = *this;
		Result.SetY(0.0f);
		return Result.GetNormalized();
	}

	// Dot product
	static float Dot(const RVec3& lhs, const RVec3& rhs)
	{
		return lhs.X() * rhs.X() + lhs.Y() * rhs.Y() + lhs.Z() * rhs.Z();
	}

	// Dot product in XZ plane
	static float Dot2D(const RVec3& lhs, const RVec3& rhs)
	{
		return lhs.X() * rhs.X() + lhs.Z() * rhs.Z();
	}

	// Cross product
	static RVec3 Cross(const RVec3& lhs, const RVec3& rhs)
	{
		return RVec3(lhs.Y() * rhs.Z() - lhs.Z() * rhs.Y(),
					 lhs.Z() * rhs.X() - lhs.X() * rhs.Z(),
					 lhs.X() * rhs.Y() - lhs.Y() * rhs.X());
	}

	// Cross product in XZ plane. Returns Y value only
	static float Cross2D(const RVec3& lhs, const RVec3& rhs)
	{
		return lhs.Z() * rhs.X() - lhs.X() * rhs.Z();
	}


	// Calculates the distance between two vectors
	static float Distance(const RVec3& lhs, const RVec3& rhs)
	{
		return (lhs - rhs).Magnitude();
	}

	static float SquaredDistance(const RVec3& lhs, const RVec3& rhs)
	{
		return (lhs - rhs).SquaredMagitude();
	}

	static float Distance2D(const RVec3& lhs, const RVec3& rhs)
	{
		return (lhs - rhs).Magnitude2D();
	}

	static float SquaredDistance2D(const RVec3& lhs, const RVec3& rhs)
	{
		return (lhs - rhs).SquaredMagitude2D();
	}

	static RVec3 Zero()
	{
		return RVec3(0.0f, 0.0f, 0.0f);
	}

	static RVec3 Lerp(const RVec3& a, const RVec3& b, float t)
	{
		return RVec3(RMath::Lerp(a.x, b.x, t),
					 RMath::Lerp(a.y, b.y, t),
					 RMath::Lerp(a.z, b.z, t));
	}

private:
	float x, y, z;
};

#else

//////////////////////////////////////////////////////////////////////////
// SIMD implementation of vector3
//
// Reference 'How To Write A Maths Library In 2016' by Richard Mitton
// http://www.codersnotes.com/notes/maths-lib-2016/
//////////////////////////////////////////////////////////////////////////
class RVec3_SIMD
{
public:
	FORCEINLINE RVec3_SIMD() {}

	FORCEINLINE RVec3_SIMD(float _x, float _y, float _z)
	{
		m = _mm_set_ps(_z, _z, _y, _x);
	}

	FORCEINLINE RVec3_SIMD(const float* v)
	{
		m = _mm_set_ps(v[2], v[2], v[1], v[0]);
	}

	FORCEINLINE RVec3_SIMD(const RVec3_SIMD& rhs)
	{
		m = rhs.m;
	}

	FORCEINLINE RVec3_SIMD& operator=(const RVec3_SIMD& rhs)
	{
		m = rhs.m; return *this;
	}

	FORCEINLINE float X() const { return _mm_cvtss_f32(m); }
	FORCEINLINE float Y() const { return _mm_cvtss_f32(_mm_shuffle_ps(m, m, _MM_SHUFFLE(1, 1, 1, 1))); }
	FORCEINLINE float Z() const { return _mm_cvtss_f32(_mm_shuffle_ps(m, m, _MM_SHUFFLE(2, 2, 2, 2))); }

	FORCEINLINE void SetX(float x)
	{
		m = _mm_move_ss(m, _mm_set_ss(x));
	}

	FORCEINLINE void SetY(float y)
	{
		__m128 t = _mm_move_ss(m, _mm_set_ss(y));
		t = _mm_shuffle_ps(t, t, _MM_SHUFFLE(3, 2, 0, 0));
		m = _mm_move_ss(t, m);
	}

	FORCEINLINE void SetZ(float z)
	{
		__m128 t = _mm_move_ss(m, _mm_set_ss(z));
		t = _mm_shuffle_ps(t, t, _MM_SHUFFLE(3, 0, 1, 0));
		m = _mm_move_ss(t, m);
	}

	FORCEINLINE RVec3_SIMD operator-() const
	{
		return RVec3_SIMD(_mm_setzero_ps()) - *this;
	}

	FORCEINLINE RVec3_SIMD operator+(const RVec3_SIMD& rhs) const { RVec3_SIMD r; r.m = _mm_add_ps(m, rhs.m); return r; }
	FORCEINLINE RVec3_SIMD operator-(const RVec3_SIMD& rhs) const { RVec3_SIMD r; r.m = _mm_sub_ps(m, rhs.m); return r; }
	FORCEINLINE RVec3_SIMD operator*(const RVec3_SIMD& rhs) const { RVec3_SIMD r; r.m = _mm_mul_ps(m, rhs.m); return r; }
	FORCEINLINE RVec3_SIMD operator/(const RVec3_SIMD& rhs) const { RVec3_SIMD r; r.m = _mm_div_ps(m, rhs.m); return r; }

	FORCEINLINE RVec3_SIMD operator*(float val) const { RVec3_SIMD r; r.m = _mm_mul_ps(m, _mm_set1_ps(val)); return r; }
	FORCEINLINE RVec3_SIMD operator/(float val) const { RVec3_SIMD r; r.m = _mm_div_ps(m, _mm_set1_ps(val)); return r; }

	FORCEINLINE RVec3_SIMD& operator+=(const RVec3_SIMD& rhs) { *this = *this + rhs; return *this; }
	FORCEINLINE RVec3_SIMD& operator-=(const RVec3_SIMD& rhs) { *this = *this - rhs; return *this; }
	FORCEINLINE RVec3_SIMD& operator*=(float val) { *this = *this * val; return *this; }
	FORCEINLINE RVec3_SIMD& operator/=(float val) { *this = *this / val; return *this; }

	FORCEINLINE float SquaredMagitude() const
	{
		return Dot(*this, *this);
	}

	// Get length of vector
	FORCEINLINE float Magnitude() const
	{
		return sqrtf(SquaredMagitude());
	}

	// Make unit vector
	FORCEINLINE void Normalize()
	{
		float mag = Magnitude();
		if (!FLT_EQUAL_ZERO(mag))
		{
			SetX(X() / mag);
			SetY(Y() / mag);
			SetZ(Z() / mag);
		}
	}

	RVec3_SIMD GetNormalized() const
	{
		float mag = Magnitude();
		RVec3_SIMD n = *this;
		if (!FLT_EQUAL_ZERO(mag))
		{
			float one_over_mag = 1.0f / mag;
			n.SetX(n.X() * one_over_mag);
			n.SetY(n.Y() * one_over_mag);
			n.SetZ(n.Z() * one_over_mag);
		}
		return n;
	}

	// Dot product
	static float Dot(const RVec3_SIMD& lhs, const RVec3_SIMD& rhs)
	{
		return lhs.X() * rhs.X() + lhs.Y() * rhs.Y() + lhs.Z() * rhs.Z();
	}

	// Cross product
	static RVec3_SIMD Cross(const RVec3_SIMD& lhs, const RVec3_SIMD& rhs)
	{
		return (lhs.zxy() * rhs - lhs * rhs.zxy()).zxy();
	}

	static RVec3_SIMD Zero()
	{
		return RVec3_SIMD(0.0f, 0.0f, 0.0f);
	}

	static RVec3_SIMD Lerp(const RVec3_SIMD& a, const RVec3_SIMD& b, float t)
	{
		return RVec3_SIMD(RMath::Lerp(a.X(), b.X(), t),
						  RMath::Lerp(a.Y(), b.Y(), t),
						  RMath::Lerp(a.Z(), b.Z(), t));
	}

private:
	FORCEINLINE RVec3_SIMD(__m128 _m) { m = _m; }

	FORCEINLINE RVec3_SIMD yzx() const { return RVec3_SIMD(_mm_shuffle_ps(m, m, _MM_SHUFFLE(0, 0, 2, 1))); }
	FORCEINLINE RVec3_SIMD zxy() const { return RVec3_SIMD(_mm_shuffle_ps(m, m, _MM_SHUFFLE(1, 1, 0, 2))); }

private:
	__m128 m;
};

typedef RVec3_SIMD RVec3;

#endif	// if (USE_SSE_MATH == 0)

class RVec4
{
public:
	float x, y, z, w;

	RVec4()
	{}

	RVec4(float _x, float _y, float _z, float _w = 1.0f)
		: x(_x), y(_y), z(_z), w(_w)
	{}

	RVec4(const float* v)
		: x(v[0]), y(v[1]), z(v[2]), w(v[3])
	{}

	RVec4(const RVec4& rhs)
		: x(rhs.x), y(rhs.y), z(rhs.z), w(rhs.w)
	{}

	RVec4(const RVec3 v, float _w = 1.0f)
		: x(v.X()), y(v.Y()), z(v.Z()), w(_w)
	{}

	RVec4& operator=(const RVec4& rhs)
	{
		x = rhs.x; y = rhs.y; z = rhs.z; w = rhs.w;
		return *this;
	}

	RVec4 operator*(float val) const								{ return RVec4(x * val, y * val, z * val, w * val); }
	RVec4 operator/(float val) const								{ return RVec4(x / val, y / val, z / val, w / val); }

	RVec4& operator*=(float val)									{ x *= val; y *= val; z *= val; w *= val; return *this; }
	RVec4& operator/=(float val)									{ x /= val; y /= val; z /= val; w /= val; return *this; }

	float& operator[](int i)										{ return (&x)[i]; }

	RVec3 ToVec3() const
	{
		return RVec3(x, y, z);
	}

	static RVec4 Lerp(const RVec4& a, const RVec4& b, float t)
	{
		return RVec4(RMath::Lerp(a.x, b.x, t),
					 RMath::Lerp(a.y, b.y, t),
					 RMath::Lerp(a.z, b.z, t),
					 RMath::Lerp(a.w, b.w, t));
	}
};

