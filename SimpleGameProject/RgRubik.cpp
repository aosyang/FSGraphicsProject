//=============================================================================
// RgRubik.cpp by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#include "RgRubik.h"

#define FloatNearlyEquals(a,b)	(fabsf(a-b)<1.192092896e-06F)

const float BlockSpacing = 80.0f;

static RMaterial RubikMaterials[7];

// Compare if two quaternions are nearly equal
bool QuaternionNearlyEquals(const RQuat& lhs, const RQuat& rhs)
{
	return (FloatNearlyEquals(lhs.w, rhs.w) && FloatNearlyEquals(lhs.x, rhs.x) && FloatNearlyEquals(lhs.y, rhs.y) && FloatNearlyEquals(lhs.z, rhs.z));
}

float GetNearestSpacingValue(float f)
{
	float Fraction = f / BlockSpacing + 0.5f;
	Fraction = floorf(Fraction);

	return Fraction * BlockSpacing;
}

RQuat BlockRotations[] 
{
	RQuat::Euler(0, 0, 0),
	RQuat::Euler(DEG_TO_RAD(90), 0, 0),
	RQuat::Euler(DEG_TO_RAD(180), 0, 0),
	RQuat::Euler(DEG_TO_RAD(270), 0, 0),
	RQuat::Euler(0, DEG_TO_RAD(90), 0),
	RQuat::Euler(0, DEG_TO_RAD(180), 0),
	RQuat::Euler(0, DEG_TO_RAD(270), 0),
	RQuat::Euler(0, 0, DEG_TO_RAD(90)),
	RQuat::Euler(0, 0, DEG_TO_RAD(180)),
	RQuat::Euler(0, 0, DEG_TO_RAD(270)),
};

//////////////////////////////////////////////////////////////////////////
// RgColorPiece
//////////////////////////////////////////////////////////////////////////

RgColorPiece::RgColorPiece(RScene* InScene)
	: Base(InScene)
{
	RMesh* CubeMesh = RResourceManager::Instance().LoadFbxMesh("../Assets/cube.rmesh");

	MeshComponent = AddNewComponent<RRenderMeshComponent>();
	MeshComponent->SetMesh(CubeMesh);
}

void RgColorPiece::Update()
{
	Base::Update();
}

void RgColorPiece::SetColor(EPieceColor Color)
{
	MeshComponent->SetMaterial(0, RubikMaterials[(int)Color]);
}

void InitializeRubikMaterials()
{
	const string MaterialFilenames[] =
	{
		"../Assets/RubikCube/Blue.rmtl",
		"../Assets/RubikCube/Green.rmtl",
		"../Assets/RubikCube/Red.rmtl",
		"../Assets/RubikCube/Orange.rmtl",
		"../Assets/RubikCube/White.rmtl",
		"../Assets/RubikCube/Yellow.rmtl",
		"../Assets/RubikCube/Black.rmtl",
	};

	for (int i = 0; i < 7; i++)
	{
		vector<RMaterial> Materials;
		RMaterial::LoadFromXMLFile(MaterialFilenames[i], Materials);
		assert(Materials.size() >= 0);
		RubikMaterials[i] = Materials[0];
	}
}

//////////////////////////////////////////////////////////////////////////
// RgCubeBlock
//////////////////////////////////////////////////////////////////////////

RgCubeBlock::RgCubeBlock(RScene* InScene)
	: Base(InScene),
	  m_Parent(nullptr)
{
	RMesh* CubeMesh = RResourceManager::Instance().LoadFbxMesh("../Assets/cube.rmesh");

	RRenderMeshComponent* MeshComponent = AddNewComponent<RRenderMeshComponent>();
	MeshComponent->SetMesh(CubeMesh);
	MeshComponent->SetMaterial(0, RubikMaterials[(int)EPieceColor::Black]);
}

RgCubeBlock::~RgCubeBlock()
{

}

void RgCubeBlock::AttachToBlock(RgCubeBlock* Parent)
{
	assert(Parent);

	m_Parent = Parent;
	RMatrix4 InvParentMatrix = Parent->GetTransform()->GetMatrix().FastInverse();
	RMatrix4 LocalMatrix = m_NodeTransform.GetMatrix() * InvParentMatrix;
	m_LocalTransform.FromMatrix4(LocalMatrix);
}

void RgCubeBlock::Detach()
{
	assert(m_Parent);

	m_Parent = nullptr;
}

void RgCubeBlock::FixTransformInaccuracy()
{
	// Fix inaccuracy after transformation
	RVec3 Position = m_NodeTransform.GetPosition();
	RQuat Rotation = m_NodeTransform.GetRotation();

	RQuat NR = Rotation;
	NR.Normalize();

	for (int i = 0; i < ARRAYSIZE(BlockRotations); i++)
	{
		if (QuaternionNearlyEquals(Rotation, BlockRotations[i]))
		{
			Rotation = BlockRotations[i];
			break;
		}

		if (i == ARRAYSIZE(BlockRotations))
		{
			// Shouldn't be here
			assert(0);
		}
	}

	Position.SetX(GetNearestSpacingValue(Position.X()));
	Position.SetY(GetNearestSpacingValue(Position.Y()));
	Position.SetZ(GetNearestSpacingValue(Position.Z()));

	m_NodeTransform = RTransform(Position, Rotation);
}

