//=============================================================================
// RAnimation.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "RAnimation.h"

#include "RenderSystem/RMesh.h"
#include "RAnimNode_Base.h"
#include "Resource/RResourceManager.h"

RAnimationBlender::RAnimationBlender()
	: m_BlendTime(0.2f)
	, m_ElapsedBlendTime(0.0f)
{

}

void RAnimationBlender::Play(RAnimation* Animation, float StartTime /*= -1.0f*/, float TimeScale /*= 1.0f */)
{
	m_SourceAnimation.Animation = Animation;
	m_SourceAnimation.CurrentPlaybackTime = StartTime >= 0.0f ? StartTime : Animation->GetStartTime();
	m_SourceAnimation.TimeScale = TimeScale;
	m_SourceAnimation.IsAnimDone = false;

	m_TargetAnimation.Animation = nullptr;

	// Reset blend time
	m_BlendTime = 0.0f;
	m_ElapsedBlendTime = 0.0f;
}

void RAnimationBlender::Blend(RAnimation* SourceAnimation, float SourceStartTime /*= -1.0f*/, float SourceTimeScale /*= 1.0f*/, RAnimation* TargetAnimation /*= nullptr*/, float TargetStartTime /*= -1.0f*/, float TargetTimeScale /*= 0.0f*/, float BlendTime /*= 0.0f*/)
{
	m_SourceAnimation.Animation = SourceAnimation;
	m_SourceAnimation.CurrentPlaybackTime = SourceStartTime >= 0.0f ? SourceStartTime : SourceAnimation->GetStartTime();
	m_SourceAnimation.TimeScale = SourceTimeScale;
	m_SourceAnimation.IsAnimDone = false;

	m_TargetAnimation.Animation = TargetAnimation;
	m_TargetAnimation.CurrentPlaybackTime = TargetStartTime >= 0.0f ? TargetStartTime : TargetAnimation->GetStartTime();
	m_TargetAnimation.TimeScale = TargetTimeScale;
	m_TargetAnimation.IsAnimDone = false;

	m_BlendTime = BlendTime;
	m_ElapsedBlendTime = 0.0f;
}

void RAnimationBlender::BlendOutTo(RAnimation* TargetAnimation, float TargetStartTime /*= -1.0f*/, float TargetTimeScale /*= 1.0f*/, float BlendTime /*= 0.0f*/)
{
	if (BlendTime == 0.0f)
	{
		Play(TargetAnimation, TargetStartTime, TargetTimeScale);
	}
	else
	{
		if (m_TargetAnimation.Animation)
		{
			m_SourceAnimation.Animation = m_TargetAnimation.Animation;
			m_SourceAnimation.CurrentPlaybackTime = m_TargetAnimation.CurrentPlaybackTime;
			m_SourceAnimation.TimeScale = m_TargetAnimation.TimeScale;
			m_SourceAnimation.IsAnimDone = m_TargetAnimation.IsAnimDone;
		}

		m_TargetAnimation.Animation = TargetAnimation;
		m_TargetAnimation.CurrentPlaybackTime = TargetStartTime >= 0.0f ? TargetStartTime : TargetAnimation->GetStartTime();
		m_TargetAnimation.TimeScale = TargetTimeScale;
		m_TargetAnimation.IsAnimDone = false;

		m_BlendTime = BlendTime;
		m_ElapsedBlendTime = 0.0f;
	}
}

void RAnimationBlender::ProceedAnimation(float deltaTime)
{
	m_SourceAnimation.UpdateNode(deltaTime);

	if (m_TargetAnimation.Animation)
	{
		m_TargetAnimation.UpdateNode(deltaTime);
		m_ElapsedBlendTime += deltaTime;
		if (m_ElapsedBlendTime >= m_BlendTime)
		{
			m_SourceAnimation = m_TargetAnimation;
			m_TargetAnimation.Animation = nullptr;
		}
	}
}

