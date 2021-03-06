//=============================================================================
// RMatrix.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "RVector.h"
#include "RQuat.h"

#include <math.h>

//////////////////////////////////////////////////////////////////////////
// Matrix3 class
//////////////////////////////////////////////////////////////////////////
class RMatrix3
{
public:
	union
	{
		float m[3][3];
		struct {
			float _m11, _m12, _m13,
				  _m21, _m22, _m23,
				  _m31, _m32, _m33;
		};
	};

	RMatrix3();
	RMatrix3(float Array[9]);
	RMatrix3(float m11, float m12, float m13, float m21, float m22, float m23, float m31, float m32, float m33);

	RMatrix3& operator=(const RMatrix3& rhs);

	RMatrix3 operator*(const RMatrix3& rhs) const;
	RMatrix3& operator*=(const RMatrix3& rhs);

	/// Extract rotation and scale from matrix
	bool Decompose(RQuat& Rotation, RVec3& Scale) const;

	static RMatrix3 CreateScale(const RVec3& scale);
	static RMatrix3 CreateScale(float sx, float sy, float sz);
};

FORCEINLINE RMatrix3 RMatrix3::CreateScale(const RVec3& scale)
{
	return CreateScale(scale.X(), scale.Y(), scale.Z());
}

FORCEINLINE RMatrix3 RMatrix3::CreateScale(float sx, float sy, float sz)
{
	return RMatrix3(sx, 0, 0, 0, sy, 0, 0, 0, sz);
}

//////////////////////////////////////////////////////////////////////////
// Matrix4 class
//////////////////////////////////////////////////////////////////////////
class RMatrix4
{
public:
	union
	{
		float arr[16];
		float m[4][4];
		struct {
			float _m11, _m12, _m13, _m14,
				  _m21, _m22, _m23, _m24,
				  _m31, _m32, _m33, _m34,
				  _m41, _m42, _m43, _m44;
		};
	};

	RMatrix4();
	RMatrix4(float Array[16]);
	RMatrix4(float m11, float m12, float m13, float m14,
			 float m21, float m22, float m23, float m24,
			 float m31, float m32, float m33, float m34, 
			 float m41, float m42, float m43, float m44);

	RMatrix4& operator=(const RMatrix4& rhs);

	RMatrix4 operator*(const RMatrix4& rhs) const;
	RMatrix4& operator*=(const RMatrix4& rhs);

	// Fast inverse a homogeneous transformation matrix
	RMatrix4 FastInverse() const;

	RVec3 GetForward() const;
	RVec3 GetUp() const;
	RVec3 GetRight() const;

	RVec4 GetRow(int index) const;
	const float* GetRowArray(int index) const;
	void SetRow(int index, const RVec4& row);

	void SetRotation(const RMatrix3& rot);
	RMatrix3 GetRotationMatrix() const;

	/// Extract position, rotation and scale from matrix
	bool Decompose(RVec3& Position, RQuat& Rotaion, RVec3& Scale) const;

	void Translate(const RVec3& vec);
	void Translate(float x, float y, float z);
	void TranslateLocal(const RVec3& vec);
	void TranslateLocal(float x, float y, float z);

	void SetTranslation(const RVec3& vec);
	void SetTranslation(float x, float y, float z);
	RVec3 GetTranslation() const;
	void GetTranslation(float& x, float& y, float& z) const;

	RVec3 RotateVector(const RVec3& vec) const;
	RVec3 Transform(const RVec3& vec) const;

	RMatrix4 Inverse() const;

	bool HasNan() const;

	// Convert the matrix to a string for display
	std::string ToDisplayString() const;

	// Identity matrix variable
	static RMatrix4 IDENTITY;
	static RMatrix4 Zero;

	// Rotation matrix
	static RMatrix4 CreateXAxisRotation(float degree);
	static RMatrix4 CreateYAxisRotation(float degree);
	static RMatrix4 CreateZAxisRotation(float degree);

	static RMatrix4 CreateTranslation(const RVec3& vec);
	static RMatrix4 CreateTranslation(float x, float y, float z);

	static RMatrix4 CreateScale(const RVec3& scale);
	static RMatrix4 CreateScale(float sx, float sy, float sz);

	static RMatrix4 CreateTransform(const RVec3& Position, const RQuat& Rotation, const RVec3& Scale = RVec3(1.0f, 1.0f, 1.0f));

	static RMatrix4 CreatePerspectiveProjectionLH(float fov, float aspect, float zNear, float zFar);
	static RMatrix4 CreateOrthographicProjectionLH(float viewWidth, float viewHeight, float zNear, float zFar);
	static RMatrix4 CreateLookAtViewLH(const RVec3& eye, const RVec3& lookAt, const RVec3& up);

