//=============================================================================
// RgRubik.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "Rhino.h"

enum class ERubikSide : UINT8
{
	PosX,
	NegX,
	PosY,
	NegY,
	PosZ,
	NegZ,

	East	= PosX,
	West	= NegX,
	Top		= PosY,
	Bottom	= NegY,
	North	= PosZ,
	South	= NegZ,
};

enum class ERubikRotation : UINT8
{
	CW,
	CCW,
};

enum class EPieceColor : UINT8
{
	Blue,
	Green,
	Red,
	Orange,
	White,
	Yellow,
	Black,
};

class RgCubeBlock;

// The color piece on one side of each block
class RgColorPiece : public RSceneObject
{
	DECLARE_SCENE_OBJECT(RSceneObject)
public:
	void Update() override;

	void SetColor(EPieceColor Color);

protected:
	RgColorPiece(const RConstructingParams& Params);

	RRenderMeshComponent* MeshComponent;
};

// The small block that can be rotated freely
class RgCubeBlock : public RSceneObject
{
	DECLARE_SCENE_OBJECT(RSceneObject)
public:
	void Update() override;

	void SetupColors(int x, int y, int z);

	void AttachToCenterBlock(RgCubeBlock* CenterBlock);
	void DetachFromCenterBlock();

	// Eliminate floating point inaccuracy after rotating the block
	void FixTransformInaccuracy();

protected:
	RgCubeBlock(const RConstructingParams& Params);
	~RgCubeBlock();

	RgCubeBlock*	m_Parent;
	RTransform		m_LocalTransform;
};

#define BLOCK_NUMS 27


class RgRubik : public RSceneObject
{
	DECLARE_SCENE_OBJECT(RSceneObject)
public:

	void Update() override;

	bool IsMoveInProcess() const;
	void FinishCurrentMove();

	void Rotate(ERubikSide Side, ERubikRotation Rotation);

protected:
	RgRubik(const RConstructingParams& Params);
	~RgRubik();

	union
	{
		RgCubeBlock*		m_Blocks[BLOCK_NUMS];
		RgCubeBlock*		m_Block3x3x3[3][3][3];	// x, y, z
	};
	vector<RgCubeBlock*>	m_MoveBlocks;
	RgCubeBlock*			m_CenterOfMove;

	RQuat					m_SourceRotation;
	RQuat					m_TargetRotation;

	float					m_MoveAnimationProgress;
	bool					m_FinishedMove;

	ERubikSide				m_LastMoveSide;
	ERubikRotation			m_LastMoveRotation;
};

