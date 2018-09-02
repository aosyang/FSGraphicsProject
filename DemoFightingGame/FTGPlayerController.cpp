//=============================================================================
// PlayerController.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "FTGPlayerController.h"

BehaviorInfo PlayerBehaviorInfo[] =
{
	{ PlayerAnim_Idle,			0.2f },
	{ PlayerAnim_Run,			0.2f },
	{ PlayerAnim_Punch1,		0.0f },
	{ PlayerAnim_Kick,			0.0f },
	{ PlayerAnim_BackKick,		0.0f },
	{ PlayerAnim_SpinAttack,	0.2f },
	{ PlayerAnim_Hit,			0.0f },
	{ PlayerAnim_Down,			0.0f },
	{ PlayerAnim_GetUp,			0.0f },
};

FTGPlayerController::FTGPlayerController(RScene* InScene)
	: Base(InScene), m_Rotation(0.0f), m_Behavior(BHV_Idle)
{

}

void FTGPlayerController::Cache()
{
	RMesh* playerMesh = RResourceManager::Instance().FindMesh("../Assets/unitychan/unitychan.fbx");
	if (!playerMesh)
		playerMesh = RResourceManager::Instance().LoadFbxMesh("../Assets/unitychan/unitychan.fbx", EResourceLoadMode::Immediate);
	SetMesh(playerMesh);

	m_Animations[PlayerAnim_Idle]		= LoadAnimation("../Assets/unitychan/FUCM05_0000_Idle.fbx",					AnimBitFlag_Loop);
	m_Animations[PlayerAnim_Run]		= LoadAnimation("../Assets/unitychan/FUCM_0012b_EH_RUN_LP_NoZ.fbx",			AnimBitFlag_Loop);
	m_Animations[PlayerAnim_Punch1]		= LoadAnimation("../Assets/unitychan/FUCM05_0001_M_CMN_LJAB.fbx",			AnimBitFlag_HasRootMotion);
	m_Animations[PlayerAnim_Kick]		= LoadAnimation("../Assets/unitychan/FUCM_04_0001_RHiKick.fbx",				AnimBitFlag_HasRootMotion);
	m_Animations[PlayerAnim_BackKick]	= LoadAnimation("../Assets/unitychan/FUCM02_0004_CH01_AS_MAWAK.fbx",		AnimBitFlag_HasRootMotion);
	m_Animations[PlayerAnim_SpinAttack] = LoadAnimation("../Assets/unitychan/FUCM02_0029_Cha01_STL01_ScrewK01.fbx",	AnimBitFlag_HasRootMotion);
	m_Animations[PlayerAnim_Hit]		= LoadAnimation("../Assets/unitychan/unitychan_DAMAGED00.fbx",				AnimBitFlag_HasRootMotion);
	m_Animations[PlayerAnim_Down]		= LoadAnimation("../Assets/unitychan/FUCM02_0025_MYA_TF_DOWN.fbx",			AnimBitFlag_HasRootMotion);
	m_Animations[PlayerAnim_GetUp]		= LoadAnimation("../Assets/unitychan/FUCM03_0019_HeadSpring.fbx",			AnimBitFlag_HasRootMotion);

	for (int i = 0; i < PlayerAnimCount; i++)
	{
		m_Mesh->CacheAnimation(m_Animations[i]);
	}

	m_AnimBlender.Play(m_Animations[PlayerAnim_Idle]);
}

void FTGPlayerController::PreUpdate(const RTimer& timer)
{
	m_AnimBlender.ProceedAnimation(timer.DeltaTime());

	m_RootOffset = m_AnimBlender.GetCurrentRootOffset();

	if (m_Behavior == BHV_Idle || m_Behavior == BHV_Running)
		m_RootOffset = RVec3(0, 0, 0);

	if (m_AnimBlender.IsAnimationDone())
	{
		if (m_Behavior == BHV_HitDown)
			SetBehavior(BHV_GetUp);
		else
			SetBehavior(BHV_Idle);
	}
}

float LerpDegreeAngle(float from, float to, float t)
{
	while (from - to > 180.0f)
	{
		from -= 360.0f;
	}

	while (from - to < -180.0f)
	{
		from += 360.0f;
	}

	return from + (to - from) * Math::Clamp(t, 0.0f, 1.0f);
}

void FTGPlayerController::UpdateMovement(const RTimer& timer, const RVec3 moveVec)
{
	bool bCanMovePlayer = CanMovePlayerWithInput();
	if (bCanMovePlayer)
	{
		RVec3 PlannarMoveVector = moveVec;
		PlannarMoveVector.SetY(0.0f);

		if (PlannarMoveVector.SquaredMagitude() > 0.0f)
		{
			PlannarMoveVector = PlannarMoveVector.GetNormalized();
			m_Rotation = LerpDegreeAngle(m_Rotation, RAD_TO_DEG(atan2f(-PlannarMoveVector.X(), -PlannarMoveVector.Z())), 10.0f * timer.DeltaTime());
		}
	}

	// Offset of stair which player can step up
	static const RVec3 StairOffset = RVec3(0, 10, 0);

	RAabb playerAabb = GetMovementCollisionShape();
	playerAabb.pMin += StairOffset;
	playerAabb.pMax += StairOffset;

	RVec3 worldMoveVec = bCanMovePlayer ? moveVec : RVec3::Zero();

	// Apply gravity
	worldMoveVec += RVec3(0, -1000.0f * timer.DeltaTime(), 0);

	worldMoveVec += (RVec4(GetRootOffset(), 0) * GetTransformMatrix()).ToVec3();
	worldMoveVec -= StairOffset;
	worldMoveVec = m_Scene->TestMovingAabbWithScene(playerAabb, worldMoveVec);

	Translate(worldMoveVec + StairOffset);

	SetRotation(RQuat::Euler(0.0f, DEG_TO_RAD(m_Rotation), 0.0f));
}

