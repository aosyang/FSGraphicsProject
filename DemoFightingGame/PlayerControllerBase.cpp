//=============================================================================
// PlayerControllerBase.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "PlayerControllerBase.h"

// For definition of RPhysicsEngineContext
// TODO: Should keep this header private. Maybe move implementation to the engine
#include "Physics/RPhysicsPrivate.h"

#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "PlayerBehavior_Navigation.h"


float TurnTowards_Rad(float From, float To, float Delta)
{
	while (From - To > PI)  { From -= 2.f * PI; }
	while (From - To < -PI) { From += 2.f * PI; }
	float Sign = RMath::Sign(To - From);

	return From + RMath::Min(fabs(To - From), fabs(Delta)) * Sign;
}

float GetYawFromDirection_Rad(const RVec3 Direction)
{
	return atan2f(-Direction.X(), -Direction.Z());
}

float LerpAngle_Deg(float From, float To, float t)
{
	while (From - To > 180.0f)  { From -= 360.0f; }
	while (From - To < -180.0f) { From += 360.0f; }
	return From + (To - From) * RMath::Clamp(t, 0.0f, 1.0f);
}

class RKinematicCharacterController : public btKinematicCharacterController
{
public:
	RKinematicCharacterController(btPairCachingGhostObject* ghostObject, btConvexShape* convexShape, btScalar stepHeight, const btVector3& up = btVector3(1.0, 0.0, 0.0))
		: btKinematicCharacterController(ghostObject, convexShape, stepHeight, up)
		, PhysicsLocalTime(0.0f)
		, EngineLocalTime(0.0f)
		, NumSimulationSubSteps(0)
		, CurrentFrame(0)
	{
		ResetTransformInterpolation();
	}

	// Override the update function from btKinematicCharacterController so it can handle transform interpolation
	virtual void updateAction(btCollisionWorld * collisionWorld, btScalar FixedDeltaTime) override
	{
		btKinematicCharacterController::updateAction(collisionWorld, FixedDeltaTime);

		PhysicsLocalTime += FixedDeltaTime;
		CurrentFrame = 1 - CurrentFrame;

		const btTransform& PhysicsTransform = getGhostObject()->getWorldTransform();
		ControllerPosition[CurrentFrame] = btVec3ToRVec3(PhysicsTransform.getOrigin());
		ControllerRotation[CurrentFrame] = btQuatToRQuat(PhysicsTransform.getRotation());
	}

	void Update_PostPhysics(float DeltaTime)
	{
		if (PhysicsLocalTime > EngineLocalTime)
		{
			PhysicsLocalTime -= EngineLocalTime;
			EngineLocalTime = 0.0f;
		}

		EngineLocalTime += DeltaTime;
	}

	RVec3 GetInterpolatedPosition() const
	{
		float t = (EngineLocalTime - PhysicsLocalTime) / RPhysicsEngine::GetFixedTimeStep();
		t = RMath::Clamp(t, 0.0f, 1.0f);
		assert(t >= 0.0f && t <= 1.0f);

		return RVec3::Lerp(ControllerPosition[1 - CurrentFrame], ControllerPosition[CurrentFrame], t);
	}

	RQuat GetInterpolatedRotation() const
	{
		float t = (EngineLocalTime - PhysicsLocalTime) / RPhysicsEngine::GetFixedTimeStep();
		t = RMath::Clamp(t, 0.0f, 1.0f);
		assert(t >= 0.0f && t <= 1.0f);

		return RQuat::Slerp(ControllerRotation[1 - CurrentFrame], ControllerRotation[CurrentFrame], t);
	}

	void ResetTransformInterpolation()
	{
		const btTransform& PhysicsTransform = getGhostObject()->getWorldTransform();
		for (int i = 0; i < 2; i++)
		{
			ControllerPosition[i] = btVec3ToRVec3(PhysicsTransform.getOrigin());
			ControllerRotation[i] = btQuatToRQuat(PhysicsTransform.getRotation());
		}
	}

private:
	float PhysicsLocalTime;
	float EngineLocalTime;
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
	, m_DampedMovementInput(0.0f, 0.0f, 0.0f)
	, m_LastInputUnitDirection(0.0f, 0.0f, 0.0f)
	, DampedYaw(PI)	// Note: The character is facing -Forward
	, PlannarMoveVector(0.0f, 0.0f, 0.0f)
	, MaxMovementSpeed(1.0f)
	, MotorAcceleration(500.0f)
	, CapsuleRadius(40.0f)
	, CapsuleHeight(70.0f)
	, m_Rotation(0.0f)
	, m_StateMachine(this)
	, MoveSpeed(0.0f)
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

	if (RAnimGraph* AnimGraph = RResourceManager::Instance().LoadResource<RAnimGraph>("/Maid/Maid_Navigation.ranimgraph", EResourceLoadMode::Immediate))
	{
		AnimGraphInstance = AnimGraph->CreateInstance();

		// Bind horizontal speed as the blend input of anim node
		AnimGraphInstance->BindAnimVariable("BlendInput", &MoveSpeed);
	}
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

