//=============================================================================
// RMatrix.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "RMatrix.h"
#include <iomanip>
#include <assert.h>

RMatrix4 RMatrix4::IDENTITY = RMatrix4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);

RMatrix4 RMatrix4::Zero = RMatrix4(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

//////////////////////////////////////////////////////////////////////////
// Matrix3 implementations
//////////////////////////////////////////////////////////////////////////

RMatrix3::RMatrix3()
{

}

RMatrix3::RMatrix3(float Array[9])
{
	memcpy(&_m11, Array, sizeof(float) * 9);
}

RMatrix3::RMatrix3(float m11, float m12, float m13, float m21, float m22, float m23, float m31, float m32, float m33)
	: _m11(m11), _m12(m12), _m13(m13), _m21(m21), _m22(m22), _m23(m23), _m31(m31), _m32(m32), _m33(m33)
{

}

RMatrix3& RMatrix3::operator=(const RMatrix3& rhs)
{
	if (this != &rhs)
	{
		_m11 = rhs._m11; _m12 = rhs._m12; _m13 = rhs._m13;
		_m21 = rhs._m21; _m22 = rhs._m22; _m23 = rhs._m23;
		_m31 = rhs._m31; _m32 = rhs._m32; _m33 = rhs._m33;
	}

	return *this;
}

RMatrix3 RMatrix3::operator*(const RMatrix3& rhs) const
{
	RMatrix3 mat;

	mat.m[0][0] = m[0][0] * rhs.m[0][0] + m[0][1] * rhs.m[1][0] + m[0][2] * rhs.m[2][0];
	mat.m[0][1] = m[0][0] * rhs.m[0][1] + m[0][1] * rhs.m[1][1] + m[0][2] * rhs.m[2][1];
	mat.m[0][2] = m[0][0] * rhs.m[0][2] + m[0][1] * rhs.m[1][2] + m[0][2] * rhs.m[2][2];

	mat.m[1][0] = m[1][0] * rhs.m[0][0] + m[1][1] * rhs.m[1][0] + m[1][2] * rhs.m[2][0];
	mat.m[1][1] = m[1][0] * rhs.m[0][1] + m[1][1] * rhs.m[1][1] + m[1][2] * rhs.m[2][1];
	mat.m[1][2] = m[1][0] * rhs.m[0][2] + m[1][1] * rhs.m[1][2] + m[1][2] * rhs.m[2][2];

	mat.m[2][0] = m[2][0] * rhs.m[0][0] + m[2][1] * rhs.m[1][0] + m[2][2] * rhs.m[2][0];
	mat.m[2][1] = m[2][0] * rhs.m[0][1] + m[2][1] * rhs.m[1][1] + m[2][2] * rhs.m[2][1];
	mat.m[2][2] = m[2][0] * rhs.m[0][2] + m[2][1] * rhs.m[1][2] + m[2][2] * rhs.m[2][2];

	return mat;
}

RMatrix3& RMatrix3::operator*=(const RMatrix3& rhs)
{
	*this = *this * rhs;
	return *this;
}

bool RMatrix3::Decompose(RQuat& Rotation, RVec3& Scale) const
{
	RVec3 v0(m[0][0], m[1][0], m[2][0]);
	RVec3 v1(m[0][1], m[1][1], m[2][1]);
	RVec3 v2(m[0][2], m[1][2], m[2][2]);

	float sx = v0.Magnitude();
	float sy = v1.Magnitude();
	float sz = v2.Magnitude();

	if (FLT_EQUAL_ZERO(sx) || FLT_EQUAL_ZERO(sy) || FLT_EQUAL_ZERO(sz))
	{
		return false;
	}

	float inv_sx = 1.0f / sx;
	float inv_sy = 1.0f / sy;
	float inv_sz = 1.0f / sz;

	v0 *= inv_sx;
	v1 *= inv_sy;
	v2 *= inv_sz;

	// Divide all components by scale
	const float n_m11 = _m11 * inv_sx;
	const float n_m21 = _m21 * inv_sx;
	const float n_m31 = _m31 * inv_sx;
	const float n_m12 = _m12 * inv_sy;
	const float n_m22 = _m22 * inv_sy;
	const float n_m32 = _m32 * inv_sy;
	const float n_m13 = _m13 * inv_sz;
	const float n_m23 = _m23 * inv_sz;
	const float n_m33 = _m33 * inv_sz;

	float t;

	if (n_m33 < 0)
	{
		if (n_m11 > n_m22)
		{
			t = 1 + n_m11 - n_m22 - n_m33;
			Rotation = RQuat(n_m23 - n_m32, t, n_m12 + n_m21, n_m31 + n_m13);
		}
		else
		{
			t = 1 - n_m11 + n_m22 - n_m33;
			Rotation = RQuat(n_m31 - n_m13, n_m12 + n_m21, t, n_m23 + n_m32);
		}
	}
	else
	{
		if (n_m11 < -n_m22)
		{
			t = 1 - n_m11 - n_m22 + n_m33;
			Rotation = RQuat(n_m12 - n_m21, n_m31 + n_m13, n_m23 + n_m32, t);
		}
		else
		{
			t = 1 + n_m11 + n_m22 + n_m33;
			Rotation = RQuat(t, n_m23 - n_m32, n_m31 - n_m13, n_m12 - n_m21);
		}
	}

	Rotation *= 0.5f / sqrtf(t);
	Rotation.Normalize();
	Scale = RVec3(sx, sy, sz);

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Matrix4 implementations
//////////////////////////////////////////////////////////////////////////

RMatrix4::RMatrix4()
	: RMatrix4(RMatrix4::IDENTITY)
{
}

RMatrix4::RMatrix4(float Array[16])
{
	memcpy(&_m11, Array, sizeof(float) * 16);
}

RMatrix4::RMatrix4(float m11, float m12, float m13, float m14,
				   float m21, float m22, float m23, float m24,
				   float m31, float m32, float m33, float m34,
				   float m41, float m42, float m43, float m44)
				   : _m11(m11), _m12(m12), _m13(m13), _m14(m14),
				     _m21(m21), _m22(m22), _m23(m23), _m24(m24),
				     _m31(m31), _m32(m32), _m33(m33), _m34(m34),
				     _m41(m41), _m42(m42), _m43(m43), _m44(m44)
{
	
}

RMatrix4 RMatrix4::operator*(const RMatrix4& rhs) const
{
	RMatrix4 mat;

	mat.m[0][0] = m[0][0] * rhs.m[0][0] + m[0][1] * rhs.m[1][0] + m[0][2] * rhs.m[2][0] + m[0][3] * rhs.m[3][0];
	mat.m[0][1] = m[0][0] * rhs.m[0][1] + m[0][1] * rhs.m[1][1] + m[0][2] * rhs.m[2][1] + m[0][3] * rhs.m[3][1];
	mat.m[0][2] = m[0][0] * rhs.m[0][2] + m[0][1] * rhs.m[1][2] + m[0][2] * rhs.m[2][2] + m[0][3] * rhs.m[3][2];
	mat.m[0][3] = m[0][0] * rhs.m[0][3] + m[0][1] * rhs.m[1][3] + m[0][2] * rhs.m[2][3] + m[0][3] * rhs.m[3][3];

	mat.m[1][0] = m[1][0] * rhs.m[0][0] + m[1][1] * rhs.m[1][0] + m[1][2] * rhs.m[2][0] + m[1][3] * rhs.m[3][0];
	mat.m[1][1] = m[1][0] * rhs.m[0][1] + m[1][1] * rhs.m[1][1] + m[1][2] * rhs.m[2][1] + m[1][3] * rhs.m[3][1];
	mat.m[1][2] = m[1][0] * rhs.m[0][2] + m[1][1] * rhs.m[1][2] + m[1][2] * rhs.m[2][2] + m[1][3] * rhs.m[3][2];
	mat.m[1][3] = m[1][0] * rhs.m[0][3] + m[1][1] * rhs.m[1][3] + m[1][2] * rhs.m[2][3] + m[1][3] * rhs.m[3][3];

	mat.m[2][0] = m[2][0] * rhs.m[0][0] + m[2][1] * rhs.m[1][0] + m[2][2] * rhs.m[2][0] + m[2][3] * rhs.m[3][0];
	mat.m[2][1] = m[2][0] * rhs.m[0][1] + m[2][1] * rhs.m[1][1] + m[2][2] * rhs.m[2][1] + m[2][3] * rhs.m[3][1];
	mat.m[2][2] = m[2][0] * rhs.m[0][2] + m[2][1] * rhs.m[1][2] + m[2][2] * rhs.m[2][2] + m[2][3] * rhs.m[3][2];
	mat.m[2][3] = m[2][0] * rhs.m[0][3] + m[2][1] * rhs.m[1][3] + m[2][2] * rhs.m[2][3] + m[2][3] * rhs.m[3][3];

	mat.m[3][0] = m[3][0] * rhs.m[0][0] + m[3][1] * rhs.m[1][0] + m[3][2] * rhs.m[2][0] + m[3][3] * rhs.m[3][0];
	mat.m[3][1] = m[3][0] * rhs.m[0][1] + m[3][1] * rhs.m[1][1] + m[3][2] * rhs.m[2][1] + m[3][3] * rhs.m[3][1];
	mat.m[3][2] = m[3][0] * rhs.m[0][2] + m[3][1] * rhs.m[1][2] + m[3][2] * rhs.m[2][2] + m[3][3] * rhs.m[3][2];
	mat.m[3][3] = m[3][0] * rhs.m[0][3] + m[3][1] * rhs.m[1][3] + m[3][2] * rhs.m[2][3] + m[3][3] * rhs.m[3][3];

	return mat;
}

RMatrix4& RMatrix4::operator=(const RMatrix4& rhs)
{
	if (this != &rhs)
	{
		_m11 = rhs._m11; _m12 = rhs._m12; _m13 = rhs._m13; _m14 = rhs._m14;
		_m21 = rhs._m21; _m22 = rhs._m22; _m23 = rhs._m23; _m24 = rhs._m24;
		_m31 = rhs._m31; _m32 = rhs._m32; _m33 = rhs._m33; _m34 = rhs._m34;
		_m41 = rhs._m41; _m42 = rhs._m42; _m43 = rhs._m43; _m44 = rhs._m44;
	}

	return *this;
}

RMatrix4& RMatrix4::operator*=(const RMatrix4& rhs)
{
	*this = *this * rhs;
	return *this;
}

RMatrix4 RMatrix4::FastInverse() const
{
	RMatrix4 view = *this;

	// Swap row and column in 3x3 matrix
	std::swap(view.m[0][1], view.m[1][0]);
	std::swap(view.m[0][2], view.m[2][0]);
	std::swap(view.m[1][2], view.m[2][1]);

	float vx = view.m[3][0] * view.m[0][0] + view.m[3][1] * view.m[1][0] + view.m[3][2] * view.m[2][0];
	float vy = view.m[3][0] * view.m[0][1] + view.m[3][1] * view.m[1][1] + view.m[3][2] * view.m[2][1];
	float vz = view.m[3][0] * view.m[0][2] + view.m[3][1] * view.m[1][2] + view.m[3][2] * view.m[2][2];

	view.m[3][0] = -vx;
	view.m[3][1] = -vy;
	view.m[3][2] = -vz;

	assert(!HasNan());
	return view;
}

bool RMatrix4::Decompose(RVec3& Position, RQuat& Rotaion, RVec3& Scale) const
{
	bool DecomposeMatrix3Result = GetRotationMatrix().Decompose(Rotaion, Scale);

	if (DecomposeMatrix3Result)
	{
		Position = RVec3(_m41, _m42, _m43);
	}

	return DecomposeMatrix3Result;
}

RVec3 RMatrix4::RotateVector(const RVec3& vec) const
{
	float _x = vec.X() * m[0][0] + vec.Y() * m[1][0] + vec.Z() * m[2][0];
	float _y = vec.X() * m[0][1] + vec.Y() * m[1][1] + vec.Z() * m[2][1];
	float _z = vec.X() * m[0][2] + vec.Y() * m[1][2] + vec.Z() * m[2][2];

	return RVec3(_x, _y, _z);
}

RVec3 RMatrix4::Transform(const RVec3& vec) const
{
	RVec4 p(vec.X(), vec.Y(), vec.Z(), 1.0f);
	return (p * (*this)).ToVec3();
}

// Calculate the determinant of a 3x3 matrix
float Matrix_Determinant(float e_11,float e_12,float e_13,
						 float e_21,float e_22,float e_23,
						 float e_31,float e_32,float e_33)
{
	return e_11 * e_22 * e_33 + e_12 * e_23 * e_31 + e_13 * e_21 * e_32 -
		   e_11 * e_23 * e_32 - e_12 * e_21 * e_33 - e_13 * e_22 * e_31;
}

// Calculate the determinal of a RMatrix4
float Matrix_Determinant(const RMatrix4& m)
{
	return m._m11 * Matrix_Determinant(m._m22, m._m23, m._m24, m._m32, m._m33, m._m34, m._m42, m._m43, m._m44) -
		   m._m12 * Matrix_Determinant(m._m21, m._m23, m._m24, m._m31, m._m33, m._m34, m._m41, m._m43, m._m44) +
		   m._m13 * Matrix_Determinant(m._m21, m._m22, m._m24, m._m31, m._m32, m._m34, m._m41, m._m42, m._m44) -
		   m._m14 * Matrix_Determinant(m._m21, m._m22, m._m23, m._m31, m._m32, m._m33, m._m41, m._m42, m._m43);
}


RMatrix4 RMatrix4::Inverse() const
{
	float det = Matrix_Determinant(*this);
	if (FLT_EQUAL_ZERO(det))
		return *this;

	RMatrix4 r;
	float inv_det = 1.0f / det;

	r._m11 =  Matrix_Determinant(_m22, _m23, _m24, _m32, _m33, _m34, _m42, _m43, _m44) * inv_det;
	r._m12 = -Matrix_Determinant(_m12, _m13, _m14, _m32, _m33, _m34, _m42, _m43, _m44) * inv_det;
	r._m13 =  Matrix_Determinant(_m12, _m13, _m14, _m22, _m23, _m24, _m42, _m43, _m44) * inv_det;
	r._m14 = -Matrix_Determinant(_m12, _m13, _m14, _m22, _m23, _m24, _m32, _m33, _m34) * inv_det;
	r._m21 = -Matrix_Determinant(_m21, _m23, _m24, _m31, _m33, _m34, _m41, _m43, _m44) * inv_det;
	r._m22 =  Matrix_Determinant(_m11, _m13, _m14, _m31, _m33, _m34, _m41, _m43, _m44) * inv_det;
	r._m23 = -Matrix_Determinant(_m11, _m13, _m14, _m21, _m23, _m24, _m41, _m43, _m44) * inv_det;
	r._m24 =  Matrix_Determinant(_m11, _m13, _m14, _m21, _m23, _m24, _m31, _m33, _m34) * inv_det;
	r._m31 =  Matrix_Determinant(_m21, _m22, _m24, _m31, _m32, _m34, _m41, _m42, _m44) * inv_det;
	r._m32 = -Matrix_Determinant(_m11, _m12, _m14, _m31, _m32, _m34, _m41, _m42, _m44) * inv_det;
	r._m33 =  Matrix_Determinant(_m11, _m12, _m14, _m21, _m22, _m24, _m41, _m42, _m44) * inv_det;
	r._m34 = -Matrix_Determinant(_m11, _m12, _m14, _m21, _m22, _m24, _m31, _m32, _m34) * inv_det;
	r._m41 = -Matrix_Determinant(_m21, _m22, _m23, _m31, _m32, _m33, _m41, _m42, _m43) * inv_det;
	r._m42 =  Matrix_Determinant(_m11, _m12, _m13, _m31, _m32, _m33, _m41, _m42, _m43) * inv_det;
	r._m43 = -Matrix_Determinant(_m11, _m12, _m13, _m21, _m22, _m23, _m41, _m42, _m43) * inv_det;
	r._m44 =  Matrix_Determinant(_m11, _m12, _m13, _m21, _m22, _m23, _m31, _m32, _m33) * inv_det;

	return r;
}


bool RMatrix4::HasNan() const
{
	for (int i = 0; i < 16; i++)
	{
		if (isnan(arr[i]))
		{
			return true;
		}
	}

	return false;
}

std::string RMatrix4::ToDisplayString() const
{
	std::ostringstream StringStream;
	StringStream.precision(2);			// "%.2f"
	StringStream << std::fixed;			// No scientific notations
	for (int x = 0; x < 4; x++)
	{
		StringStream << "[";
		for (int y = 0; y < 4; y++)
		{
			StringStream << std::setw(10) << m[x][y];
		}
		StringStream << "]" << std::endl;
	}
	return StringStream.str();
}

RMatrix4 RMatrix4::CreateXAxisRotation(float degree)
{
	RMatrix4 mat = RMatrix4::IDENTITY;
	mat.m[1][1] = cosf(DEG_TO_RAD(degree));
	mat.m[1][2] = sinf(DEG_TO_RAD(degree));
	mat.m[2][1] = -sinf(DEG_TO_RAD(degree));
	mat.m[2][2] = cosf(DEG_TO_RAD(degree));

	return mat;
}

RMatrix4 RMatrix4::CreateYAxisRotation(float degree)
{
	RMatrix4 mat = RMatrix4::IDENTITY;
	mat.m[0][0] = cosf(DEG_TO_RAD(degree));
	mat.m[0][2] = -sinf(DEG_TO_RAD(degree));
	mat.m[2][0] = sinf(DEG_TO_RAD(degree));
	mat.m[2][2] = cosf(DEG_TO_RAD(degree));

	return mat;
}

RMatrix4 RMatrix4::CreateZAxisRotation(float degree)
{
	RMatrix4 mat = RMatrix4::IDENTITY;
	mat.m[0][0] = cosf(DEG_TO_RAD(degree));
	mat.m[0][1] = sinf(DEG_TO_RAD(degree));
	mat.m[1][0] = -sinf(DEG_TO_RAD(degree));
	mat.m[1][1] = cosf(DEG_TO_RAD(degree));

	return mat;
}

RMatrix4 RMatrix4::CreatePerspectiveProjectionLH(float fov, float aspect, float zNear, float zFar)
{
	float y_scale = 1.0f / tanf(0.5f * DEG_TO_RAD(fov));

	return RMatrix4(y_scale / aspect,	0.0f,		0.0f,								0.0f,
					0.0f,				y_scale,	0.0f,								0.0f,
					0.0f,				0.0f,		zFar / (zFar - zNear),				1.0f,
					0.0f,				0.0f,		-(zFar * zNear) / (zFar - zNear),	0.0f);
}

RMatrix4 RMatrix4::CreateOrthographicProjectionLH(float viewWidth, float viewHeight, float zNear, float zFar)
{
	float fRange = 1.0f / (zFar - zNear);

	return RMatrix4(2.0f / viewWidth,	0.0f,				0.0f,				0.0f,
					0.0f,				2.0f / viewHeight,	0.0f,				0.0f,
					0.0f,				0.0f,				fRange,				0.0f,
					0.0f,				0.0f,				-fRange * zNear,	1.0f);
}

RMatrix4 RMatrix4::CreateLookAtViewLH(const RVec3& eye, const RVec3& lookAt, const RVec3& up)
{
	RVec3 zaxis = (lookAt - eye).GetNormalized();
	RVec3 xaxis = RVec3::Cross(up, zaxis).GetNormalized();
	RVec3 yaxis = RVec3::Cross(zaxis, xaxis);
	RVec3 pos(-RVec3::Dot(xaxis, eye), -RVec3::Dot(yaxis, eye), -RVec3::Dot(zaxis, eye));

	return RMatrix4(xaxis.X(),	yaxis.X(),	zaxis.X(),	0,
					xaxis.Y(),	yaxis.Y(),	zaxis.Y(),	0,
					xaxis.Z(),	yaxis.Z(),	zaxis.Z(),	0,
					pos.X(),	pos.Y(),	pos.Z(),	1);
}

RMatrix4 RMatrix4::Lerp(const RMatrix4& lhs, const RMatrix4& rhs, float t)
{
	RMatrix4 result;

	for (int j = 0; j < 16; j++)
	{
		float va = ((float*)&lhs)[j];
		float vb = ((float*)&rhs)[j];
		((float*)&result)[j] = va + (vb - va) * t;
	}

	return result;
}

RMatrix4 RMatrix4::Slerp(const RMatrix4& lhs, const RMatrix4& rhs, float t)
{
	RVec3 Position1, Position2;
	RQuat Rotation1, Rotation2;
	RVec3 Scale1, Scale2;

	lhs.Decompose(Position1, Rotation1, Scale1);
	rhs.Decompose(Position2, Rotation2, Scale2);

	return CreateTransform(
		RVec3::Lerp(Position1, Position2, t),
		RQuat::Slerp(Rotation1, Rotation2, t),
		RVec3::Lerp(Scale1, Scale2, t)
	);
}

RVec4 operator*(const RVec4& v, const RMatrix4& m)
{
	RVec4 v_result = v;
	v_result.x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0] + v.w * m.m[3][0];
	v_result.y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1] + v.w * m.m[3][1];
	v_result.z = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] + v.w * m.m[3][2];
	v_result.w = v.x * m.m[0][3] + v.y * m.m[1][3] + v.z * m.m[2][3] + v.w * m.m[3][3];
	return v_result;
}