void FTGPlayerController::PostUpdate(const RTimer& timer)
{
	for (int i = 0; i < m_Mesh->GetBoneCount(); i++)
	{
		RMatrix4 matrix;

		int sourceBoneId = m_Mesh->GetCachedAnimationNodeId(m_AnimBlender.GetSourceAnimation(), i);
		int targetBondId = m_Mesh->GetCachedAnimationNodeId(m_AnimBlender.GetTargetAnimation(), i);
		m_AnimBlender.GetCurrentBlendedNodePose(sourceBoneId, targetBondId, &matrix);

		m_BoneMatrices[i] = m_Mesh->GetBoneInitInvMatrices(i) * matrix * GetTransformMatrix();
	}
}

void FTGPlayerController::Draw()
{
	RConstantBuffers::cbBoneMatrices.UpdateBufferData((SHADER_SKINNED_BUFFER*)&m_BoneMatrices);
	RConstantBuffers::cbBoneMatrices.BindBuffer();

	RSMeshObject::Draw();
}

void FTGPlayerController::DrawDepthPass()
{
	RConstantBuffers::cbBoneMatrices.UpdateBufferData((SHADER_SKINNED_BUFFER*)&m_BoneMatrices);
	RConstantBuffers::cbBoneMatrices.BindBuffer();

	RSMeshObject::DrawDepthPass();
}

void FTGPlayerController::SetBehavior(EPlayerBehavior behavior)
{
	switch (behavior)
	{
	case BHV_Kick:
		if (m_Behavior != BHV_Kick && m_Behavior != BHV_BackKick)
		{
			m_AnimBlender.Play(m_Animations[PlayerAnim_Kick]);
			m_Behavior = BHV_Kick;
		}
		else if (m_Behavior == BHV_Kick && GetBehaviorTime() >= 0.5f)
		{
			m_AnimBlender.Play(m_Animations[PlayerAnim_BackKick]);
			m_Behavior = BHV_BackKick;
		}
		break;

	case BHV_Hit:
		m_AnimBlender.Play(m_Animations[PlayerAnim_Hit]);
		m_Behavior = BHV_Hit;
		break;

	default:
		if (m_Behavior != behavior)
		{
			const BehaviorInfo& CurrentBehavior = PlayerBehaviorInfo[behavior];

			if (CurrentBehavior.blendTime > 0.0f)
			{
				m_AnimBlender.BlendTo(m_Animations[CurrentBehavior.anim],
									  m_Animations[CurrentBehavior.anim]->GetStartTime(), 1.0f,
									  CurrentBehavior.blendTime);
			}
			else
			{
				m_AnimBlender.Play(m_Animations[CurrentBehavior.anim]);
			}
			m_Behavior = behavior;
		}
	}
}

float FTGPlayerController::GetBehaviorTime()
{
	if (m_AnimBlender.GetSourceAnimation())
	{
		if (m_AnimBlender.GetTargetAnimation())
		{
			return m_AnimBlender.GetTargetPlaybackTime() / m_AnimBlender.GetTargetAnimation()->GetFrameRate();
		}
		else
		{
			return m_AnimBlender.GetSourcePlaybackTime() / m_AnimBlender.GetSourceAnimation()->GetFrameRate();
		}
	}

	return 0.0f;
}

RAabb FTGPlayerController::GetMovementCollisionShape() const
{
	RAabb playerAabb;
	playerAabb.pMin = RVec3(-50.0f, 0.0f, -50.0f) + GetPosition();
	playerAabb.pMax = RVec3(50.0f, 150.0f, 50.0f) + GetPosition();

	return playerAabb;
}

RCapsule FTGPlayerController::GetCollisionShape() const
{
	return RCapsule{ GetPosition() + RVec3(0, 40, 0), GetPosition() + RVec3(0, 110, 0), 40 };
}

RAnimation* FTGPlayerController::LoadAnimation(const char* resPath, int flags)
{
	RMesh* mesh = RResourceManager::Instance().FindMesh(resPath);
	if (!mesh)
		mesh = RResourceManager::Instance().LoadFbxMesh(resPath, EResourceLoadMode::Immediate);

	if (mesh)
	{
		RAnimation* Animation = mesh->GetAnimation();
		if (Animation)
		{
			Animation->SetBitFlags(flags);

			string strResPath = string(resPath);
			string filename = RFileUtil::GetFileNameInPath(strResPath);
			Animation->SetName(filename);
			return Animation;
		}
		else
		{
			RLogWarning("Unable to find animation data in mesh resource \'%s\'\n", resPath);
		}
	}
	else
	{
		RLogWarning("Failed to load mesh resource \'%s\'\n", resPath);
	}

	return nullptr;
}

bool FTGPlayerController::CanMovePlayerWithInput() const
{
	return GetBehavior() == BHV_Running || GetBehavior() == BHV_Idle;
}