void RAnimationBlender::EvaluatePose(RAnimPoseData& PoseData) const
{
	RAnimation* const SourceAnim = GetSourceAnimation();
	RAnimation* const TargetAnim = GetTargetAnimation();

	for (int i = 0; i < PoseData.SkinnedMesh->GetBoneCount(); i++)
	{
		RMatrix4 BoneMatrix;

		int SourceBoneId = PoseData.SkinnedMesh->ConvertBoneIndex_MeshToAnimation(SourceAnim, i);
		int TargetBondId = PoseData.SkinnedMesh->ConvertBoneIndex_MeshToAnimation(TargetAnim, i);

		bool Result = GetCurrentBlendedNodePose(SourceBoneId, TargetBondId, &BoneMatrix);
		if (!Result)
		{
			BoneMatrix = RMatrix4::Zero;
		}

		// Output poses in object space
		PoseData.BoneMatrices[i] = BoneMatrix;

#if 0	// Debug rendering bones
		{
			int ParentId = SourceAnim->GetParentId(i);
			if (ParentId != -1)
			{
				RMatrix4 ParentMatrix;
				if (!GetCurrentBlendedNodePose(ParentId, ParentId, &ParentMatrix))
				{
					ParentMatrix = RMatrix4::Zero;
				}

				GDebugRenderer.DrawLine(BoneMatrix.GetTranslation(), ParentMatrix.GetTranslation());
			}
		}
#endif
	}
}

bool RAnimationBlender::GetCurrentBlendedNodePose(int SourceNodeId, int TargetNodeId, RMatrix4* OutMatrix) const
{
	if (m_SourceAnimation.Animation && m_TargetAnimation.Animation)
	{
		RMatrix4 mat1, mat2;
		m_SourceAnimation.Animation->GetMeshSpaceBoneMatrixAtTime(SourceNodeId, m_SourceAnimation.CurrentPlaybackTime, &mat1);
		m_TargetAnimation.Animation->GetMeshSpaceBoneMatrixAtTime(TargetNodeId, m_TargetAnimation.CurrentPlaybackTime, &mat2);

		float t = RMath::Clamp(m_ElapsedBlendTime / m_BlendTime, 0.0f, 1.0f);
		*OutMatrix = RMatrix4::Slerp(mat1, mat2, t);

		return true;
	}
	else if (m_SourceAnimation.Animation)
	{
		m_SourceAnimation.Animation->GetMeshSpaceBoneMatrixAtTime(SourceNodeId, m_SourceAnimation.CurrentPlaybackTime, OutMatrix);
		return true;
	}

	return false;
}

RVec3 RAnimationBlender::GetCurrentRootOffset() const
{
	if (m_SourceAnimation.Animation && m_TargetAnimation.Animation)
	{
		float t = RMath::Clamp(m_ElapsedBlendTime / m_BlendTime, 0.0f, 1.0f);

		return RVec3::Lerp(m_SourceAnimation.RootOffset, m_TargetAnimation.RootOffset, t);
	}
	else if (m_SourceAnimation.Animation)
	{
		return m_SourceAnimation.RootOffset;
	}

	return RVec3::Zero();
}

bool RAnimationBlender::HasFinishedPlaying() const
{
	if (m_TargetAnimation.Animation)
	{
		return m_TargetAnimation.IsAnimDone;
	}

	return (m_SourceAnimation.Animation && m_SourceAnimation.IsAnimDone);
}

RAnimation::RAnimation()
	: RAnimation("", nullptr, 0, 0, 0.0f, 0.0f, 0.0f)
{

}

RAnimation::RAnimation(const std::string& InName, RMesh* InSkeletalMesh, int nodeCount, int frameCount, float startTime, float endTime, float frameRate)
	: m_Name(InName)
	, SkeletalMesh(InSkeletalMesh)
	, m_Flags(0)
	, m_FrameCount(frameCount)
	, m_StartTime(startTime)
	, m_EndTime(endTime)
	, m_FrameRate(frameRate)
	, RootSpeed(0.0f)
{
	BoneNodeData.resize(nodeCount, RAnimBoneData());
	for (int i = 0; i < nodeCount; i++)
	{
		BoneNodeData[i].FrameMatrices_MeshSpace.resize(frameCount);
		BoneNodeData[i].FrameMatrices_LocalSpace.resize(frameCount);
	}
}


RAnimation::~RAnimation()
{
}

void RAnimation::Serialize(RSerializer& serializer)
{
	if (!serializer.EnsureHeader("ANIM", 4))
		return;

	serializer.SerializeData(m_Name);
	serializer.SerializeData(m_Flags);
	serializer.SerializeData(m_FrameCount);
	serializer.SerializeData(m_StartTime);
	serializer.SerializeData(m_EndTime);
	serializer.SerializeData(m_FrameRate);

	serializer.SerializeVector(BoneNodeData, &RSerializer::SerializeObject);
}

