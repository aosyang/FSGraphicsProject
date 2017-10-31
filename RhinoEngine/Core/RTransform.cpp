//=============================================================================
// RTransform.cpp by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"
#include "RTransform.h"

RTransform RTransform::IDENTITY = RTransform();

RTransform::RTransform()
	: RTransform(RVec3(0, 0, 0), RQuat::IDENTITY, RVec3(1, 1, 1))
{

}

RTransform::RTransform(const RVec3& InPosition, const RQuat& InRotation, const RVec3& InScale /*= RVec3(1, 1, 1)*/)
	: Position(InPosition),
	  Rotation(InRotation),
	  Scale(InScale),
	  Parent(nullptr),
	  bIsCachedMatrixDirty(true)
{

}

RTransform::RTransform(const RTransform& rhs)
	: RTransform(rhs.Position, rhs.Rotation, rhs.Scale)
{

}

RTransform& RTransform::operator=(const RTransform& rhs)
{
	if (this != &rhs)
	{
		Position = rhs.Position;
		Rotation = rhs.Rotation;
		Scale = rhs.Scale;
		Parent = rhs.Parent;
		bIsCachedMatrixDirty = true;
	}

	return *this;
}

const RMatrix4& RTransform::GetMatrix()
{
	if (bIsCachedMatrixDirty)
	{
		CachedMatrix = RMatrix4::IDENTITY;
		RMatrix3 rot_scale = Rotation.GetRotationMatrix() * RMatrix3::CreateScale(Scale);
		CachedMatrix.SetRotation(rot_scale);
		CachedMatrix.SetTranslation(Position);

		if (Parent)
		{
			CachedMatrix *= Parent->GetMatrix();
		}
		NotifyChildrenMatricesChanged();

		bIsCachedMatrixDirty = false;
	}

	return CachedMatrix;
}

bool RTransform::FromMatrix4(const RMatrix4& Matrix)
{
	bIsCachedMatrixDirty = true;
	return Matrix.Decompose(Position, Rotation, Scale);
}

void RTransform::LookAt(const RVec3& target, const RVec3& world_up /*= RVec3(0, 1, 0)*/)
{
	const RVec3& pos = Position;
	RVec3 forward = target - pos;
	forward.Normalize();
	RVec3 right = RVec3::Cross(world_up, forward);
	right.Normalize();
	RVec3 up = RVec3::Cross(forward, right);

	RMatrix4 m;
	m.SetRow(0, RVec4(right, 0));
	m.SetRow(1, RVec4(up, 0));
	m.SetRow(2, RVec4(forward, 0));
	m.SetRow(3, RVec4(pos, 1));

	m.Decompose(Position, Rotation, Scale);

	bIsCachedMatrixDirty = true;
}

void RTransform::Attach(RTransform* NodeParent)
{
	assert(Parent == nullptr);
	Parent = NodeParent;

	// Assume transform is not in parent's child list already
	assert(find(Parent->Children.begin(), Parent->Children.end(), this) == Parent->Children.end());
	Parent->Children.push_back(this);
	bIsCachedMatrixDirty = true;
}

void RTransform::Detach()
{
	auto Iter = find(Parent->Children.begin(), Parent->Children.end(), this);
	if (Iter != Parent->Children.end())
	{
		Parent->Children.erase(Iter);
	}
	Parent = nullptr;
	bIsCachedMatrixDirty = true;
}

RTransform* RTransform::GetParent() const
{
	return Parent;
}

void RTransform::NotifyChildrenMatricesChanged()
{
	for (auto Iter : Children)
	{
		Iter->bIsCachedMatrixDirty = true;
		Iter->NotifyChildrenMatricesChanged();
	}
}

RTransform RTransform::Combine(RTransform* lhs, RTransform* rhs)
{
	// TODO: Implement this without decompose
	RTransform Result;
	Result.FromMatrix4(lhs->GetMatrix() * rhs->GetMatrix());
	return Result;
}
