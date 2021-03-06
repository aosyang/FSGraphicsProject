//=============================================================================
// RgRubik.cpp by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#include "RgRubik.h"

#define FloatNearlyEquals(a,b)	(fabsf((a)-(b))<0.00001f)

const float BlockSpacing = 80.0f;
const float CubeRotatingSpeed = 5.0f;

static RMaterial* RubikMaterials[7];

float EaseInOut(float x)
{
	float sq = x * x;
	return sq / (2.0f * (sq - x) + 1.0f);
}

// Compare if two quaternions are nearly equal
bool QuaternionsNearlyEqual(const RQuat& lhs, const RQuat& rhs)
{
	return (FloatNearlyEquals(lhs.w, rhs.w) && FloatNearlyEquals(lhs.x, rhs.x) && FloatNearlyEquals(lhs.y, rhs.y) && FloatNearlyEquals(lhs.z, rhs.z)) ||
		   (FloatNearlyEquals(lhs.w, -rhs.w) && FloatNearlyEquals(lhs.x, -rhs.x) && FloatNearlyEquals(lhs.y, -rhs.y) && FloatNearlyEquals(lhs.z, -rhs.z));
}

float GetNearestSpacingValue(float f)
{
	float Fraction = f / BlockSpacing + 0.5f;
	Fraction = floorf(Fraction);

	return Fraction * BlockSpacing;
}

// All possible rotations of a single block in rubik cube game
// Used to check and adjust block to closest rotation
RQuat BlockRotations[] 
{
	RQuat(-1.000000f, 0.000000f, -0.000000f, 0.000000f),
	RQuat(-0.707107f, -0.707107f, 0.000000f, 0.000000f),
	RQuat(0.000000f, -1.000000f, 0.000000f, 0.000000f),
	RQuat(0.707107f, -0.707107f, 0.000000f, 0.000000f),
	RQuat(-0.707107f, 0.000000f, 0.707107f, 0.000000f),
	RQuat(-0.500000f, -0.500000f, 0.500000f, -0.500000f),
	RQuat(-0.000000f, -0.707107f, 0.000000f, -0.707107f),
	RQuat(0.500000f, -0.500000f, -0.500000f, -0.500000f),
	RQuat(0.000000f, 0.000000f, 1.000000f, 0.000000f),
	RQuat(-0.000000f, 0.000000f, 0.707107f, -0.707107f),
	RQuat(-0.000000f, 0.000000f, -0.000000f, -1.000000f),
	RQuat(-0.000000f, -0.000000f, -0.707107f, -0.707107f),
	RQuat(0.707107f, 0.000000f, 0.707107f, 0.000000f),
	RQuat(0.500000f, 0.500000f, 0.500000f, -0.500000f),
	RQuat(-0.000000f, 0.707107f, -0.000000f, -0.707107f),
	RQuat(-0.500000f, 0.500000f, -0.500000f, -0.500000f),
	RQuat(-0.707107f, 0.000000f, 0.000000f, -0.707107f),
	RQuat(-0.500000f, -0.500000f, -0.500000f, -0.500000f),
	RQuat(-0.000000f, -0.707107f, -0.707107f, 0.000000f),
	RQuat(0.500000f, -0.500000f, -0.500000f, 0.500000f),
	RQuat(-0.000000f, -0.707107f, 0.707107f, 0.000000f),
	RQuat(0.500000f, -0.500000f, 0.500000f, -0.500000f),
	RQuat(0.707107f, 0.000000f, 0.000000f, -0.707107f),
	RQuat(0.500000f, 0.500000f, -0.500000f, -0.500000f), 
};

//////////////////////////////////////////////////////////////////////////
// RgColorPiece
//////////////////////////////////////////////////////////////////////////

IMPLEMENT_SCENE_OBJECT(RgColorPiece);

RgColorPiece::RgColorPiece(const RConstructingParams& Params)
	: Base(Params)
{
	RMesh* CubeMesh = RResourceManager::Instance().LoadResource<RMesh>("/RubikCube/RoundedCornerCube.fbx");

	MeshComponent = AddNewComponent<RRenderMeshComponent>();
	MeshComponent->SetMesh(CubeMesh);
}