void RgCubeBlock::Update()
{
	// Update transform based on parent
	if (m_Parent)
	{
		m_NodeTransform = RTransform::Combine(&m_LocalTransform, m_Parent->GetTransform());
	}

	Base::Update();
}

void RgCubeBlock::SetupColors(int x, int y, int z)
{
	const float SideOffset = BlockSpacing * 0.5f;

	if (x == 2)
	{
		RgColorPiece* RightPiece = GetScene()->CreateSceneObjectOfType<RgColorPiece>();
		RightPiece->SetTransform(RVec3(SideOffset, 0, 0), RQuat::IDENTITY, RVec3(0.1f, 0.9f, 0.9f));
		RightPiece->GetTransform()->Attach(GetTransform());
		RightPiece->SetColor(EPieceColor::Blue);
	}
	else if (x == 0)
	{
		RgColorPiece* LeftPiece = GetScene()->CreateSceneObjectOfType<RgColorPiece>();
		LeftPiece->SetTransform(RVec3(-SideOffset, 0, 0), RQuat::IDENTITY, RVec3(0.1f, 0.9f, 0.9f));
		LeftPiece->GetTransform()->Attach(GetTransform());
		LeftPiece->SetColor(EPieceColor::Green);
	}

	if (y == 2)
	{
		RgColorPiece* TopPiece = GetScene()->CreateSceneObjectOfType<RgColorPiece>();
		TopPiece->SetTransform(RVec3(0, SideOffset, 0), RQuat::IDENTITY, RVec3(0.9f, 0.1f, 0.9f));
		TopPiece->GetTransform()->Attach(GetTransform());
		TopPiece->SetColor(EPieceColor::Red);
	}
	else if (y == 0)
	{
		RgColorPiece* BottomPiece = GetScene()->CreateSceneObjectOfType<RgColorPiece>();
		BottomPiece->SetTransform(RVec3(0, -SideOffset, 0), RQuat::IDENTITY, RVec3(0.9f, 0.1f, 0.9f));
		BottomPiece->GetTransform()->Attach(GetTransform());
		BottomPiece->SetColor(EPieceColor::Orange);
	}

	if (z == 2)
	{
		RgColorPiece* FrontPiece = GetScene()->CreateSceneObjectOfType<RgColorPiece>();
		FrontPiece->SetTransform(RVec3(0, 0, SideOffset), RQuat::IDENTITY, RVec3(0.9f, 0.9f, 0.1f));
		FrontPiece->GetTransform()->Attach(GetTransform());
		FrontPiece->SetColor(EPieceColor::White);
	}
	else if (z == 0)
	{
		RgColorPiece* BottomPiece = GetScene()->CreateSceneObjectOfType<RgColorPiece>();
		BottomPiece->SetTransform(RVec3(0, 0, -SideOffset), RQuat::IDENTITY, RVec3(0.9f, 0.9f, 0.1f));
		BottomPiece->GetTransform()->Attach(GetTransform());
		BottomPiece->SetColor(EPieceColor::Yellow);
	}
}

//////////////////////////////////////////////////////////////////////////
// RgRubikCube
//////////////////////////////////////////////////////////////////////////

RgRubik::RgRubik(RScene* InScene)
	: Base(InScene),
	  m_CenterOfMove(nullptr),
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

void RgRubik::Update()
{
	Base::Update();

	if (m_FinishedMove)
	{
		m_FinishedMove = false;

		FinishCurrentMove();
	}

	if (m_MoveAnimationRatio < 1.0f && m_CenterOfMove)
	{
		float DeltaTime = GEngine.GetTimer().DeltaTime() * 2.0f;
		m_MoveAnimationRatio += DeltaTime;
		if (m_MoveAnimationRatio >= 1.0f)
		{
			m_MoveAnimationRatio = 1.0f;
			m_FinishedMove = true;
		}

		// Rotate center block
		m_CenterOfMove->SetRotation(RQuat::Slerp(m_SourceRotation, m_TargetRotation, m_MoveAnimationRatio));
	}
}

void RgRubik::FinishCurrentMove()
{
	for (auto Block : m_MoveBlocks)
	{
		if (Block != m_CenterOfMove)
		{
			Block->Detach();
			Block->FixTransformInaccuracy();
		}
	}
	m_MoveBlocks.clear();

	m_CenterOfMove->FixTransformInaccuracy();
	m_CenterOfMove = nullptr;

	vector<RgCubeBlock*> AllBlocks;
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
			m_MoveBlocks[i]->AttachToBlock(m_CenterOfMove);
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

	m_MoveAnimationRatio = 0.0f;
}
