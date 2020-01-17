//=============================================================================
// FTGPlayerController.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Rhino.h"

#include "PlayerControllerBase.h"

/// Base fighting game player controller
class FTGPlayerController : public PlayerControllerBase
{
	DECLARE_SCENE_OBJECT(FTGPlayerController, PlayerControllerBase);
public:
	~FTGPlayerController();

	void PerformPunch();
	void PerformKick();
	void PerformSpinAttack();

	void AddHitPlayerController(FTGPlayerController* HitTarget);
	bool HasHitPlayerController(FTGPlayerController* HitTarget) const;
	void ClearHitPlayerControllers();

	/// Run a sphere shape hit test with other players
	std::vector<FTGPlayerController*> TestSphereHitWithOtherPlayers(float Radius, const RVec3& LocalSpaceOffset);

	/// Whether to draw debug shape that represents player's hit
	static bool DrawDebugHitShape;

private:
	FTGPlayerController(const RConstructingParams& Params);

	std::vector<FTGPlayerController*> HitPlayerControllers;
};

