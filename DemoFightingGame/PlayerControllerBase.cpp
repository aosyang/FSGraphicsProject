//=============================================================================
// PlayerControllerBase.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "PlayerControllerBase.h"

#include "Navigation/RAINavigationComponent.h"

// For definition of RPhysicsEngineContext
// TODO: Should keep this header private. Maybe move implementation to the engine
#include "Physics/RPhysicsPrivate.h"

#include "BulletCollision/CollisionDispatch/btGhostObject.h"

std::list<RSceneObject*> PlayerControllerBase::ActivePlayerControllers;

PlayerControllerBase::PlayerControllerBase(const RConstructingParams& Params)
	: Base(Params)
	, StairOffset(0.0f, 20.0f, 0.0f)
	, m_MovementInput(0.0f, 0.0f, 0.0f)
	, CapsuleRadius(40.0f)
	, CapsuleHeight(70.0f)
	, m_Rotation(0.0f)
	, m_StateMachine(this)
{
	ActivePlayerControllers.push_back(this);

	GhostObject = std::make_unique<btPairCachingGhostObject>();
	CapsuleShape = std::make_unique<btCapsuleShape>(CapsuleRadius, CapsuleHeight);
	KinematicCharacterController = std::make_unique<btKinematicCharacterController>(GhostObject.get(), CapsuleShape.get(), 30.0f, btVector3(0.0f, 1.0f, 0.0f));
	KinematicCharacterController->setGravity(btVector3(0, -980 * 3, 0));
	KinematicCharacterController->setFallSpeed(55000);

	GhostObject->setCollisionShape(CapsuleShape.get());
	GhostObject->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);

	GPhysicsEngine.GetContext()->DynamicWorld->addCollisionObject(GhostObject.get(), btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);
	GPhysicsEngine.GetContext()->DynamicWorld->addAction(KinematicCharacterController.get());
}

PlayerControllerBase::~PlayerControllerBase()
{
	GPhysicsEngine.GetContext()->DynamicWorld->removeAction(KinematicCharacterController.get());
	GPhysicsEngine.GetContext()->DynamicWorld->removeCollisionObject(GhostObject.get());

	ActivePlayerControllers.remove(this);
}

void PlayerControllerBase::Update(float DeltaTime)
{
	Base::Update(DeltaTime);

	const btTransform& PhysicsTransform = GhostObject->getWorldTransform();
	RVec3 Position = btVec3ToRVec3(PhysicsTransform.getOrigin());
	RQuat Rotation = btQuatToRQuat(PhysicsTransform.getRotation());

	// Debug draw character capsule
	//GDebugRenderer.DrawCapsule(Position, CapsuleHeight, CapsuleRadius, RColor::Yellow, 8);

	RScopeInternalTransformUpdate InternalTransformUpdate(this);

	SetPosition(Position - GetHalfCapsuleOffset());
	SetRotation(Rotation);
}

void PlayerControllerBase::InitAssets(const std::string& MeshResourcePath)
{
	RMesh* playerMesh = RResourceManager::Instance().LoadResource<RMesh>(MeshResourcePath, EResourceLoadMode::Immediate);
	SetMesh(playerMesh);

	m_StateMachine.InitAssets();
}

void PlayerControllerBase::UpdateController(float DeltaTime)
{
	PreUpdate(DeltaTime);

	// Note: Bullet physics requires a fixed input vector for character movements
	UpdateMovement(DeltaTime, m_MovementInput / 60.0f);
	PostUpdate(DeltaTime);
}

void PlayerControllerBase::PreUpdate(float DeltaTime)
{
	m_StateMachine.Update(DeltaTime);

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

	return from + (to - from) * RMath::Clamp(t, 0.0f, 1.0f);
}

void PlayerControllerBase::UpdateMovement(float DeltaTime, const RVec3 MoveVec)
{
	KinematicCharacterController->setWalkDirection(RVec3TobtVec3(MoveVec));

	bool bCanMovePlayer = CanMovePlayerWithInput();
	if (bCanMovePlayer)
	{
		RVec3 PlannarMoveVector = MoveVec;
		PlannarMoveVector.SetY(0.0f);

		if (PlannarMoveVector.SquaredMagitude() > 0.0f)
		{
			PlannarMoveVector = PlannarMoveVector.GetNormalized();
			m_Rotation = LerpDegreeAngle(m_Rotation, RAD_TO_DEG(atan2f(-PlannarMoveVector.X(), -PlannarMoveVector.Z())), 10.0f * DeltaTime);

			SetBehavior(BHV_Run);
		}
		else
		{
			SetBehavior(BHV_Idle);
		}
	}

	btTransform PhysicsTransform = GhostObject->getWorldTransform();
	PhysicsTransform.setRotation(RQuatTobtQuat(RQuat::Euler(0.0f, DEG_TO_RAD(m_Rotation), 0.0f)));
	GhostObject->setWorldTransform(PhysicsTransform);

#if 0
	RAabb playerAabb = GetMovementCollisionShape();
	playerAabb.pMin += StairOffset;
	playerAabb.pMax += StairOffset;

	RVec3 worldMoveVec = bCanMovePlayer ? moveVec : RVec3::Zero();

	// Apply gravity
	worldMoveVec += RVec3(0, -1000.0f * DeltaTime, 0);

	worldMoveVec += (RVec4(GetRootOffset(), 0) * GetTransformMatrix()).ToVec3();
	worldMoveVec -= StairOffset;
	worldMoveVec = m_Scene->TestMovingAabbWithScene(playerAabb, worldMoveVec, ActivePlayerControllers);

	Translate(worldMoveVec + StairOffset, ETransformSpace::World);

	SetRotation(RQuat::Euler(0.0f, DEG_TO_RAD(m_Rotation), 0.0f));
#endif	// if 0
}

