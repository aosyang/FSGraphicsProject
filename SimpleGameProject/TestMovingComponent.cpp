//=============================================================================
// TestMovingComponent.cpp by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#include "TestMovingComponent.h"

TestMovingComponent::TestMovingComponent(RSceneObject* InOwner)
	: Base(InOwner)
{

}

TestMovingComponent* TestMovingComponent::Create(RSceneObject* InOwner)
{
	TestMovingComponent* NewComponent = new TestMovingComponent(InOwner);
	return NewComponent;
}

void TestMovingComponent::Update()
{
	RVec3 Position = GetOwner()->GetPosition();
	float y = Position.Y();
	y += GEngine.GetTimer().DeltaTime() * 500.0f;

	if (y >= 500.0f)
	{
		y -= 1000.0f;
	}

	Position.SetY(y);

	float s = (y + 500.0f) / 1000.0f * 5.0f;
	GetOwner()->SetScale(RVec3(s, s, s));

	GetOwner()->SetPosition(Position);
}
