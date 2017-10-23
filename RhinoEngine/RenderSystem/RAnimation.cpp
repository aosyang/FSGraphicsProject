//=============================================================================
// RAnimation.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"
#include "RAnimation.h"


RAnimationPlayer::RAnimationPlayer()
	: IsAnimDone(false)
{
}

void RAnimationPlayer::Proceed(float deltaTime)
{
	if (Animation && !IsAnimDone)
	{
		// Changing time may cause start time greater than end time
		if (CurrentTime >= Animation->GetEndTime() - 1)
		{
			CurrentTime = Animation->GetStartTime();
		}
		RVec3 start_offset = Animation->GetRootPosition(CurrentTime);

		CurrentTime += deltaTime * Animation->GetFrameRate() * TimeScale;
		bool startOver = false;

		if (CurrentTime >= Animation->GetEndTime() - 1)
		{
			if (Animation->IsLooping())
			{
				do
				{
					CurrentTime -= Animation->GetEndTime() - Animation->GetStartTime() - 1;
					startOver = true;
				} while (CurrentTime >= Animation->GetEndTime() - 1);
			}
			else
			{
				CurrentTime = Animation->GetEndTime() - 1;
				IsAnimDone = true;
			}
		}

		if (Animation->GetBitFlags() & AnimBitFlag_HasRootMotion)
		{
			RootOffset = Animation->GetRootPosition(CurrentTime) - start_offset;
			if (startOver)
			{
				RootOffset = Animation->GetRootPosition(Animation->GetEndTime() - 1) - start_offset +
							 Animation->GetRootPosition(CurrentTime) - Animation->GetInitRootPosition();
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
	CurrentTime = Animation->GetStartTime();
}

RAnimationBlender::RAnimationBlender()
{

}

void RAnimationBlender::Play(RAnimation* anim, float time, float timeScale)
{
	m_SourceAnimation.Animation = anim;
	m_SourceAnimation.CurrentTime = time;
	m_SourceAnimation.TimeScale = timeScale;
	m_SourceAnimation.IsAnimDone = false;

	m_TargetAnimation.Animation = nullptr;
}

void RAnimationBlender::Play(RAnimation* anim, float timeScale)
{
	Play(anim, anim->GetStartTime(), timeScale);
}

void RAnimationBlender::Blend(RAnimation* source, float sourceTime, float sourceTimeScale /*= 1.0f*/, RAnimation* target /*= nullptr*/, float targetTime /*= 0.0f*/, float targetTimeScale /*= 0.0f*/, float blendTime /*= 0.0f*/)
{
	m_SourceAnimation.Animation = source;
	m_SourceAnimation.CurrentTime = sourceTime;
	m_SourceAnimation.TimeScale = sourceTimeScale;
	m_SourceAnimation.IsAnimDone = false;

	m_TargetAnimation.Animation = target;
	m_TargetAnimation.CurrentTime = targetTime;
	m_TargetAnimation.TimeScale = targetTimeScale;
	m_TargetAnimation.IsAnimDone = false;

	m_BlendTime = blendTime;
	m_ElapsedBlendTime = 0.0f;
}

void RAnimationBlender::BlendTo(RAnimation* target, float targetTime, float targetTimeScale, float blendTime)
{
	if (m_TargetAnimation.Animation)
	{
		m_SourceAnimation.Animation = m_TargetAnimation.Animation;
		m_SourceAnimation.CurrentTime = m_TargetAnimation.CurrentTime;
		m_SourceAnimation.TimeScale = m_TargetAnimation.TimeScale;
		m_SourceAnimation.IsAnimDone = m_TargetAnimation.IsAnimDone;
	}

	m_TargetAnimation.Animation = target;
	m_TargetAnimation.CurrentTime = targetTime;
	m_TargetAnimation.TimeScale = targetTimeScale;
	m_TargetAnimation.IsAnimDone = false;

	m_BlendTime = blendTime;
	m_ElapsedBlendTime = 0.0f;
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

void RAnimationBlender::GetCurrentBlendedNodePose(int sourceNodeId, int targetNodeId, RMatrix4* matrix)
{
	if (m_SourceAnimation.Animation && m_TargetAnimation.Animation)
	{
		RMatrix4 mat1, mat2;
		m_SourceAnimation.Animation->GetNodePose(sourceNodeId, m_SourceAnimation.CurrentTime, &mat1);
		m_TargetAnimation.Animation->GetNodePose(targetNodeId, m_TargetAnimation.CurrentTime, &mat2);

		// Apply inversed root translation
		if (m_SourceAnimation.Animation->HasRootMotion())
			mat1 *= RMatrix4::CreateTranslation(-m_SourceAnimation.Animation->GetRootPosition(m_SourceAnimation.CurrentTime));
		if (m_TargetAnimation.Animation->HasRootMotion())
			mat2 *= RMatrix4::CreateTranslation(-m_TargetAnimation.Animation->GetRootPosition(m_TargetAnimation.CurrentTime));

		float t = min(1.0f, m_ElapsedBlendTime / m_BlendTime);
		*matrix = RMatrix4::Lerp(mat1, mat2, t);
	}
	else if (m_SourceAnimation.Animation)
	{
		m_SourceAnimation.Animation->GetNodePose(sourceNodeId, m_SourceAnimation.CurrentTime, matrix);

		if (m_SourceAnimation.Animation->HasRootMotion())
			*matrix *= RMatrix4::CreateTranslation(-m_SourceAnimation.Animation->GetRootPosition(m_SourceAnimation.CurrentTime));
	}
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

bool RAnimationBlender::IsAnimationDone()
{
	if (m_TargetAnimation.Animation)
		return m_TargetAnimation.IsAnimDone;
	return (m_SourceAnimation.Animation && m_SourceAnimation.IsAnimDone);
}

RAnimation::RAnimation()
	: m_Flags(0)
{

}

RAnimation::RAnimation(int nodeCount, int frameCount, float startTime, float endTime, float frameRate)
	: m_Flags(0), m_FrameCount(frameCount), m_StartTime(startTime), m_EndTime(endTime), m_FrameRate(frameRate), m_RootNode(-1)
{
	m_NodeKeyFrames = vector<RMatrix4*>(nodeCount, nullptr);
	m_NodeParents = vector<int>(nodeCount, -1);
	m_NodeNames = vector<string>(nodeCount, "");
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

void RAnimation::GetNodePose(int nodeId, float time, RMatrix4* matrix) const
{
	// Make zero based time
	time -= m_StartTime;

	assert(nodeId >= 0 && nodeId < (int)m_NodeKeyFrames.size());
	assert(time >= 0 && time < m_FrameCount);

	int frame1 = (int)time;
	int frame2 = ((int)time + 1) % m_FrameCount;
	//if (frame2 < frame1)
	//	frame2 = frame1;
	float t = time - frame1;

	// Linear interpolate two matrices
	// TODO: Use quaternion slerp and position lerp instead!
	for (int i = 0; i < 16; i++)
	{
		float va = ((float*)&m_NodeKeyFrames[nodeId][frame1])[i];
		float vb = ((float*)&m_NodeKeyFrames[nodeId][frame2])[i];
		((float*)matrix)[i] = va + (vb - va) * t;
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
	for (vector<string>::const_iterator iter = m_NodeNames.begin(); iter != m_NodeNames.end(); iter++)
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