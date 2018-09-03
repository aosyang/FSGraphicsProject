//=============================================================================
// FTGPlayerController.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "FTGPlayerController.h"

IMPLEMENT_SCENE_OBJECT(FTGPlayerController);

std::list<RSceneObject*> FTGPlayerController::ActivePlayerControllers;

FTGPlayerController::FTGPlayerController(RScene* InScene)
	: Base(InScene)
	, m_Rotation(0.0f)
{
	ActivePlayerControllers.push_back(this);
}

FTGPlayerController::~FTGPlayerController()
{
	ActivePlayerControllers.remove(this);
}

void FTGPlayerController::InitAssets()
{
	RMesh* playerMesh = RResourceManager::Instance().LoadFbxMesh("../Assets/unitychan/unitychan.fbx", EResourceLoadMode::Immediate);
	SetMesh(playerMesh);

	m_StateMachine.Init(this);
	m_StateMachine.CacheAnimations(m_Mesh);
}

void FTGPlayerController::PreUpdate(const RTimer& timer)
{
	m_StateMachine.Update(timer.DeltaTime());

	m_RootOffset = GetAnimBlender().GetCurrentRootOffset();

	//if (m_Behavior == BHV_Idle || m_Behavior == BHV_Run)
	//	m_RootOffset = RVec3(0, 0, 0);
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
	worldMoveVec = m_Scene->TestMovingAabbWithScene(playerAabb, worldMoveVec, ActivePlayerControllers);

	Translate(worldMoveVec + StairOffset, ETransformSpace::World);

	SetRotation(RQuat::Euler(0.0f, DEG_TO_RAD(m_Rotation), 0.0f));
}

void FTGPlayerController::PostUpdate(const RTimer& timer)
{
	for (int i = 0; i < m_Mesh->GetBoneCount(); i++)
	{
		RMatrix4 matrix;

		int sourceBoneId = m_Mesh->GetCachedAnimationNodeId(GetAnimBlender().GetSourceAnimation(), i);
		int targetBondId = m_Mesh->GetCachedAnimationNodeId(GetAnimBlender().GetTargetAnimation(), i);
		GetAnimBlender().GetCurrentBlendedNodePose(sourceBoneId, targetBondId, &matrix);

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

void FTGPlayerController::SetPlayerFacing(const RVec3& Direction)
{
	if (Direction.Magnitude() > 0.0f)
	{
		RVec3 NormalizedDir = Direction.GetNormalized();
		SetPlayerRotation(RAD_TO_DEG(atan2f(-NormalizedDir.X(), -NormalizedDir.Z())));
	}
}

void FTGPlayerController::SetBehavior(EPlayerBehavior behavior)
{
	m_StateMachine.SetNextBehavior(behavior);

	//switch (behavior)
	//{
	//case BHV_Kick:
	//	if (m_Behavior != BHV_Kick && m_Behavior != BHV_BackKick)
	//	{
	//		GetAnimBlender().Play(m_Animations[PlayerAnim_Kick]);
	//		m_Behavior = BHV_Kick;
	//	}
	//	else if (m_Behavior == BHV_Kick && GetBehaviorTime() >= 0.3f)
	//	{
	//		GetAnimBlender().Play(m_Animations[PlayerAnim_BackKick]);
	//		m_Behavior = BHV_BackKick;
	//	}
	//	break;

	//case BHV_Hit:
	//	GetAnimBlender().Play(m_Animations[PlayerAnim_Hit]);
	//	m_Behavior = BHV_Hit;
	//	break;

	//default:
	//	if (m_Behavior != behavior)
	//	{
	//		const BehaviorInfo& CurrentBehavior = PlayerBehaviorInfo[behavior];

	//		if (CurrentBehavior.blendTime > 0.0f)
	//		{
	//			GetAnimBlender().BlendTo(m_Animations[CurrentBehavior.anim],
	//								  m_Animations[CurrentBehavior.anim]->GetStartTime(), 1.0f,
	//								  CurrentBehavior.blendTime);
	//		}
	//		else
	//		{
	//			GetAnimBlender().Play(m_Animations[CurrentBehavior.anim]);
	//		}
	//		m_Behavior = behavior;
	//	}
	//}
}

float FTGPlayerController::GetBehaviorTime()
{
	return m_StateMachine.GetCurrentBehaviorTime();
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

vector<FTGPlayerController*> FTGPlayerController::TestSphereHitWithOtherPlayers(float Radius, const RVec3& LocalSpaceOffset)
{
	RSphere HitSphere;
	HitSphere.center = GetTransform()->GetTranslatedVector(LocalSpaceOffset, ETransformSpace::Local);
	HitSphere.radius = Radius;

	GDebugRenderer.DrawSphere(HitSphere.center, HitSphere.radius);

	vector<FTGPlayerController*> Results;
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

bool FTGPlayerController::CanMovePlayerWithInput() const
{
	return m_StateMachine.GetCurrentBehavior() == BHV_Run || m_StateMachine.GetCurrentBehavior() == BHV_Idle;
}
