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
#include "PlayerBehavior_Navigation.h"

namespace
{
	static const float FixedTimestamp = 1.0f / 60.0f;
}

class RKinematicCharacterController : public btKinematicCharacterController
{
public:
	RKinematicCharacterController(btPairCachingGhostObject* ghostObject, btConvexShape* convexShape, btScalar stepHeight, const btVector3& up = btVector3(1.0, 0.0, 0.0))
		: btKinematicCharacterController(ghostObject, convexShape, stepHeight, up)
		, LocalTime(0.0f)
		, NumSimulationSubSteps(0)
		, CurrentFrame(0)
	{
		const btTransform& PhysicsTransform = ghostObject->getWorldTransform();
		for (int i = 0; i < 2; i++)
		{
			ControllerPosition[i] = btVec3ToRVec3(PhysicsTransform.getOrigin());
			ControllerRotation[i] = btQuatToRQuat(PhysicsTransform.getRotation());
		}
	}

	// Override the update function from btKinematicCharacterController so it can handle transform interpolation
	virtual void updateAction(btCollisionWorld * collisionWorld, btScalar deltaTime) override
	{
		preStep(collisionWorld);
		playerStep(collisionWorld, deltaTime);

		const btTransform& PhysicsTransform = getGhostObject()->getWorldTransform();
		ControllerPosition[CurrentFrame] = btVec3ToRVec3(PhysicsTransform.getOrigin());
		ControllerRotation[CurrentFrame] = btQuatToRQuat(PhysicsTransform.getRotation());

		CurrentFrame = 1 - CurrentFrame;
	}

	void Update(float DeltaTime)
	{
		LocalTime += DeltaTime;

		if (LocalTime > FixedTimestamp)
		{
			NumSimulationSubSteps = int(LocalTime / FixedTimestamp);
			LocalTime -= NumSimulationSubSteps * FixedTimestamp;
		}
	}

	RVec3 GetInterpolatedPosition() const
	{
		float t = LocalTime / FixedTimestamp;
		assert(t >= 0.0f && t <= 1.0f);

		return RVec3::Lerp(ControllerPosition[1 - CurrentFrame], ControllerPosition[CurrentFrame], t);
	}

	RQuat GetInterpolatedRotation() const
	{
		float t = LocalTime / FixedTimestamp;
		assert(t >= 0.0f && t <= 1.0f);

		return RQuat::Slerp(ControllerRotation[1 - CurrentFrame], ControllerRotation[CurrentFrame], t);
	}

private:
	float LocalTime;
	int NumSimulationSubSteps;

	int CurrentFrame;
	RVec3 ControllerPosition[2];
	RQuat ControllerRotation[2];
};

std::list<RSceneObject*> PlayerControllerBase::ActivePlayerControllers;

PlayerControllerBase::PlayerControllerBase(const RConstructingParams& Params)
	: Base(Params)
	, StairOffset(0.0f, 20.0f, 0.0f)
	, m_MovementInput(0.0f, 0.0f, 0.0f)
	, PlannarMoveVector(0.0f, 0.0f, 0.0f)
	, MaxMovementSpeed(0.5f)
	, CapsuleRadius(40.0f)
	, CapsuleHeight(70.0f)
	, m_Rotation(0.0f)
	, m_StateMachine(this)
{
	ActivePlayerControllers.push_back(this);

	GhostObject = std::make_unique<btPairCachingGhostObject>();
	CapsuleShape = std::make_unique<btCapsuleShape>(CapsuleRadius, CapsuleHeight);
	KinematicCharacterController = std::make_unique<RKinematicCharacterController>(GhostObject.get(), CapsuleShape.get(), 30.0f, btVector3(0.0f, 1.0f, 0.0f));
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

	KinematicCharacterController->Update(DeltaTime);

	RVec3 Position = KinematicCharacterController->GetInterpolatedPosition();
	RQuat Rotation = KinematicCharacterController->GetInterpolatedRotation();

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

	m_RootOffset = GetStateMachine().GetCurrentRootOffset();

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
	bool bCanMovePlayer = CanMovePlayerWithInput();
	if (bCanMovePlayer)
	{
		KinematicCharacterController->setWalkDirection(RVec3TobtVec3(MoveVec));

		PlannarMoveVector = MoveVec;
		PlannarMoveVector.SetY(0.0f);
		float SqrMagnitude = PlannarMoveVector.SquaredMagitude();

		if (SqrMagnitude > 0.0f)
		{
			RVec3 MoveDirection = PlannarMoveVector.GetNormalized();
			m_Rotation = LerpDegreeAngle(m_Rotation, RAD_TO_DEG(atan2f(-MoveDirection.X(), -MoveDirection.Z())), 10.0f * DeltaTime);
		}
	}
	else
	{
		PlannarMoveVector = RVec3::Zero();
		KinematicCharacterController->setWalkDirection(btVector3(0, 0, 0));
	}

	RVec3 RootMotionTranslation = (RVec4(GetRootOffset(), 0) * GetTransformMatrix()).ToVec3();
	btTransform PhysicsTransform = GhostObject->getWorldTransform();
	PhysicsTransform.setOrigin(PhysicsTransform.getOrigin() + RVec3TobtVec3(RootMotionTranslation));
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
	// Evaluate the skeletal pose at current time
	GetStateMachine().EvaluatePose(*m_Mesh, m_BoneMatrices);

	// Transform all bones to world space
	const RMatrix4& Transform = GetTransformMatrix();
	for (int i = 0; i < m_Mesh->GetBoneCount(); i++)
	{
		m_BoneMatrices[i] = m_Mesh->GetBoneInitInvMatrices(i) * m_BoneMatrices[i] * Transform;
	}
}

void PlayerControllerBase::Draw()
{
	// Copy bone transforms to constant buffer
	memcpy(&RConstantBuffers::cbBoneMatrices.Data.boneMatrix, m_BoneMatrices, sizeof(RMatrix4) * MAX_BONE_COUNT);
	RConstantBuffers::cbBoneMatrices.UpdateBufferData();
	RConstantBuffers::cbBoneMatrices.BindBuffer();

	RSMeshObject::Draw();
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

	m_MovementInput = Input * MaxMovementSpeed * m_StateMachine.GetAnimationDeviation();
}

RVec3 PlayerControllerBase::GetPlannarMovementVector() const
{
	return PlannarMoveVector;
}

RVec3 PlayerControllerBase::GetVelocity() const
{
	return btVec3ToRVec3(KinematicCharacterController->getLinearVelocity());
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
	return m_StateMachine.GetCurrentBehaviorId() == PlayerBehavior_Run::StaticClassId() ||
		   m_StateMachine.GetCurrentBehaviorId() == PlayerBehavior_Walk::StaticClassId() ||
		   m_StateMachine.GetCurrentBehaviorId() == PlayerBehavior_Idle::StaticClassId() ||
		   m_StateMachine.GetCurrentBehaviorId() == PlayerBehavior_Navigation::StaticClassId();
}