void RgColorPiece::Update(float DeltaTime)
{
	Base::Update(DeltaTime);
}

void RgColorPiece::SetColor(EPieceColor Color)
{
	MeshComponent->SetMaterial(0, RubikMaterials[(int)Color]);
}

void InitializeRubikMaterials()
{
	const std::string MaterialPaths[] =
	{
		"/RubikCube/Blue.material",
		"/RubikCube/Green.material",
		"/RubikCube/Red.material",
		"/RubikCube/Orange.material",
		"/RubikCube/White.material",
		"/RubikCube/Yellow.material",
		"/RubikCube/Black.material",
	};

	for (int i = 0; i < ARRAYSIZE(MaterialPaths); i++)
	{
		RubikMaterials[i] = RResourceManager::Instance().LoadResource<RMaterial>(MaterialPaths[i], EResourceLoadMode::Immediate);
	}
}

//////////////////////////////////////////////////////////////////////////
// RgCubeBlock
//////////////////////////////////////////////////////////////////////////

IMPLEMENT_SCENE_OBJECT(RgCubeBlock);

RgCubeBlock::RgCubeBlock(const RConstructingParams& Params)
	: Base(Params),
	  m_Parent(nullptr)
{
	RMesh* CubeMesh = RResourceManager::Instance().LoadResource<RMesh>("/RubikCube/RoundedCornerCube.fbx");

	RRenderMeshComponent* MeshComponent = AddNewComponent<RRenderMeshComponent>();
	MeshComponent->SetMesh(CubeMesh);
	MeshComponent->SetMaterial(0, RubikMaterials[(int)EPieceColor::Black]);
}

RgCubeBlock::~RgCubeBlock()
{

}

void RgCubeBlock::AttachToCenterBlock(RgCubeBlock* CenterBlock)
{
	assert(CenterBlock);

	m_Parent = CenterBlock;
	RMatrix4 InvParentMatrix = CenterBlock->GetTransform()->GetMatrix().FastInverse();
	RMatrix4 LocalMatrix = m_NodeTransform.GetMatrix() * InvParentMatrix;
	m_LocalTransform.FromMatrix4(LocalMatrix);
}

void RgCubeBlock::DetachFromCenterBlock()
{
	assert(m_Parent);

	m_Parent = nullptr;
}

void RgCubeBlock::FixTransformInaccuracy()
{
	// Fix inaccuracy after transformation
	RVec3 Position = m_NodeTransform.GetPosition();
	RQuat Rotation = m_NodeTransform.GetRotation();

	// Check all possible rotations and adjust to a closest one
	for (int i = 0; i < ARRAYSIZE(BlockRotations); i++)
	{
		if (QuaternionsNearlyEqual(Rotation, BlockRotations[i]))
		{
			Rotation = BlockRotations[i];
			break;
		}

		// Not equals to any one, bad rotation!
		if (i == ARRAYSIZE(BlockRotations) - 1)
		{
			// Shouldn't be here
			RQuat NR = Rotation;
			NR.Normalize();

			RLogError("Rotation [%.4f %.4f %.4f %.4f] is not aligned to any of six directions!\n", Rotation.w, Rotation.x, Rotation.y, Rotation.z);
			RLogError("Norm = %f\n", Rotation.Norm());
			RLogError("Normalized rotation = [%.4f %.4f %.4f %.4f]\n", NR.w, NR.x, NR.y, NR.z);

			assert(0);
		}
	}

	// Adjust position to closest aligned ones
	Position.SetX(GetNearestSpacingValue(Position.X()));
	Position.SetY(GetNearestSpacingValue(Position.Y()));
	Position.SetZ(GetNearestSpacingValue(Position.Z()));

	m_NodeTransform = RTransform(Position, Rotation);
}

void RgCubeBlock::Update(float DeltaTime)
{
	// Update transform based on parent
	if (m_Parent)
	{
		m_NodeTransform = RTransform::Combine(&m_LocalTransform, m_Parent->GetTransform());
	}

	Base::Update(DeltaTime);
}

