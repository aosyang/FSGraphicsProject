//=============================================================================
// PlayerController.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "PlayerController.h"

BehaviorInfo PlayerBehaviorInfo[] =
{
	{ PlayerAnim_Idle },
	{ PlayerAnim_Run },
	{ PlayerAnim_Punch1 },
	{ PlayerAnim_Kick },
	{ PlayerAnim_BackKick },
	{ PlayerAnim_SpinAttack },
	{ PlayerAnim_Hit },
	{ PlayerAnim_Down },
	{ PlayerAnim_GetUp },
};

PlayerController::PlayerController()
	: m_Rotation(0.0f), m_CurrAnimTime(0.0f), m_Behavior(BHV_Idle)
{

}

void PlayerController::Cache()
{
	RMesh* playerMesh = RResourceManager::Instance().FindMesh("../Assets/unitychan/unitychan.fbx");
	if (!playerMesh)
		playerMesh = RResourceManager::Instance().LoadFbxMesh("../Assets/unitychan/unitychan.fbx", RLM_Immediate);
	SetMesh(playerMesh);

	m_Animations[PlayerAnim_Idle]		= LoadAnimation("../Assets/unitychan/FUCM05_0000_Idle.fbx");
	m_Animations[PlayerAnim_Run]		= LoadAnimation("../Assets/unitychan/FUCM_0012b_EH_RUN_LP_NoZ.fbx");
	m_Animations[PlayerAnim_Punch1]		= LoadAnimation("../Assets/unitychan/FUCM05_0001_M_CMN_LJAB.fbx");
	m_Animations[PlayerAnim_Kick]		= LoadAnimation("../Assets/unitychan/FUCM_04_0001_RHiKick.fbx");
	m_Animations[PlayerAnim_BackKick]	= LoadAnimation("../Assets/unitychan/FUCM02_0004_CH01_AS_MAWAK.fbx");
	m_Animations[PlayerAnim_SpinAttack] = LoadAnimation("../Assets/unitychan/FUCM02_0029_Cha01_STL01_ScrewK01.fbx");
	m_Animations[PlayerAnim_Hit]		= LoadAnimation("../Assets/unitychan/unitychan_DAMAGED00.fbx");
	m_Animations[PlayerAnim_Down]		= LoadAnimation("../Assets/unitychan/FUCM02_0025_MYA_TF_DOWN.fbx");
	m_Animations[PlayerAnim_GetUp]		= LoadAnimation("../Assets/unitychan/FUCM03_0019_HeadSpring.fbx");

	for (int i = 0; i < PlayerAnimCount; i++)
	{
		m_Mesh->CacheAnimation(m_Animations[i]);
	}

	m_CurrAnimation = m_Animations[PlayerAnim_Idle];
	m_CurrAnimTime = m_CurrAnimation->GetStartTime();
}

