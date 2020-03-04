//=============================================================================
// FTGPlayerController.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "FTGPlayerController.h"

IMPLEMENT_SCENE_OBJECT(FTGPlayerController);

bool FTGPlayerController::DrawDebugHitShape = false;

FTGPlayerController::FTGPlayerController(const RConstructingParams& Params)
	: Base(Params)
{
}

FTGPlayerController::~FTGPlayerController()
{
}

void FTGPlayerController::PerformPunch()
{
	if (GetBehaviorId() == PlayerBehavior_Idle::StaticClassId() ||
		GetBehaviorId() == PlayerBehavior_Run::StaticClassId())
	{
		SetBehavior(BHV_Punch);
	}
}

void FTGPlayerController::PerformKick()
{
	if (GetBehavior() == BHV_Idle || GetBehavior() == BHV_Run || GetBehavior() == BHV_Kick)
	{
		if (GetBehavior() == BHV_Kick)
		{
			SetBehavior(BHV_BackKick);
		}
		else
		{
			SetBehavior(BHV_Kick);
		}
	}
}

void FTGPlayerController::PerformSpinAttack()
{
	if (GetBehavior() == BHV_Idle || GetBehavior() == BHV_Run)
	{
		SetBehavior(BHV_SpinAttack);
	}
}

void FTGPlayerController::AddHitPlayerController(FTGPlayerController* HitTarget)
{
	HitPlayerControllers.push_back(HitTarget);
}

bool FTGPlayerController::HasHitPlayerController(FTGPlayerController* HitTarget) const
{
	for (auto PlayerController : HitPlayerControllers)
	{
		if (PlayerController == HitTarget)
		{
			return true;
		}
	}

	return false;
}

void FTGPlayerController::ClearHitPlayerControllers()
{
	HitPlayerControllers.clear();
}

std::vector<FTGPlayerController*> FTGPlayerController::TestSphereHitWithOtherPlayers(float Radius, const RVec3& LocalSpaceOffset)
{
	RSphere HitSphere;
	HitSphere.center = GetTransform()->GetTranslatedVector(LocalSpaceOffset, ETransformSpace::Local);
	HitSphere.radius = Radius;

	if (DrawDebugHitShape)
	{
		GDebugRenderer.DrawSphere(HitSphere.center, HitSphere.radius);
	}

	std::vector<FTGPlayerController*> Results;
	for (auto PlayerController : m_Scene->FindAllObjectsOfType<FTGPlayerController>())
	{
		if (PlayerController == this)
		{
			continue;
		}

		if (RCollision::TestSphereWithCapsule(HitSphere, PlayerController->GetCollisionShape()))
		{
			Results.push_back(PlayerController);
		}
	}

	return Results;
}