void RgCubeBlock::SetupColors(int x, int y, int z)
{
	const float SideOffset = BlockSpacing * 0.5f;

	// Create side pieces based on block's position
	if (x == 2)
	{
		RgColorPiece* RightPiece = GetScene()->CreateSceneObjectOfType<RgColorPiece>();
		RightPiece->SetTransform(RVec3(SideOffset, 0, 0), RQuat::IDENTITY, RVec3(0.02f, 0.9f, 0.9f));
		RightPiece->GetTransform()->Attach(GetTransform());
		RightPiece->SetColor(EPieceColor::Blue);
	}
	else if (x == 0)
	{
		RgColorPiece* LeftPiece = GetScene()->CreateSceneObjectOfType<RgColorPiece>();
		LeftPiece->SetTransform(RVec3(-SideOffset, 0, 0), RQuat::IDENTITY, RVec3(0.02f, 0.9f, 0.9f));
		LeftPiece->GetTransform()->Attach(GetTransform());
		LeftPiece->SetColor(EPieceColor::Green);
	}

	if (y == 2)
	{
		RgColorPiece* TopPiece = GetScene()->CreateSceneObjectOfType<RgColorPiece>();
		TopPiece->SetTransform(RVec3(0, SideOffset, 0), RQuat::IDENTITY, RVec3(0.9f, 0.02f, 0.9f));
		TopPiece->GetTransform()->Attach(GetTransform());
		TopPiece->SetColor(EPieceColor::Red);
	}
	else if (y == 0)
	{
		RgColorPiece* BottomPiece = GetScene()->CreateSceneObjectOfType<RgColorPiece>();
		BottomPiece->SetTransform(RVec3(0, -SideOffset, 0), RQuat::IDENTITY, RVec3(0.9f, 0.02f, 0.9f));
		BottomPiece->GetTransform()->Attach(GetTransform());
		BottomPiece->SetColor(EPieceColor::Orange);
	}

	if (z == 2)
	{
		RgColorPiece* FrontPiece = GetScene()->CreateSceneObjectOfType<RgColorPiece>();
		FrontPiece->SetTransform(RVec3(0, 0, SideOffset), RQuat::IDENTITY, RVec3(0.9f, 0.9f, 0.02f));
		FrontPiece->GetTransform()->Attach(GetTransform());
		FrontPiece->SetColor(EPieceColor::White);
	}
	else if (z == 0)
	{
		RgColorPiece* BottomPiece = GetScene()->CreateSceneObjectOfType<RgColorPiece>();
		BottomPiece->SetTransform(RVec3(0, 0, -SideOffset), RQuat::IDENTITY, RVec3(0.9f, 0.9f, 0.02f));
		BottomPiece->GetTransform()->Attach(GetTransform());
		BottomPiece->SetColor(EPieceColor::Yellow);
	}
}

//////////////////////////////////////////////////////////////////////////
// RgRubikCube
//////////////////////////////////////////////////////////////////////////

IMPLEMENT_SCENE_OBJECT(RgRubik);

RgRubik::RgRubik(const RConstructingParams& Params)
	: Base(Params),
	  m_CenterOfMove(nullptr),
	  m_MoveAnimationProgress(1.0f),
	  m_FinishedMove(false)
{
	InitializeRubikMaterials();

	// Create blocks for the rubik cube
	for (int x = 0; x < 3; x++)
	{
		for (int y = 0; y < 3; y++)
		{
			for (int z = 0; z < 3; z++)
			{
				int BlockIndex = x * 9 + y * 3 + z;
				m_Blocks[BlockIndex] = GetScene()->CreateSceneObjectOfType<RgCubeBlock>();
				m_Blocks[BlockIndex]->SetPosition(RVec3((float)x - 1, (float)y - 1, (float)z - 1) * BlockSpacing);
				m_Blocks[BlockIndex]->SetupColors(x, y, z);
			}
		}
	}
}


RgRubik::~RgRubik()
{
}

