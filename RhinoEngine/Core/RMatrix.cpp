//=============================================================================
// RMatrix.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"
#include "RMatrix.h"

RMatrix4 RMatrix4::IDENTITY = RMatrix4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);

RMatrix4::RMatrix4()
{
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

RMatrix4 RMatrix4::operator*(const RMatrix4& rhs)
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

RMatrix4 RMatrix4::GetViewMatrix() const
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

	return view;
}

RVec3 RMatrix4::GetForward() const
{
	return -RVec3(m[2][0], m[2][1], m[2][2]);
}

RVec3 RMatrix4::GetUp() const
{
	return RVec3(m[1][0], m[1][1], m[1][2]);
}

RVec3 RMatrix4::GetRight() const
{
	return RVec3(m[0][0], m[0][1], m[0][2]);
}

RVec4 RMatrix4::GetRow(int index)
{
	return RVec4(m[index]);
}

void RMatrix4::SetRow(int index, const RVec4& row)
{
	m[index][0] = row.x;
	m[index][1] = row.y;
	m[index][2] = row.z;
	m[index][3] = row.w;
}

void RMatrix4::TranslateLocal(const RVec3& vec)
{
	TranslateLocal(vec.x, vec.y, vec.z);
}

void RMatrix4::TranslateLocal(float x, float y, float z)
{
	float _x = m[3][0] + x * m[0][0] + y * m[1][0] + z * m[2][0];
	float _y = m[3][1] + x * m[0][1] + y * m[1][1] + z * m[2][1];
	float _z = m[3][2] + x * m[0][2] + y * m[1][2] + z * m[2][2];

	m[3][0] = _x;
	m[3][1] = _y;
	m[3][2] = _z;
}

void RMatrix4::SetTranslation(const RVec3& vec)
{
	SetTranslation(vec.x, vec.y, vec.z);
}

void RMatrix4::SetTranslation(float x, float y, float z)
{
	m[3][0] = x;
	m[3][1] = y;
	m[3][2] = z;
}

RVec3 RMatrix4::GetTranslation() const
{
	return RVec3(m[3][0], m[3][1], m[3][2]);
}

void RMatrix4::GetTranslation(float& x, float& y, float& z) const
{
	x = m[3][0];
	y = m[3][1];
	z = m[3][2];
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

RMatrix4 RMatrix4::CreateTranslation(const RVec3& vec)
{
	return CreateTranslation(vec.x, vec.y, vec.z);
}

RMatrix4 RMatrix4::CreateTranslation(float x, float y, float z)
{
	RMatrix4 mat = RMatrix4::IDENTITY;
	mat.m[3][0] = x;
	mat.m[3][1] = y;
	mat.m[3][2] = z;

	return mat;
}

RMatrix4 RMatrix4::CreateScale(const RVec3& scale)
{
	return CreateScale(scale.x, scale.y, scale.z);
}

RMatrix4 RMatrix4::CreateScale(float sx, float sy, float sz)
{
	RMatrix4 mat = RMatrix4::IDENTITY;
	mat.m[0][0] = sx;
	mat.m[1][1] = sy;
	mat.m[2][2] = sz;

	return mat;
}

RMatrix4 RMatrix4::CreatePerspectiveProjectionLH(float fov, float aspect, float zNear, float zFar)
{
	float y_scale = 1.0f / tanf(0.5f * DEG_TO_RAD(fov));

	return RMatrix4(y_scale,	0.0f,				0.0f,								0.0f,
					0.0f,		y_scale * aspect,	0.0f,								0.0f,
					0.0f,		0.0f,				zFar / (zFar - zNear),				1.0f,
					0.0f,		0.0f,				-(zFar * zNear) / (zFar - zNear),	0.0f);
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
	RVec3 zaxis = (lookAt - eye).GetNormalizedVec3();
	RVec3 xaxis = up.Cross(zaxis).GetNormalizedVec3();
	RVec3 yaxis = zaxis.Cross(xaxis);

	return RMatrix4(xaxis.x,			yaxis.x,			zaxis.x,			0,
					xaxis.y,			yaxis.y,			zaxis.y,			0,
					xaxis.z,			yaxis.z,			zaxis.z,			0,
					-xaxis.Dot(eye),	-yaxis.Dot(eye),	-zaxis.Dot(eye),	1);
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