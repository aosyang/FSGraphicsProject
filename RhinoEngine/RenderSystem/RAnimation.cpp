//=============================================================================
// RAnimation.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"
#include "RAnimation.h"


RAnimationPlayer::RAnimationPlayer()
	: Animation(nullptr)
	, IsAnimDone(false)
{
}

void RAnimationPlayer::Proceed(float deltaTime)
{
	if (Animation && !IsAnimDone)
	{
		// Changing time may cause start time greater than end time
		if (CurrentPlaybackTime >= Animation->GetEndTime())
		{
			CurrentPlaybackTime = Animation->GetStartTime();
		}

		// Root motion offset at the beginning of this frame
		RVec3 PrevRootOffset = Animation->GetRootPosition(CurrentPlaybackTime);

		CurrentPlaybackTime += deltaTime * Animation->GetFrameRate() * TimeScale;
		bool startOver = false;

		// The playback time has passed the end time of the animation
		if (CurrentPlaybackTime >= Animation->GetEndTime())
		{
			if (Animation->IsLooping())
			{
				float AnimDuration = Animation->GetEndTime() - Animation->GetStartTime();

				if (AnimDuration == 0.0f)
				{
					// If the animation has only one frame, always loop the first frame
					CurrentPlaybackTime = Animation->GetStartTime();
					startOver = true;
				}
				else
				{
					while (CurrentPlaybackTime >= Animation->GetEndTime())
					{
						CurrentPlaybackTime -= AnimDuration;
						startOver = true;
					}
				}
			}
			else
			{
				CurrentPlaybackTime = Animation->GetEndTime() - 0.01f;
				IsAnimDone = true;
			}
		}

		if (Animation->HasRootMotion())
		{
			RootOffset = Animation->GetRootPosition(CurrentPlaybackTime) - PrevRootOffset;
			if (startOver)
			{
				RootOffset = Animation->GetRootPosition(Animation->GetEndTime() - 1) - PrevRootOffset +
							 Animation->GetRootPosition(CurrentPlaybackTime) - Animation->GetInitRootPosition();
			}
		}
		else
		{
			RootOffset = RVec3(0, 0, 0);
		}
	}
}

void RAnimationPlayer::Reset()
{
	IsAnimDone = false;
	CurrentPlaybackTime = Animation ? Animation->GetStartTime() : 0.0f;
}

RAnimationBlender::RAnimationBlender()
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
	m_SourceAnimation.Proceed(deltaTime);

	if (m_TargetAnimation.Animation)
	{
		m_TargetAnimation.Proceed(deltaTime);
		m_ElapsedBlendTime += deltaTime;
		if (m_ElapsedBlendTime >= m_BlendTime)
		{
			m_SourceAnimation = m_TargetAnimation;
			m_TargetAnimation.Animation = nullptr;
		}
	}
}

bool RAnimationBlender::EvaluatePose(RMesh* SkinnedMesh, const RMatrix4& LocalToWorld, RMatrix4* OutBoneMatrices) const
{
	if (!SkinnedMesh)
	{
		return false;
	}

	RAnimation* const SourceAnim = GetSourceAnimation();
	RAnimation* const TargetAnim = GetTargetAnimation();

	for (int i = 0; i < SkinnedMesh->GetBoneCount(); i++)
	{
		RMatrix4 BoneMatrix;

		int SourceBoneId = SkinnedMesh->GetCachedAnimationNodeId(SourceAnim, i);
		int TargetBondId = SkinnedMesh->GetCachedAnimationNodeId(TargetAnim, i);

		bool Result = GetCurrentBlendedNodePose(SourceBoneId, TargetBondId, &BoneMatrix);
		if (!Result)
		{
			BoneMatrix = RMatrix4::Zero;
		}

		OutBoneMatrices[i] = SkinnedMesh->GetBoneInitInvMatrices(i) * BoneMatrix * LocalToWorld;

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

	return true;
}

bool RAnimationBlender::GetCurrentBlendedNodePose(int SourceNodeId, int TargetNodeId, RMatrix4* OutMatrix) const
{
	if (m_SourceAnimation.Animation && m_TargetAnimation.Animation)
	{
		RMatrix4 mat1, mat2;
		m_SourceAnimation.Animation->GetNodePoseAtTime(SourceNodeId, m_SourceAnimation.CurrentPlaybackTime, &mat1);
		m_TargetAnimation.Animation->GetNodePoseAtTime(TargetNodeId, m_TargetAnimation.CurrentPlaybackTime, &mat2);

		float t = min(1.0f, m_ElapsedBlendTime / m_BlendTime);
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

RVec3 RAnimationBlender::GetCurrentRootOffset()
{
	if (m_SourceAnimation.Animation && m_TargetAnimation.Animation)
	{
		float t = min(1.0f, m_ElapsedBlendTime / m_BlendTime);

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
	: m_Flags(0)
{

}

RAnimation::RAnimation(int nodeCount, int frameCount, float startTime, float endTime, float frameRate)
	: m_Flags(0)
	, m_FrameCount(frameCount)
	, m_StartTime(startTime)
	, m_EndTime(endTime)
	, m_FrameRate(frameRate)
	, m_RootNode(-1)
{
	BoneNodeData.resize(nodeCount, RAnimNodeData());
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
	Time -= m_StartTime;

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

	if (HasRootMotion())
	{
		Position1 -= GetRootPosition((float)frame1);
		Position2 -= GetRootPosition((float)frame2);
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
	time -= m_StartTime;

	int frame1 = (int)time;

	// If the animation has reached its end and loops from the beginning,
	// make sure we don't end up taking a large step back with the root offset
	int frame2 = RMath::Min(((int)time + 1), m_FrameCount - 1);
	float t = time - frame1;

	RVec3 va = m_RootDisplacement[frame1];
	RVec3 vb = m_RootDisplacement[frame2];
	return RVec3::Lerp(va, vb, t);
}

void RAnimation::SetParentId(int nodeId, int parentId)
{
	BoneNodeData[nodeId].ParentId = parentId;
}

int RAnimation::GetParentId(int nodeId) const
{
	return BoneNodeData[nodeId].ParentId;
}

void RAnimation::AddNodeNameToId(const char* nodeName, int nodeId)
{
	BoneNodeData[nodeId].BoneName = nodeName;
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
}