void RgRubik::Update(float DeltaTime)
{
	Base::Update(DeltaTime);

	if (m_FinishedMove)
	{
		m_FinishedMove = false;

		FinishCurrentMove();
	}

	if (m_MoveAnimationProgress < 1.0f && m_CenterOfMove)
	{
		float DeltaTime = GEngine.GetTimer().DeltaTime() * CubeRotatingSpeed;
		m_MoveAnimationProgress += DeltaTime;
		if (m_MoveAnimationProgress >= 1.0f)
		{
			m_MoveAnimationProgress = 1.0f;
			m_FinishedMove = true;
		}

		// Rotate center block
		float t = EaseInOut(m_MoveAnimationProgress);
		m_CenterOfMove->SetRotation(RQuat::Slerp(m_SourceRotation, m_TargetRotation, t));
	}
}

bool RgRubik::IsMoveInProcess() const
{
	return (m_MoveAnimationProgress < 1.0f || m_CenterOfMove != nullptr);
}

void RgRubik::FinishCurrentMove()
{
	for (auto Block : m_MoveBlocks)
	{
		if (Block != m_CenterOfMove)
		{
			Block->DetachFromCenterBlock();
			Block->FixTransformInaccuracy();
		}
	}
	m_MoveBlocks.clear();

	m_CenterOfMove->FixTransformInaccuracy();
	m_CenterOfMove = nullptr;

	std::vector<RgCubeBlock*> AllBlocks;
	AllBlocks.assign(m_Blocks, m_Blocks + 27);

	// Also rotate blocks in the array
	for (int x = 0; x < 3; x++)
	{
		for (int y = 0; y < 3; y++)
		{
			for (int z = 0; z < 3; z++)
			{
				int BlockIndex = x * 9 + y * 3 + z;
				RVec3 BlockPosition(BlockSpacing * (x - 1), BlockSpacing * (y - 1), BlockSpacing * (z - 1));

				for (auto Block : AllBlocks)
				{
					RVec3 P = Block->GetPosition();
					if (P.X() == BlockPosition.X() && P.Y() == BlockPosition.Y() && P.Z() == BlockPosition.Z())
					{
						m_Blocks[BlockIndex] = Block;
						break;
					}
				}
			}
		}
	}
}

void RgRubik::Rotate(ERubikSide Side, ERubikRotation Rotation)
{
	// A moving is in progress, abort
	if (m_CenterOfMove)
	{
		return;
	}

	m_LastMoveSide = Side;
	m_LastMoveRotation = Rotation;;

	m_MoveBlocks.clear();

	// Get all blocks involved in the move
	for (int x = 0; x < 3; x++)
	{
		for (int y = 0; y < 3; y++)
		{
			for (int z = 0; z < 3; z++)
			{
				int BlockIndex = x * 9 + y * 3 + z;

				bool Result = false;
				switch (Side)
				{
				case ERubikSide::East:		Result = (x == 2);	break;
				case ERubikSide::West:		Result = (x == 0);	break;
				case ERubikSide::Top:		Result = (y == 2);	break;
				case ERubikSide::Bottom:	Result = (y == 0);	break;
				case ERubikSide::North:		Result = (z == 2);	break;
				case ERubikSide::South:		Result = (z == 0);	break;
				default:										break;
				}

				if (Result)
				{
					m_MoveBlocks.push_back(m_Blocks[BlockIndex]);
				}
			}
		}
	}

	assert(m_MoveBlocks.size() == 9);
	m_CenterOfMove = m_MoveBlocks[4];

	for (int i = 0; i < 9; i++)
	{
		if (i != 4)
		{
			m_MoveBlocks[i]->AttachToCenterBlock(m_CenterOfMove);
		}
	}

	static const RVec3 Rots[] =
	{
		RVec3(DEG_TO_RAD(90), 0, 0),
		RVec3(DEG_TO_RAD(-90), 0, 0),
		RVec3(0, DEG_TO_RAD(90), 0),
		RVec3(0, DEG_TO_RAD(-90), 0),
		RVec3(0, 0, DEG_TO_RAD(90)),
		RVec3(0, 0, DEG_TO_RAD(-90)),
	};

	m_SourceRotation = m_CenterOfMove->GetTransform()->GetRotation();
	m_TargetRotation = m_SourceRotation * RQuat::Euler(Rots[(int)Side] * (Rotation == ERubikRotation::CW ? 1.0f : -1.0f));

	m_MoveAnimationProgress = 0.0f;
}