	// Linear interpolates on two matrices
	static RMatrix4 Lerp(const RMatrix4& lhs, const RMatrix4& rhs, float t);

	// Spherical linear interpolates two matrices (Better handled rotation than lerp)
	static RMatrix4 Slerp(const RMatrix4& lhs, const RMatrix4& rhs, float t);
};

RVec4 operator*(const RVec4& v, const RMatrix4& m);

FORCEINLINE RVec3 RMatrix4::GetForward() const
{
	return RVec3(m[2][0], m[2][1], m[2][2]);
}

FORCEINLINE RVec3 RMatrix4::GetUp() const
{
	return RVec3(m[1][0], m[1][1], m[1][2]);
}

FORCEINLINE RVec3 RMatrix4::GetRight() const
{
	return RVec3(m[0][0], m[0][1], m[0][2]);
}

FORCEINLINE RVec4 RMatrix4::GetRow(int index) const
{
	return RVec4(m[index]);
}

FORCEINLINE const float* RMatrix4::GetRowArray(int index) const
{
	return m[index];
}

FORCEINLINE void RMatrix4::SetRow(int index, const RVec4& row)
{
	m[index][0] = row.x;
	m[index][1] = row.y;
	m[index][2] = row.z;
	m[index][3] = row.w;
}

FORCEINLINE void RMatrix4::SetRotation(const RMatrix3& rot)
{
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			m[i][j] = rot.m[i][j];
}

FORCEINLINE RMatrix3 RMatrix4::GetRotationMatrix() const
{
	return RMatrix3(
		_m11, _m12, _m13,
		_m21, _m22, _m23,
		_m31, _m32, _m33
	);
}

FORCEINLINE void RMatrix4::Translate(const RVec3& vec)
{
	Translate(vec.X(), vec.Y(), vec.Z());
}

FORCEINLINE void RMatrix4::Translate(float x, float y, float z)
{
	m[3][0] += x;
	m[3][1] += y;
	m[3][2] += z;
}

FORCEINLINE void RMatrix4::TranslateLocal(const RVec3& vec)
{
	TranslateLocal(vec.X(), vec.Y(), vec.Z());
}

FORCEINLINE void RMatrix4::TranslateLocal(float x, float y, float z)
{
	float _x = m[3][0] + x * m[0][0] + y * m[1][0] + z * m[2][0];
	float _y = m[3][1] + x * m[0][1] + y * m[1][1] + z * m[2][1];
	float _z = m[3][2] + x * m[0][2] + y * m[1][2] + z * m[2][2];

	m[3][0] = _x;
	m[3][1] = _y;
	m[3][2] = _z;
}

FORCEINLINE void RMatrix4::SetTranslation(const RVec3& vec)
{
	SetTranslation(vec.X(), vec.Y(), vec.Z());
}

FORCEINLINE void RMatrix4::SetTranslation(float x, float y, float z)
{
	m[3][0] = x;
	m[3][1] = y;
	m[3][2] = z;
}

FORCEINLINE RVec3 RMatrix4::GetTranslation() const
{
	return RVec3(m[3][0], m[3][1], m[3][2]);
}

FORCEINLINE void RMatrix4::GetTranslation(float& x, float& y, float& z) const
{
	x = m[3][0];
	y = m[3][1];
	z = m[3][2];
}

FORCEINLINE RMatrix4 RMatrix4::CreateTranslation(const RVec3& vec)
{
	return CreateTranslation(vec.X(), vec.Y(), vec.Z());
}

FORCEINLINE RMatrix4 RMatrix4::CreateTranslation(float x, float y, float z)
{
	RMatrix4 mat = RMatrix4::IDENTITY;
	mat.m[3][0] = x;
	mat.m[3][1] = y;
	mat.m[3][2] = z;

	return mat;
}

FORCEINLINE RMatrix4 RMatrix4::CreateScale(const RVec3& scale)
{
	return CreateScale(scale.X(), scale.Y(), scale.Z());
}

FORCEINLINE RMatrix4 RMatrix4::CreateScale(float sx, float sy, float sz)
{
	RMatrix4 mat = RMatrix4::IDENTITY;
	mat.m[0][0] = sx;
	mat.m[1][1] = sy;
	mat.m[2][2] = sz;

	return mat;
}

FORCEINLINE RMatrix4 RMatrix4::CreateTransform(const RVec3& Position, const RQuat& Rotation, const RVec3& Scale /*= RVec3(1.0f, 1.0f, 1.0f)*/)
{
	RMatrix4 TransformMatrix = RMatrix4::IDENTITY;
	const RMatrix3 ScaleRotation = RMatrix3::CreateScale(Scale) * Rotation.GetRotationMatrix();
	TransformMatrix.SetRotation(ScaleRotation);
	TransformMatrix.SetTranslation(Position);

	return TransformMatrix;
}