void PlayerController::PreUpdate(const RTimer& timer)
{
	if (m_CurrAnimation)
	{
		// Changing time may cause start time greater than end time
		if (m_CurrAnimTime >= m_CurrAnimation->GetEndTime() - 1)
		{
			m_CurrAnimTime = m_CurrAnimation->GetStartTime();
		}
		RVec3 start_offset = m_CurrAnimation->GetRootPosition(m_CurrAnimTime);

		m_CurrAnimTime += timer.DeltaTime() * m_CurrAnimation->GetFrameRate();
		bool startOver = false;
		bool animDone = false;

		if (m_CurrAnimTime >= m_CurrAnimation->GetEndTime() - 1)
		{
			if (IsPlayingLoopAnimation())
			{
				do
				{
					m_CurrAnimTime -= m_CurrAnimation->GetEndTime() - m_CurrAnimation->GetStartTime() - 1;
					startOver = true;
				} while (m_CurrAnimTime >= m_CurrAnimation->GetEndTime() - 1);
			}
			else
			{
				m_CurrAnimTime = m_CurrAnimation->GetEndTime() - 1;
				animDone = true;
			}
		}

		m_RootOffset = m_CurrAnimation->GetRootPosition(m_CurrAnimTime) - start_offset;
		if (startOver)
		{
			m_RootOffset = m_CurrAnimation->GetRootPosition(m_CurrAnimation->GetEndTime() - 1) - start_offset +
						   m_CurrAnimation->GetRootPosition(m_CurrAnimTime) - m_CurrAnimation->GetInitRootPosition();
		}

		if (m_Behavior == BHV_Idle || m_Behavior == BHV_Running)
			m_RootOffset = RVec3(0, 0, 0);

		if (animDone)
		{
			if (m_Behavior == BHV_HitDown)
				SetBehavior(BHV_GetUp);
			else
				SetBehavior(BHV_Idle);
		}
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

	if (t < 0) t = 0;
	if (t > 1) t = 1;

	return from + (to - from) * t;
}

void PlayerController::UpdateMovement(const RTimer& timer, const RVec3 moveVec)
{
	RVec3 hVec = moveVec;
	hVec.y = 0.0f;

	if (hVec.SquaredMagitude() > 0.0f)
	{
		hVec = hVec.GetNormalizedVec3();
		m_Rotation = LerpDegreeAngle(m_Rotation, RAD_TO_DEG(atan2f(hVec.x, hVec.z)), 10.0f * timer.DeltaTime());
	}

	RAabb playerAabb = GetMovementCollisionShape();

	RVec3 worldMoveVec = moveVec;
	worldMoveVec += (RVec4(GetRootOffset(), 0) * GetNodeTransform()).ToVec3();
	worldMoveVec = m_Scene->TestMovingAabbWithScene(playerAabb, worldMoveVec);

	Translate(worldMoveVec);

	SetRotation(RMatrix4::CreateYAxisRotation(m_Rotation));
}

void PlayerController::PostUpdate(const RTimer& timer)
{
	if (m_CurrAnimation)
	{
		bool hasRootMotion = (m_Behavior != BHV_Idle && m_Behavior != BHV_Running);

		RVec3 invOffset = -m_CurrAnimation->GetRootPosition(m_CurrAnimTime);
		RMatrix4 rootInversedTranslation = RMatrix4::CreateTranslation(invOffset);

		for (int i = 0; i < m_Mesh->GetBoneCount(); i++)
		{
			RMatrix4 matrix;

			int boneId = m_Mesh->GetCachedAnimationNodeId(m_CurrAnimation, i);
			m_CurrAnimation->GetNodePose(boneId, m_CurrAnimTime, &matrix);

			if (hasRootMotion)
				cbSkinned.boneMatrix[i] = m_Mesh->GetBoneInitInvMatrices(i) * matrix * rootInversedTranslation * GetNodeTransform();
			else
				cbSkinned.boneMatrix[i] = m_Mesh->GetBoneInitInvMatrices(i) * matrix * GetNodeTransform();
		}
	}
}

void PlayerController::Draw()
{
	m_Scene->cbBoneMatrices.UpdateContent(&cbSkinned);
	m_Scene->cbBoneMatrices.ApplyToShaders();

	RSMeshObject::Draw();
}

void PlayerController::DrawDepthPass()
{
	m_Scene->cbBoneMatrices.UpdateContent(&cbSkinned);
	m_Scene->cbBoneMatrices.ApplyToShaders();

	RSMeshObject::DrawDepthPass();
}

void PlayerController::SetBehavior(PlayerBehavior behavior)
{
	switch (behavior)
	{
	case BHV_Kick:
		if (m_Behavior != BHV_Kick && m_Behavior != BHV_BackKick)
		{
			m_CurrAnimation = m_Animations[PlayerAnim_Kick];
			m_CurrAnimTime = m_CurrAnimation->GetStartTime();
			m_Behavior = BHV_Kick;
		}
		else if (m_Behavior == BHV_Kick && (m_CurrAnimTime / m_CurrAnimation->GetFrameRate()) >= 0.5f)
		{
			m_CurrAnimation = m_Animations[PlayerAnim_BackKick];
			m_CurrAnimTime = m_CurrAnimation->GetStartTime();
			m_Behavior = BHV_BackKick;
		}
		break;

	case BHV_Hit:
		m_CurrAnimation = m_Animations[PlayerAnim_Hit];
		m_CurrAnimTime = m_CurrAnimation->GetStartTime();
		m_Behavior = BHV_Hit;
		break;

	default:
		if (m_Behavior != behavior)
		{
			m_CurrAnimation = m_Animations[PlayerBehaviorInfo[behavior].anim];
			m_CurrAnimTime = m_CurrAnimation->GetStartTime();
			m_Behavior = behavior;
		}
	}
}

PlayerBehavior PlayerController::GetBehavior() const
{
	return m_Behavior;
}

float PlayerController::GetBehaviorTime() const
{
	if (m_CurrAnimation)
	{
		return m_CurrAnimTime / m_CurrAnimation->GetFrameRate();
	}

	return 0.0f;
}

bool PlayerController::IsPlayingLoopAnimation() const
{
	switch (m_Behavior)
	{
	case BHV_Idle:
	case BHV_Running:
		return true;
	case BHV_Punch:
	case BHV_Kick:
	case BHV_SpinAttack:
		return false;
	}

	return false;
}

RAabb PlayerController::GetMovementCollisionShape() const
{
	RAabb playerAabb;
	playerAabb.pMin = RVec3(-50.0f, 0.0f, -50.0f) + GetPosition();
	playerAabb.pMax = RVec3(50.0f, 150.0f, 50.0f) + GetPosition();

	return playerAabb;
}

RCapsule PlayerController::GetCollisionShape() const
{
	return RCapsule{ GetPosition() + RVec3(0, 40, 0), GetPosition() + RVec3(0, 110, 0), 40 };
}

RAnimation* PlayerController::LoadAnimation(const char* resPath)
{
	RMesh* mesh = RResourceManager::Instance().FindMesh(resPath);
	if (!mesh)
		mesh = RResourceManager::Instance().LoadFbxMesh(resPath, RLM_Immediate);

	if (mesh)
		return mesh->GetAnimation();

	return nullptr;
}