void RAnimation::SetMeshSpaceBoneMatrixAtFrame(int BoneId, int FrameId, const RMatrix4& InMatrix)
{
	assert(BoneId >= 0 && BoneId < GetNodeCount());
	assert(FrameId >= 0 && FrameId < m_FrameCount);
	assert(FrameId < BoneNodeData[BoneId].FrameMatrices_MeshSpace.size());

	BoneNodeData[BoneId].FrameMatrices_MeshSpace[FrameId] = InMatrix;
}

void RAnimation::SetLocalSpaceBoneMatrixAtFrame(int BoneId, int FrameId, const RMatrix4& InMatrix)
{
	assert(BoneId >= 0 && BoneId < GetNodeCount());
	assert(FrameId >= 0 && FrameId < m_FrameCount);
	assert(FrameId < BoneNodeData[BoneId].FrameMatrices_LocalSpace.size());

	BoneNodeData[BoneId].FrameMatrices_LocalSpace[FrameId] = InMatrix;
}

void RAnimation::GetMeshSpaceBoneMatrixAtTime(int BoneId, float Time, RMatrix4* OutMatrix) const
{
	assert(BoneId >= 0 && BoneId < GetNodeCount());

	int frame1, frame2;
	float t;
	GetNeighborFramesAtTime(Time, frame1, frame2, t);

	const RMatrix4& Transform1 = BoneNodeData[BoneId].FrameMatrices_MeshSpace[frame1];
	const RMatrix4& Transform2 = BoneNodeData[BoneId].FrameMatrices_MeshSpace[frame2];

	RVec3 Position1, Position2;
	RQuat Rotation1, Rotation2;
	RVec3 Scale1, Scale2;

	Transform1.Decompose(Position1, Rotation1, Scale1);
	Transform2.Decompose(Position2, Rotation2, Scale2);

	if (HasRootMotion() || IsRootLocked())
	{
		Position1 -= GetRootPositionAtFrame(frame1);
		Position2 -= GetRootPositionAtFrame(frame2);
	}

	RTransform ResultTransform(
		RVec3::Lerp(Position1, Position2, t),
		RQuat::Slerp(Rotation1, Rotation2, t),
		RVec3::Lerp(Scale1, Scale2, t)
	);

	*OutMatrix = ResultTransform.GetMatrix();
}

void RAnimation::GetLocalSpaceBoneMatrixAtTime(int BoneId, float Time, RMatrix4* OutMatrix) const
{
	assert(BoneId >= 0 && BoneId < GetNodeCount());

	int frame1, frame2;
	float t;
	GetNeighborFramesAtTime(Time, frame1, frame2, t);

	const RMatrix4& Transform1 = BoneNodeData[BoneId].FrameMatrices_LocalSpace[frame1];
	const RMatrix4& Transform2 = BoneNodeData[BoneId].FrameMatrices_LocalSpace[frame2];

	RVec3 Position1, Position2;
	RQuat Rotation1, Rotation2;
	RVec3 Scale1, Scale2;

	Transform1.Decompose(Position1, Rotation1, Scale1);
	Transform2.Decompose(Position2, Rotation2, Scale2);

	if (IsRootBone(BoneId))
	{
		if (HasRootMotion() || IsRootLocked())
		{
			Position1 -= GetRootPositionAtFrame(frame1);
			Position2 -= GetRootPositionAtFrame(frame2);
		}
	}

	RTransform ResultTransform(
		RVec3::Lerp(Position1, Position2, t),
		RQuat::Slerp(Rotation1, Rotation2, t),
		RVec3::Lerp(Scale1, Scale2, t)
	);

	*OutMatrix = ResultTransform.GetMatrix();
}

void RAnimation::EvaluatePoseAtTime(RAnimPoseData& PoseData, float Time) const
{
	for (int i = 0; i < PoseData.SkinnedMesh->GetBoneCount(); i++)
	{
		RMatrix4 BoneMatrix;

		// Note: A skinned mesh may have different bone indices than an animation
		// Map animation bone index to skinned mesh bone index
		int BoneId = PoseData.SkinnedMesh->ConvertBoneIndex_MeshToAnimation(this, i);

#if 0
		// All bones are evaluated in mesh space
		// (This is deprecated as local space bone matrices support per bone modification)
		GetMeshSpaceBoneMatrixAtTime(BoneId, Time, &BoneMatrix);
#else
		// Note: Assuming the first bone is the root bone for now.
		//		 Maybe this code need change in the future
		if (i == 0)
		{
			GetMeshSpaceBoneMatrixAtTime(BoneId, Time, &BoneMatrix);
		}
		else
		{
			GetLocalSpaceBoneMatrixAtTime(BoneId, Time, &BoneMatrix);
		}
#endif

		// Output poses in object space
		// Array index is in skinned mesh bone index
		PoseData.BoneMatrices[i] = BoneMatrix;
	}
}

