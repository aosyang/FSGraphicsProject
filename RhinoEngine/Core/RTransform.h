//=============================================================================
// RTransform.h by Shiyang Ao, 2017 All Rights Reserved.
//
// Transform class
//=============================================================================
#pragma once

#include "RVector.h"
#include "RQuat.h"

class RTransform
{
public:
	RTransform();
	RTransform(const RTransform& rhs);
	RTransform(const RVec3& InPosition, const RQuat& InRotation, const RVec3& InScale = RVec3(1, 1, 1));

	RTransform& operator=(const RTransform& rhs);

	void SetPosition(const RVec3& InPosition) { Position = InPosition; }
	void SetRotation(const RQuat& InRotation) { Rotation = InRotation; }
	void SetScale(const RVec3& InScale) { Scale = InScale; }
	const RVec3& GetPosition() const { return Position; }
	const RQuat& GetRotation() const { return Rotation; }
	const RVec3& GetScale() const { return Scale; }

	RMatrix4 GetMatrix() const;

	RVec3 GetForward() const;
	RVec3 GetUp() const;
	RVec3 GetRight() const;

	void Translate(const RVec3& t);
	void TranslateLocal(const RVec3& t);

	void LookAt(const RVec3& target, const RVec3& world_up = RVec3(0, 1, 0));

	static RTransform IDENTITY;

private:
	RVec3 Position;
	RQuat Rotation;
	RVec3 Scale;
};

FORCEINLINE RVec3 RTransform::GetForward() const
{
	return Rotation * RVec3(0, 0, 1);
}

FORCEINLINE RVec3 RTransform::GetUp() const
{
	return Rotation * RVec3(0, 1, 0);
}

FORCEINLINE RVec3 RTransform::GetRight() const
{
	return Rotation * RVec3(1, 0, 0);
}

FORCEINLINE void RTransform::Translate(const RVec3& t)
{
	Position += t;
}

FORCEINLINE void RTransform::TranslateLocal(const RVec3& t)
{
	Position += Rotation * t;
}
