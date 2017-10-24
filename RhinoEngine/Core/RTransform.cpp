//=============================================================================
// RTransform.cpp by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"
#include "RTransform.h"

RTransform RTransform::IDENTITY = RTransform();

RTransform::RTransform()
	: Position(0, 0, 0),
	  Rotation(RQuat::IDENTITY),
	  Scale(1, 1, 1)
{

}

RTransform::RTransform(const RVec3& InPosition, const RQuat& InRotation, const RVec3& InScale /*= RVec3(1, 1, 1)*/)
	: Position(InPosition),
	  Rotation(InRotation),
	  Scale(InScale)
{

}

RTransform::RTransform(const RTransform& rhs)
	: Position(rhs.Position),
	  Rotation(rhs.Rotation),
	  Scale(rhs.Scale)
{

}

RTransform& RTransform::operator=(const RTransform& rhs)
{
	if (this != &rhs)
	{
		Position = rhs.Position;
		Rotation = rhs.Rotation;
		Scale = rhs.Scale;
	}

	return *this;
}

RMatrix4 RTransform::GetMatrix() const
{
	RMatrix4 mat(RMatrix4::IDENTITY);
	RMatrix3 rot_scale = Rotation.GetRotationMatrix() * RMatrix3::CreateScale(Scale);
	mat.SetRotation(rot_scale);
	mat.SetTranslation(Position);

	return mat;
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
}