RVec3 RAnimation::GetInitRootPosition() const
{
	if (m_RootDisplacement.size() == 0)
		return RVec3::Zero();

	return m_RootDisplacement[0];
}

RVec3 RAnimation::GetRootPosition(float time) const
{
	if (m_RootDisplacement.size() == 0)
		return RVec3::Zero();

	// Make zero based time
	time = RMath::Max(time - m_StartTime, 0.0f);

	int frame1 = (int)time;

	// If the animation has reached its end and loops from the beginning,
	// make sure we don't end up taking a large step back with the root offset
	int frame2 = RMath::Min(((int)time + 1), m_FrameCount - 1);
	float t = time - frame1;

	RVec3 va = m_RootDisplacement[frame1];
	RVec3 vb = m_RootDisplacement[frame2];
	return RVec3::Lerp(va, vb, t);
}

RVec3 RAnimation::GetRootPositionAtFrame(int FrameId) const
{
	if (m_RootDisplacement.size() == 0)
	{
		return RVec3::Zero();
	}

	return m_RootDisplacement[FrameId];
}

void RAnimation::BuildRootDisplacements()
{
	if (SkeletalMesh == nullptr)
	{
		return;
	}

	int MeshRootNode = SkeletalMesh->GetSkeletalData().GetRootBone();
	int AnimRootNode = SkeletalMesh->ConvertBoneIndex_MeshToAnimation(this, MeshRootNode);
	if (AnimRootNode == -1)
	{
		return;
	}

	m_RootDisplacement.resize(m_FrameCount);

	for (int i = 0; i < m_FrameCount; i++)
	{
		m_RootDisplacement[i] = BoneNodeData[AnimRootNode].FrameMatrices_MeshSpace[i].GetTranslation();
		m_RootDisplacement[i].SetY(0.0f);
	}

	// Calculate root speed by position difference between the first and the last frame
	if (m_FrameCount > 0)
	{
		RootSpeed = (m_RootDisplacement[m_FrameCount - 1] - m_RootDisplacement[0]).Magnitude() / (m_FrameCount / m_FrameRate);
	}
	else
	{
		RootSpeed = 0.0f;
	}
}

int RAnimation::FindAnimBoneIndexByName(const std::string& BoneName) const
{
	for (int i = 0; i < (int)BoneNodeData.size(); i++)
	{
		if (BoneNodeData[i].BoneName == BoneName)
		{
			return i;
		}
	}

	return -1;
}

void RAnimation::SetAnimBoneName(int AnimBoneId, const std::string& BoneName)
{
	BoneNodeData[AnimBoneId].BoneName = BoneName;
}

bool RAnimation::IsRootBone(int BoneId) const
{
	assert(SkeletalMesh);
	int MeshBoneId = SkeletalMesh->ConvertBoneIndex_AnimationToMesh(this, BoneId);
	return SkeletalMesh->GetSkeletalData().FindParentForBone(MeshBoneId) == -1;
}

void RAnimation::SetSkeletalMesh(RMesh* InSkelMesh)
{
	SkeletalMesh = InSkelMesh;
}

void RAnimation::GetNeighborFramesAtTime(float Time, int& OutFrame1, int& OutFrame2, float& OutFactor) const
{
	// Make zero based time
	Time = RMath::Max(Time - m_StartTime, 0.0f);
	assert(Time >= 0 && Time < m_FrameCount);

	OutFrame1 = (int)Time;
	if (IsLooping())
	{
		OutFrame2 = ((int)Time + 1) % m_FrameCount;
	}
	else
	{
		// Non-looping animations should stay at the last frame when finish playing
		OutFrame2 = RMath::Min((int)Time + 1, m_FrameCount - 1);
	}

	// Factor between two frames
	OutFactor = Time - OutFrame1;
}
