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

		int SourceBoneId = PoseData.SkinnedMesh->GetCachedAnimationNodeId(SourceAnim, i);
		int TargetBondId = PoseData.SkinnedMesh->GetCachedAnimationNodeId(TargetAnim, i);

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
		m_SourceAnimation.Animation->GetNodePoseAtTime(SourceNodeId, m_SourceAnimation.CurrentPlaybackTime, &mat1);
		m_TargetAnimation.Animation->GetNodePoseAtTime(TargetNodeId, m_TargetAnimation.CurrentPlaybackTime, &mat2);

		float t = RMath::Clamp(m_ElapsedBlendTime / m_BlendTime, 0.0f, 1.0f);
		*OutMatrix = RMatrix4::Slerp(mat1, mat2, t);

		return true;
	}
	else if (m_SourceAnimation.Animation)
	{
		m_SourceAnimation.Animation->GetNodePoseAtTime(SourceNodeId, m_SourceAnimation.CurrentPlaybackTime, OutMatrix);
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
	: RAnimation(0, 0, 0.0f, 0.0f, 0.0f)
{

}

RAnimation::RAnimation(int nodeCount, int frameCount, float startTime, float endTime, float frameRate)
	: m_Flags(0)
	, m_FrameCount(frameCount)
	, m_StartTime(startTime)
	, m_EndTime(endTime)
	, m_FrameRate(frameRate)
	, RootSpeed(0.0f)
	, m_RootNode(-1)
{
	BoneNodeData.resize(nodeCount, RAnimBoneData());
	for (int i = 0; i < nodeCount; i++)
	{
		BoneNodeData[i].FrameMatrices.resize(frameCount);
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
	serializer.SerializeData(m_RootNode);

	serializer.SerializeVector(BoneNodeData, &RSerializer::SerializeObject);
}

void RAnimation::AddNodePoseAtFrame(int nodeId, int frameId, const RMatrix4* matrix)
{
	assert(nodeId >= 0 && nodeId < GetNodeCount());
	assert(frameId >= 0 && frameId < m_FrameCount);
	assert(frameId < BoneNodeData[nodeId].FrameMatrices.size());

	BoneNodeData[nodeId].FrameMatrices[frameId] = *matrix;
}

void RAnimation::GetNodePoseAtTime(int NodeId, float Time, RMatrix4* OutMatrix) const
{
	// Make zero based time
	Time = RMath::Max(Time - m_StartTime, 0.0f);

	assert(NodeId >= 0 && NodeId < GetNodeCount());
	assert(Time >= 0 && Time < m_FrameCount);

	int frame1 = (int)Time;
	int frame2;
	
	if (IsLooping())
	{
		frame2 = ((int)Time + 1) % m_FrameCount;
	}
	else
	{
		// Non-looping animations should stay at the last frame when finish playing
		frame2 = RMath::Min((int)Time + 1, m_FrameCount - 1);
	}

	float t = Time - frame1;

	const RMatrix4& Transform1 = BoneNodeData[NodeId].FrameMatrices[frame1];
	const RMatrix4& Transform2 = BoneNodeData[NodeId].FrameMatrices[frame2];

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

#if 0
	// Linear interpolate two matrices
	for (int i = 0; i < 16; i++)
	{
		float va = ((float*)&m_NodeKeyFrames[NodeId][frame1])[i];
		float vb = ((float*)&m_NodeKeyFrames[NodeId][frame2])[i];
		((float*)OutMatrix)[i] = va + (vb - va) * t;
	}
#endif
}

void RAnimation::EvaluatePoseAtTime(RAnimPoseData& PoseData, float Time) const
{
	for (int i = 0; i < PoseData.SkinnedMesh->GetBoneCount(); i++)
	{
		RMatrix4 BoneMatrix;
		int BoneId = PoseData.SkinnedMesh->GetCachedAnimationNodeId(this, i);
		GetNodePoseAtTime(BoneId, Time, &BoneMatrix);

		// Output poses in object space
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

void RAnimation::SetNodeParentId(int NodeId, int ParentId)
{
	BoneNodeData[NodeId].ParentId = ParentId;
}

int RAnimation::GetNodeParentId(int NodeId) const
{
	return BoneNodeData[NodeId].ParentId;
}

void RAnimation::SetNodeName(int NodeId, const char* NodeName)
{
	BoneNodeData[NodeId].BoneName = NodeName;
}

int RAnimation::GetNodeIdByName(const char* nodeName) const
{
	for (auto iter = BoneNodeData.begin(); iter != BoneNodeData.end(); iter++)
	{
		if (strcmp(iter->BoneName.c_str(), nodeName) == 0)
		{
			return (int)(iter - BoneNodeData.begin());
		}
	}

	return -1;
}

void RAnimation::BuildRootDisplacements()
{
	if (m_RootNode == -1)
		return;

	m_RootDisplacement.resize(m_FrameCount);

	for (int i = 0; i < m_FrameCount; i++)
	{
		m_RootDisplacement[i] = BoneNodeData[m_RootNode].FrameMatrices[i].GetTranslation();
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
