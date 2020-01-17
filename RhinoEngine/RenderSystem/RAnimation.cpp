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
		if (CurrentPlaybackTime >= Animation->GetEndTime() - 1)
		{
			CurrentPlaybackTime = Animation->GetStartTime();
		}
		RVec3 start_offset = Animation->GetRootPosition(CurrentPlaybackTime);

		CurrentPlaybackTime += deltaTime * Animation->GetFrameRate() * TimeScale;
		bool startOver = false;

		if (CurrentPlaybackTime >= Animation->GetEndTime() - 1)
		{
			if (Animation->IsLooping())
			{
				float AnimDuration = Animation->GetEndTime() - Animation->GetStartTime() - 1;

				if (AnimDuration == 0.0f)
				{
					// If the animation has only one frame, always loop the first frame
					CurrentPlaybackTime = Animation->GetStartTime();
					startOver = true;
				}
				else
				{
					do
					{
						CurrentPlaybackTime -= AnimDuration;
						startOver = true;
					} while (CurrentPlaybackTime >= Animation->GetEndTime() - 1);
				}
			}
			else
			{
				CurrentPlaybackTime = Animation->GetEndTime() - 1;
				IsAnimDone = true;
			}
		}

		if (Animation->GetBitFlags() & AnimBitFlag_HasRootMotion)
		{
			RootOffset = Animation->GetRootPosition(CurrentPlaybackTime) - start_offset;
			if (startOver)
			{
				RootOffset = Animation->GetRootPosition(Animation->GetEndTime() - 1) - start_offset +
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

bool RAnimationBlender::GetCurrentBlendedNodePose(int SourceNodeId, int TargetNodeId, RMatrix4* OutMatrix)
{
	if (m_SourceAnimation.Animation && m_TargetAnimation.Animation)
	{
		RMatrix4 mat1, mat2;
		m_SourceAnimation.Animation->GetNodePose(SourceNodeId, m_SourceAnimation.CurrentPlaybackTime, &mat1);
		m_TargetAnimation.Animation->GetNodePose(TargetNodeId, m_TargetAnimation.CurrentPlaybackTime, &mat2);

		// Apply inversed root translation
		if (m_SourceAnimation.Animation->HasRootMotion())
		{
			mat1 *= RMatrix4::CreateTranslation(-m_SourceAnimation.Animation->GetRootPosition(m_SourceAnimation.CurrentPlaybackTime));
		}

		if (m_TargetAnimation.Animation->HasRootMotion())
		{
			mat2 *= RMatrix4::CreateTranslation(-m_TargetAnimation.Animation->GetRootPosition(m_TargetAnimation.CurrentPlaybackTime));
		}

		float t = min(1.0f, m_ElapsedBlendTime / m_BlendTime);
		*OutMatrix = RMatrix4::Lerp(mat1, mat2, t);

		return true;
	}
	else if (m_SourceAnimation.Animation)
	{
		m_SourceAnimation.Animation->GetNodePose(SourceNodeId, m_SourceAnimation.CurrentPlaybackTime, OutMatrix);

		if (m_SourceAnimation.Animation->HasRootMotion())
		{
			*OutMatrix *= RMatrix4::CreateTranslation(-m_SourceAnimation.Animation->GetRootPosition(m_SourceAnimation.CurrentPlaybackTime));
		}

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
	: m_Flags(0), m_FrameCount(frameCount), m_StartTime(startTime), m_EndTime(endTime), m_FrameRate(frameRate), m_RootNode(-1)
{
	m_NodeKeyFrames = std::vector<RMatrix4*>(nodeCount, nullptr);
	m_NodeParents = std::vector<int>(nodeCount, -1);
	m_NodeNames = std::vector<std::string>(nodeCount, "");
}


RAnimation::~RAnimation()
{
	for (unsigned int i = 0; i < m_NodeKeyFrames.size(); i++)
	{
		if (m_NodeKeyFrames[i])
		{
			delete [] m_NodeKeyFrames[i];
		}
	}
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
	serializer.SerializeVector(m_NodeNames, &RSerializer::SerializeData);
	serializer.SerializeVector(m_NodeParents);

	if (serializer.IsReading())
		m_NodeKeyFrames.resize(m_NodeNames.size());

	for (size_t i = 0; i < m_NodeKeyFrames.size(); i++)
	{
		serializer.SerializeArray(&m_NodeKeyFrames[i], m_FrameCount);
	}
}

void RAnimation::AddNodePose(int nodeId, int frameId, const RMatrix4* matrix)
{
	assert(nodeId >= 0 && nodeId < (int)m_NodeKeyFrames.size());
	assert(frameId >= 0 && frameId < m_FrameCount);

	if (!m_NodeKeyFrames[nodeId])
	{
		m_NodeKeyFrames[nodeId] = new RMatrix4[m_FrameCount];
	}

	memcpy(&m_NodeKeyFrames[nodeId][frameId], matrix, sizeof(RMatrix4));
}

void RAnimation::GetNodePose(int NodeId, float Time, RMatrix4* OutMatrix) const
{
	// Make zero based time
	Time -= m_StartTime;

	assert(NodeId >= 0 && NodeId < (int)m_NodeKeyFrames.size());
	assert(Time >= 0 && Time < m_FrameCount);

	int frame1 = (int)Time;
	int frame2 = ((int)Time + 1) % m_FrameCount;
	//if (frame2 < frame1)
	//	frame2 = frame1;
	float t = Time - frame1;

	// Linear interpolate two matrices
	// TODO: Use quaternion slerp and position lerp instead!
	for (int i = 0; i < 16; i++)
	{
		float va = ((float*)&m_NodeKeyFrames[NodeId][frame1])[i];
		float vb = ((float*)&m_NodeKeyFrames[NodeId][frame2])[i];
		((float*)OutMatrix)[i] = va + (vb - va) * t;
	}
	//XMMATRIX m1 = XMLoadFloat4x4(&m_NodeKeyFrames[nodeId][frame1]);
	//XMMATRIX m2 = XMLoadFloat4x4(&m_NodeKeyFrames[nodeId][frame2]);
	//XMVECTOR s1, s2, q1, q2, t1, t2;
	//XMMatrixDecompose(&s1, &q1, &t1, m1);
	//XMMatrixDecompose(&s2, &q2, &t2, m2);

	//XMVECTOR s = XMVectorLerp(s1, s2, t);
	//XMVECTOR q = XMQuaternionSlerp(q1, q2, t);
	//XMVECTOR tm = XMVectorLerp(t1, t2, t);

	//XMVECTOR vec_zero = XMLoadFloat3(&XMFLOAT3(0.0f, 0.0f, 0.0f));
	//XMVECTOR quat_identity = XMLoadFloat4(&XMFLOAT4(0, 0, 0, 1));
	//XMMATRIX m = XMMatrixTransformation(vec_zero, quat_identity, s, vec_zero, q, tm);

	//XMStoreFloat4x4(matrix, m);
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
	int frame2 = ((int)time + 1) % m_FrameCount;
	//if (frame2 < frame1)
	//	frame2 = frame1;
	float t = time - frame1;

	RVec3 va = m_RootDisplacement[frame1];
	RVec3 vb = m_RootDisplacement[frame2];
	return RVec3::Lerp(va, vb, t);
}

void RAnimation::SetParentId(int nodeId, int parentId)
{
	m_NodeParents[nodeId] = parentId;
}

int RAnimation::GetParentId(int nodeId) const
{
	return m_NodeParents[nodeId];
}

void RAnimation::AddNodeNameToId(const char* nodeName, int nodeId)
{
	m_NodeNames[nodeId] = nodeName;
}

int RAnimation::GetNodeIdByName(const char* nodeName) const
{
	for (std::vector<std::string>::const_iterator iter = m_NodeNames.begin(); iter != m_NodeNames.end(); iter++)
	{
		if (strcmp(iter->c_str(), nodeName) == 0)
			return (int)(iter - m_NodeNames.begin());
	}

	return -1;
}

void RAnimation::BuildRootDisplacementArray()
{
	if (m_RootNode == -1)
		return;

	m_RootDisplacement.resize(m_FrameCount);

	for (int i = 0; i < m_FrameCount; i++)
	{
		m_RootDisplacement[i] = m_NodeKeyFrames[m_RootNode][i].GetTranslation();
		m_RootDisplacement[i].SetY(0.0f);
	}
}