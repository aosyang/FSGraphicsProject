//=============================================================================
// RMatrix.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#ifndef _RMATRIX_H
#define _RMATRIX_H

#include "RVector.h"
#include <math.h>

class RMatrix4
{
public:
	union
	{
		float m[4][4];
		struct {
			float _m11, _m12, _m13, _m14,
				  _m21, _m22, _m23, _m24,
				  _m31, _m32, _m33, _m34,
				  _m41, _m42, _m43, _m44;
		};
	};

	RMatrix4();

	RMatrix4(float m11, float m12, float m13, float m14,
			 float m21, float m22, float m23, float m24,
			 float m31, float m32, float m33, float m34, 
			 float m41, float m42, float m43, float m44);

	RMatrix4& operator=(const RMatrix4& rhs);

	RMatrix4 operator*(const RMatrix4& rhs);

	void SetIdentity();

	// Fast inverse a camera matrix
	RMatrix4 GetViewMatrix() const;

	RVec3 GetForward() const;
	RVec3 GetUp() const;
	RVec3 GetRight() const;

	void TranslateLocal(const RVec3& vec);
	void TranslateLocal(float x, float y, float z);

	void SetTranslation(const RVec3& vec);
	void SetTranslation(float x, float y, float z);
	RVec3 GetTranslation() const;
	void GetTranslation(float& x, float& y, float& z) const;

	static RMatrix4 CreateXAxisRotation(float degree);
	static RMatrix4 CreateYAxisRotation(float degree);
	static RMatrix4 CreateZAxisRotation(float degree);

	static RMatrix4 CreateTranslation(const RVec3& vec);
	static RMatrix4 CreateTranslation(float x, float y, float z);

	static RMatrix4 CreateScale(const RVec3& scale);
	static RMatrix4 CreateScale(float sx, float sy, float sz);

	static RMatrix4 CreatePerspectiveProjection(float fov, float aspect, float zNear, float zFar);
};

RVec4 operator*(const RVec4& v, const RMatrix4& m);

#endif