void PlayerControllerBase::PostUpdate(float DeltaTime)
{
	RAnimation* const SourceAnim = GetAnimBlender().GetSourceAnimation();
	RAnimation* const TargetAnim = GetAnimBlender().GetTargetAnimation();

	for (int i = 0; i < m_Mesh->GetBoneCount(); i++)
	{
		RMatrix4 BoneMatrix;

		int SourceBoneId = m_Mesh->GetCachedAnimationNodeId(SourceAnim, i);
		int TargetBondId = m_Mesh->GetCachedAnimationNodeId(TargetAnim, i);

		bool Result = GetAnimBlender().GetCurrentBlendedNodePose(SourceBoneId, TargetBondId, &BoneMatrix);
		if (!Result)
		{
			BoneMatrix = RMatrix4::Zero;
		}

		m_BoneMatrices[i] = m_Mesh->GetBoneInitInvMatrices(i) * BoneMatrix * GetTransformMatrix();

#if 0	// Debug rendering bones
		{
			int ParentId = SourceAnim->GetParentId(i);
			if (ParentId != -1)
			{
				RMatrix4 ParentMatrix;
				if (!GetAnimBlender().GetCurrentBlendedNodePose(ParentId, ParentId, &ParentMatrix))
				{
					ParentMatrix = RMatrix4::Zero;
				}

				GDebugRenderer.DrawLine(BoneMatrix.GetTranslation(), ParentMatrix.GetTranslation());
			}
		}
#endif
	}
}

void PlayerControllerBase::Draw()
{
	// TODO: blend state should be set by material
	GRenderer.SetBlendState(Blend_AlphaBlending);

	// Copy bone transforms to constant buffer
	memcpy(&RConstantBuffers::cbBoneMatrices.Data.boneMatrix, m_BoneMatrices, sizeof(RMatrix4) * MAX_BONE_COUNT);
	RConstantBuffers::cbBoneMatrices.UpdateBufferData();
	RConstantBuffers::cbBoneMatrices.BindBuffer();

	RSMeshObject::Draw();

	GRenderer.SetBlendState(Blend_Opaque);
}

void PlayerControllerBase::DrawDepthPass()
{
	// Copy bone transforms to constant buffer
	memcpy(&RConstantBuffers::cbBoneMatrices.Data.boneMatrix, m_BoneMatrices, sizeof(RMatrix4) * MAX_BONE_COUNT);
	RConstantBuffers::cbBoneMatrices.UpdateBufferData();
	RConstantBuffers::cbBoneMatrices.BindBuffer();

	RSMeshObject::DrawDepthPass();
}

void PlayerControllerBase::SetMovementInput(const RVec3& Input)
{
	if (Input.HasNan())
	{
		DebugBreak();
	}

	m_MovementInput = Input * m_StateMachine.GetAnimationDeviation();
}

void PlayerControllerBase::SetPlayerFacing(const RVec3& Direction, bool bCheckMoveAllowed /*= true*/)
{
	if (bCheckMoveAllowed && !CanMovePlayerWithInput())
	{
		return;
	}

	if (Direction.Magnitude() > 0.0f)
	{
		RVec3 NormalizedDir = Direction.GetNormalized();
		SetPlayerRotation(RAD_TO_DEG(atan2f(-NormalizedDir.X(), -NormalizedDir.Z())));
	}
}

void PlayerControllerBase::SetBehavior(EPlayerBehavior behavior)
{
	m_StateMachine.SetNextBehavior(behavior);
}

float PlayerControllerBase::GetBehaviorTime()
{
	return m_StateMachine.GetCurrentBehaviorTime();
}

RAabb PlayerControllerBase::GetMovementCollisionShape() const
{
	RAabb playerAabb;
	playerAabb.pMin = RVec3(-50.0f, 0.0f, -50.0f) + GetPosition();
	playerAabb.pMax = RVec3(50.0f, 150.0f, 50.0f) + GetPosition();

	return playerAabb;
}

RCapsule PlayerControllerBase::GetCollisionShape() const
{
	return RCapsule{ GetPosition() + RVec3(0, 40, 0), GetPosition() + RVec3(0, 110, 0), 40 };
}

void PlayerControllerBase::SetAnimationDeviation(float Deviation)
{
	m_StateMachine.SetAnimationDeviation(Deviation);
}

void PlayerControllerBase::Reset()
{
	// When reseting the position of an AI player, also clear its current nav path.
	RAINavigationComponent* AINavigationComponent = FindComponent<RAINavigationComponent>();
	if (AINavigationComponent)
	{
		AINavigationComponent->StopMovement();
	}

	OnPlayerReset.Execute();
}

void PlayerControllerBase::OnTransformModified()
{
	// If SetPosition/SetRotation is called on a player controller, update transform for physics object
	btTransform PhysicsTransform;
	PhysicsTransform.setIdentity();
	PhysicsTransform.setOrigin(RVec3TobtVec3(GetWorldPosition() + GetHalfCapsuleOffset()));
	PhysicsTransform.setRotation(RQuatTobtQuat(GetRotation()));

	GhostObject->setWorldTransform(PhysicsTransform);
}

RVec3 PlayerControllerBase::GetHalfCapsuleOffset() const
{
	return RVec3(0, CapsuleHeight / 2 + CapsuleRadius, 0);
}

bool PlayerControllerBase::CanMovePlayerWithInput() const
{
	return m_StateMachine.GetCurrentBehavior() == BHV_Run || m_StateMachine.GetCurrentBehavior() == BHV_Idle;
}
