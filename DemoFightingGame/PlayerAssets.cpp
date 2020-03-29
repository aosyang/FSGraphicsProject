//=============================================================================
// PlayerAssets.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "PlayerAssets.h"

#include "PlayerControllerBase.h"
#include "PlayerBehavior_Navigation.h"

void InitializePlayerAsset_UnityChan(PlayerControllerBase* PlayerController)
{
	FTGPlayerStateMachine& StateMachine = PlayerController->GetStateMachine();
	StateMachine.AllocateBehaviorInstance<PlayerBehavior_Idle>("/unitychan/FUCM05_0000_Idle.fbx", AnimBitFlag_Loop);
	StateMachine.AllocateBehaviorInstance<PlayerBehavior_Run>("/unitychan/FUCM_0012b_EH_RUN_LP_NoZ.fbx", AnimBitFlag_Loop);
	StateMachine.AllocateBehaviorInstance<PlayerBehavior_Punch>("/unitychan/FUCM05_0001_M_CMN_LJAB.fbx", AnimBitFlag_HasRootMotion);
	StateMachine.AllocateBehaviorInstance<PlayerBehavior_Kick>("/unitychan/FUCM_04_0001_RHiKick.fbx", AnimBitFlag_HasRootMotion);
	StateMachine.AllocateBehaviorInstance<PlayerBehavior_BackKick>("/unitychan/FUCM02_0004_CH01_AS_MAWAK.fbx", AnimBitFlag_HasRootMotion);
	StateMachine.AllocateBehaviorInstance<PlayerBehavior_SpinAttack>("/unitychan/FUCM02_0029_Cha01_STL01_ScrewK01.fbx", AnimBitFlag_HasRootMotion);
	StateMachine.AllocateBehaviorInstance<PlayerBehavior_Hit>("/unitychan/unitychan_DAMAGED00.fbx", AnimBitFlag_HasRootMotion);
	StateMachine.AllocateBehaviorInstance<PlayerBehavior_KnockedDown>("/unitychan/FUCM02_0025_MYA_TF_DOWN.fbx", AnimBitFlag_HasRootMotion);
	StateMachine.AllocateBehaviorInstance<PlayerBehavior_GetUp>("/unitychan/FUCM03_0019_HeadSpring.fbx", AnimBitFlag_HasRootMotion);

	PlayerController->InitAssets("/unitychan/unitychan.fbx");
}

void InitializePlayerAsset_HumanBody(PlayerControllerBase* PlayerController)
{
	FTGPlayerStateMachine& StateMachine = PlayerController->GetStateMachine();
	StateMachine.AllocateBehaviorInstance<PlayerBehavior_Idle>("/HumanBody/Animations/Idle.fbx", AnimBitFlag_Loop);
	StateMachine.AllocateBehaviorInstance<PlayerBehavior_Walk>("/HumanBody/Animations/Walking.fbx", AnimBitFlag_Loop);
	StateMachine.AllocateBehaviorInstance<PlayerBehavior_Run>("/HumanBody/Animations/Running.fbx", AnimBitFlag_Loop);

	// Initialize the skinning mesh
	PlayerController->InitAssets("/HumanBody/HumanBody_DefaultPose.fbx");
}

void InitializePlayerAsset_Maid(PlayerControllerBase* PlayerController)
{
	FTGPlayerStateMachine& StateMachine = PlayerController->GetStateMachine();
	PlayerBehavior_Navigation* NavigationBehavior = StateMachine.AllocateBehaviorInstance<PlayerBehavior_Navigation>();

	static char* NavAnimNames[] = {
		"/Maid/Maid_Idle.fbx",
		"/Maid/Maid_Walk.fbx",
		"/Maid/Maid_Run.fbx",
	};

	// Cache animation by the mesh. 
	// TODO: Make this process less redundant
	RMesh* PlayerMesh = RResourceManager::Instance().LoadResource<RMesh>("/Maid/Maid.fbx", EResourceLoadMode::Immediate);
	if (PlayerMesh)
	{
		for (int i = 0; i < ARRAYSIZE(NavAnimNames); i++)
		{
			RMesh* AnimMesh = RResourceManager::Instance().LoadResource<RMesh>(NavAnimNames[i], EResourceLoadMode::Immediate);
			RAnimation* Animation = AnimMesh ? AnimMesh->GetAnimation() : nullptr;
			if (Animation)
			{
				PlayerMesh->CacheAnimation(Animation);
				NavigationBehavior->AddAnimation(NavAnimNames[i], Animation->GetRootSpeed());
			}
		}
	}

	// Initialize the skinning mesh
	PlayerController->InitAssets("/Maid/Maid.fbx");
}
