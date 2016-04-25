//=============================================================================
// PlayerController.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "PlayerController.h"

PlayerController::PlayerController()
	: m_CurrAnimTime(0.0f), m_Behavior(BHV_Idle)
{

}

void PlayerController::Cache()
{
	SetMesh(RResourceManager::Instance().LoadFbxMesh("../Assets/unitychan/unitychan.fbx", RLM_Immediate));
	m_Animations[PlayerAnim_Idle]		= LoadAnimation("../Assets/unitychan/FUCM05_0000_Idle.fbx");
	m_Animations[PlayerAnim_Run]		= LoadAnimation("../Assets/unitychan/FUCM_0012b_EH_RUN_LP_NoZ.fbx");
	m_Animations[PlayerAnim_Punch1]		= LoadAnimation("../Assets/unitychan/FUCM05_0001_M_CMN_LJAB.fbx");
	m_Animations[PlayerAnim_SpinAttack]	= LoadAnimation("../Assets/unitychan/FUCM02_0029_Cha01_STL01_ScrewK01.fbx");

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

		if (animDone)
		{
			SetBehavior(BHV_Idle);
		}
	}
}

void PlayerController::PostUpdate(const RTimer& timer)
{
	if (m_CurrAnimation)
	{
		RVec3 invOffset = -m_CurrAnimation->GetRootPosition(m_CurrAnimTime);
		RMatrix4 rootInversedTranslation = RMatrix4::CreateTranslation(invOffset);

		SHADER_SKINNED_BUFFER cbSkinned;
		for (int i = 0; i < m_Mesh->GetBoneCount(); i++)
		{
			RMatrix4 matrix;

			int boneId = m_Mesh->GetCachedAnimationNodeId(m_CurrAnimation, i);
			m_CurrAnimation->GetNodePose(boneId, m_CurrAnimTime, &matrix);

			cbSkinned.boneMatrix[i] = m_Mesh->GetBoneInitInvMatrices(i) * matrix * rootInversedTranslation * GetNodeTransform();
		}

		m_Scene->cbBoneMatrices.UpdateContent(&cbSkinned);
		m_Scene->cbBoneMatrices.ApplyToShaders();
	}
}

void PlayerController::SetBehavior(PlayerBehavior behavior)
{
	switch (behavior)
	{
	case BHV_Idle:
		if (m_Behavior != BHV_Idle)
		{
			m_CurrAnimation = m_Animations[PlayerAnim_Idle];
			m_CurrAnimTime = m_CurrAnimation->GetStartTime();
			m_Behavior = BHV_Idle;
		}
		break;

	case BHV_Running:
		if (m_Behavior != BHV_Running)
		{
			m_CurrAnimation = m_Animations[PlayerAnim_Run];
			m_CurrAnimTime = m_CurrAnimation->GetStartTime();
			m_Behavior = BHV_Running;
		}
		break;

	case BHV_SpinAttack:
		if (m_Behavior != BHV_SpinAttack)
		{
			m_CurrAnimation = m_Animations[PlayerAnim_SpinAttack];
			m_CurrAnimTime = m_CurrAnimation->GetStartTime();
			m_Behavior = BHV_SpinAttack;
		}
		break;
	}
}

PlayerBehavior PlayerController::GetBehavior() const
{
	return m_Behavior;
}

bool PlayerController::IsPlayingLoopAnimation() const
{
	switch (m_Behavior)
	{
	case BHV_Idle:
	case BHV_Running:
		return true;
	case BHV_SpinAttack:
		return false;
	}

	return false;
}

RAnimation* PlayerController::LoadAnimation(const char* resPath)
{
	RMesh* mesh = RResourceManager::Instance().LoadFbxMesh(resPath, RLM_Immediate);
	if (mesh)
		return mesh->GetAnimation();

	return nullptr;
}