	if (RAnimGraphInstance* RawAnimGraphInstance = AnimGraphInstance.get())
	{
		// Calculate horizontal speed for the character
		MoveSpeed = GetVelocity().Magnitude2D();
		RawAnimGraphInstance->Update(DeltaTime);
	}
}

void PlayerControllerBase::Update_PostPhysics(float DeltaTime)
{
	Base::Update_PostPhysics(DeltaTime);
	KinematicCharacterController->Update_PostPhysics(DeltaTime);

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

#if 1
	// Faster damping if movement input is non-zero
	//const float DampingSpeed = (m_MovementInput.IsZero() ? 500.f : 1000.0f) * DeltaTime;
	const float DampingSpeed = GetMotorAcceleration() * DeltaTime;

	const float Magnitude = m_MovementInput.Magnitude();
	float DampedMagnitude = m_DampedMovementInput.Magnitude();
	const float SignMagnitude = RMath::Sign(Magnitude - DampedMagnitude);
	DampedMagnitude += RMath::Min(fabs(DampedMagnitude - Magnitude), DampingSpeed) * SignMagnitude;

	const RVec3 MoveDirection = m_MovementInput.GetNormalized2D();
#if 0		// Rotation damping
	float TargetYaw = MoveDirection.IsZero() ? DampedYaw : GetYawFromDirection_Rad(MoveDirection) + PI;

	//RLog("Yaw from damped dir: %f - Target yaw: %f\n", DampedYaw, TargetYaw);

	DampedYaw = TurnTowards_Rad(DampedYaw, TargetYaw, 10.0f * DeltaTime);
	//RLog("Damped yaw: %f\n", DampedYaw);

	const RVec3 DampedForward = RQuat::Euler(0.0f, DampedYaw, 0.0f) * RVec3(0, 0, 1);
#else
	const RVec3 DampedForward = MoveDirection.IsZero() ? m_LastInputUnitDirection : MoveDirection;
#endif

	m_DampedMovementInput = DampedForward * DampedMagnitude;
	const RVec3 WorldPosition = GetWorldPosition() + RVec3(0.0f, 75.0f, 0.0f);
	GDebugRenderer.DrawLine(WorldPosition, WorldPosition + m_LastInputUnitDirection * 100.0f, RColor::Cyan);
	GDebugRenderer.DrawLine(WorldPosition, WorldPosition + DampedForward * 100.0f, RColor::Green);
#else
	// No damping
	m_DampedMovementInput = m_MovementInput;
#endif

	if (!m_MovementInput.IsZero())
	{
		m_LastInputUnitDirection = m_MovementInput.GetNormalized2D();
	}

	// Note: Bullet physics requires a fixed input vector for character movements
	UpdateMovement(DeltaTime, m_DampedMovementInput);
	PostUpdate(DeltaTime);
}

void PlayerControllerBase::PreUpdate(float DeltaTime)
{
	m_StateMachine.Update(DeltaTime);

	m_RootOffset = GetStateMachine().GetCurrentRootOffset();

	//if (m_Behavior == BHV_Idle || m_Behavior == BHV_Run)
	//	m_RootOffset = RVec3(0, 0, 0);
}

void PlayerControllerBase::UpdateMovement(float DeltaTime, const RVec3 MoveVec)
{
	bool bCanMovePlayer = CanMovePlayerWithInput();
	if (bCanMovePlayer)
	{
		KinematicCharacterController->setWalkDirection(RVec3TobtVec3(MoveVec) * RPhysicsEngine::GetFixedTimeStep());

		PlannarMoveVector = MoveVec;
		PlannarMoveVector.SetY(0.0f);
		float SqrMagnitude = PlannarMoveVector.SquaredMagitude();

		if (SqrMagnitude > 0.0f)
		{
			RVec3 MoveDirection = PlannarMoveVector.GetNormalized();
			m_Rotation = LerpAngle_Deg(m_Rotation, RAD_TO_DEG(atan2f(-MoveDirection.X(), -MoveDirection.Z())), 10.0f * DeltaTime);
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
	RAnimPoseData PoseData(*m_Mesh);

	// Evaluate the skeletal pose at current time
	//GetStateMachine().EvaluatePose(PoseData);

	if (RAnimGraphInstance* RawAnimGraphInstance = AnimGraphInstance.get())
	{
		RawAnimGraphInstance->EvaluatePose(PoseData);
	}

	// Transform all bones from object space to world space
	PoseData.CopyFinalPose(GetTransformMatrix(), m_BoneMatrices);
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

RVec3 PlayerControllerBase::GetPhysicsVelocity() const
{
	return btVec3ToRVec3(KinematicCharacterController->getLinearVelocity());
}

RVec3 PlayerControllerBase::GetVelocity() const
{
	return GetPhysicsVelocity() * RPhysicsEngine::GetFixedFrameRate();
}

float PlayerControllerBase::GetMotorAcceleration() const
{
	// m/s^2
	return MotorAcceleration;
}

void PlayerControllerBase::SetMotorAcceleration(float Acceleration)
{
	MotorAcceleration = Acceleration;
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

	KinematicCharacterController->ResetTransformInterpolation();

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
