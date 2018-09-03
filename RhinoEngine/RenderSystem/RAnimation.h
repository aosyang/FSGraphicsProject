//=============================================================================
// RAnimation.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#ifndef _RANIMATION_H
#define _RANIMATION_H

using namespace std;

enum AnimationBitFlag
{
	AnimBitFlag_Loop			= 1 << 0,
	AnimBitFlag_HasRootMotion	= 1 << 1,
};

class RAnimation;

class RAnimationPlayer
{
public:
	RAnimationPlayer();

	RAnimation*		Animation;
	float			CurrentTime;
	float			TimeScale;
	
	RVec3			RootOffset;
	bool			IsAnimDone;

	void Proceed(float deltaTime);
	void Reset();
};

class RAnimationBlender
{
public:
	RAnimationBlender();

	void Play(RAnimation* anim, float time, float timeScale);
	void Play(RAnimation* anim, float timeScale = 1.0f);
	void Blend(RAnimation* source, float sourceTime, float sourceTimeScale = 1.0f,
			   RAnimation* target = nullptr, float targetTime = 0.0f, float targetTimeScale = 0.0f,
			   float blendTime = 0.0f);
	void BlendTo(RAnimation* target, float targetTime, float targetTimeScale = 1.0f, float blendTime = 0.0f);

	void ProceedAnimation(float deltaTime);
	void GetCurrentBlendedNodePose(int sourceNodeId, int targetNodeId, RMatrix4* matrix);
	RVec3 GetCurrentRootOffset();

	/// Check if animation has finished playing.
	bool HasFinishedPlaying() const;

	RAnimation* GetSourceAnimation() const;
	float GetSourcePlaybackTime() const;
	RAnimation* GetTargetAnimation() const;
	float GetTargetPlaybackTime() const;
	float GetElapsedBlendTime() const;
private:
	RAnimationPlayer	m_SourceAnimation;
	RAnimationPlayer	m_TargetAnimation;
	float				m_BlendTime;
	float				m_ElapsedBlendTime;
};


class RAnimation
{
public:
	RAnimation();
	RAnimation(int nodeCount, int frameCount, float startTime, float endTime, float frameRate);
	~RAnimation();

	void SetName(const string& name) { m_Name = name; }
	const string& GetName() const { return m_Name; }

	void SetBitFlags(int flags) { m_Flags = flags; }
	int GetBitFlags() const { return m_Flags; }

	bool HasRootMotion() const;
	bool IsLooping() const;

	void Serialize(RSerializer& serializer);

	void AddNodePose(int nodeId, int frameId, const RMatrix4* matrix);
	void GetNodePose(int nodeId, float time, RMatrix4* matrix) const;
	int GetNodeCount() const { return (int)m_NodeKeyFrames.size(); }

	RVec3 GetInitRootPosition() const;
	RVec3 GetRootPosition(float time) const;

	void SetParentId(int nodeId, int parentId);
	int GetParentId(int nodeId) const;

	void AddNodeNameToId(const char* nodeName, int nodeId);
	int GetNodeIdByName(const char* nodeName) const;

	void SetRootNode(int nodeId) { m_RootNode = nodeId; }
	int GetRootNode() const { return m_RootNode; }
	void BuildRootDisplacementArray();

	float GetStartTime() const { return m_StartTime; }
	float GetEndTime() const { return m_EndTime; }

	/// Get frame rate in frames per second
	float GetFrameRate() const { return m_FrameRate; }
private:
	string					m_Name;
	int						m_Flags;
	int						m_FrameCount;
	float					m_StartTime, m_EndTime, m_FrameRate;

	vector<string>			m_NodeNames;
	vector<int>				m_NodeParents;
	vector<RMatrix4*>		m_NodeKeyFrames;

	vector<RVec3>			m_RootDisplacement;
	int						m_RootNode;
};


FORCEINLINE RAnimation* RAnimationBlender::GetSourceAnimation() const
{
	return m_SourceAnimation.Animation;
}

FORCEINLINE float RAnimationBlender::GetSourcePlaybackTime() const
{
	return m_SourceAnimation.CurrentTime;
}

FORCEINLINE RAnimation* RAnimationBlender::GetTargetAnimation() const
{
	return m_TargetAnimation.Animation;
}

FORCEINLINE float RAnimationBlender::GetTargetPlaybackTime() const
{
	return m_TargetAnimation.CurrentTime;
}

FORCEINLINE float RAnimationBlender::GetElapsedBlendTime() const
{
	return m_ElapsedBlendTime;
}



FORCEINLINE bool RAnimation::HasRootMotion() const
{
	return (GetBitFlags() & AnimBitFlag_HasRootMotion) != 0;
}

FORCEINLINE bool RAnimation::IsLooping() const
{
	return (GetBitFlags() & AnimBitFlag_Loop) != 0;
}

#endif
