//=============================================================================
// RTransform.h by Shiyang Ao, 2017 All Rights Reserved.
//
// Transform class
//=============================================================================
#pragma once

#include "RVector.h"
#include "RQuat.h"

enum class ETransformSpace : UINT8
{
	Local,
	World,
};

class RTransform
{
public:
	RTransform();
	RTransform(const RTransform& rhs);
	RTransform(const RVec3& InPosition, const RQuat& InRotation, const RVec3& InScale = RVec3(1, 1, 1));

	RTransform& operator=(const RTransform& rhs);

	void SetPosition(const RVec3& InPosition)	{ Position = InPosition; bIsCachedMatrixDirty = true; }
	void SetRotation(const RQuat& InRotation)	{ Rotation = InRotation; bIsCachedMatrixDirty = true; }
	void SetScale(const RVec3& InScale)			{ Scale = InScale; bIsCachedMatrixDirty = true; }
	const RVec3& GetPosition() const { return Position; }
	const RQuat& GetRotation() const { return Rotation; }
	const RVec3& GetScale() const { return Scale; }

	const RMatrix4& GetMatrix() const;
	bool FromMatrix4(const RMatrix4& Matrix);

	RVec3 GetForward() const;
	RVec3 GetUp() const;
	RVec3 GetRight() const;

	void Translate(const RVec3& t, ETransformSpace Space);
	RVec3 GetTranslatedVector(const RVec3& t, ETransformSpace Space) const;

	void LookAt(const RVec3& target, const RVec3& world_up = RVec3(0, 1, 0));

	void Attach(RTransform* NodeParent);
	void Detach();

	RTransform* GetParent() const;

	bool IsCacheDirty() const;
	void NotifyChildrenMatricesChanged() const;

	static RTransform Combine(RTransform* lhs, RTransform* rhs);

	static RTransform IDENTITY;

private:
	RVec3 Position;
	RQuat Rotation;
	RVec3 Scale;

	RTransform*	Parent;
	std::vector<RTransform*>	Children;

	mutable RMatrix4	CachedMatrix;
	mutable bool		bIsCachedMatrixDirty;
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

FORCEINLINE bool RTransform::IsCacheDirty() const
{
	return bIsCachedMatrixDirty;
}

