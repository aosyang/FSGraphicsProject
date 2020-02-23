//=============================================================================
// RAnimation.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

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
	float			CurrentPlaybackTime;
	float			TimeScale;
	
	RVec3			RootOffset;
	bool			IsAnimDone;

	void Proceed(float deltaTime);
	void Reset();
};

/// An animation evaluator that does the two-way blending
class RAnimationBlender
{
public:
	RAnimationBlender();

	/// Start playing an animation immediately
	void Play(RAnimation* Animation, float StartTime = -1.0f, float TimeScale = 1.0f);

	void Blend(RAnimation* SourceAnimation, float SourceStartTime = -1.0f, float SourceTimeScale = 1.0f,
			   RAnimation* TargetAnimation = nullptr, float TargetStartTime = -1.0f, float TargetTimeScale = 0.0f,
			   float BlendTime = 0.0f);

	/// Blend out from current animation to a target animation
	void BlendOutTo(RAnimation* TargetAnimation, float TargetStartTime = -1.0f, float TargetTimeScale = 1.0f, float BlendTime = 0.0f);

	/// Update the animation blender for the frame
	void ProceedAnimation(float deltaTime);

	/// Evaluate the pose for a skinned mesh at current state
	bool EvaluatePose(RMesh* SkinnedMesh, const RMatrix4& LocalToWorld, RMatrix4* OutBoneMatrices) const;

	/// Get the root offset at current state
	RVec3 GetCurrentRootOffset();

	/// Check if animation has finished playing.
	bool HasFinishedPlaying() const;

	RAnimation* GetSourceAnimation() const;
	float GetSourcePlaybackTime() const;
	RAnimation* GetTargetAnimation() const;
	float GetTargetPlaybackTime() const;
	float GetElapsedBlendTime() const;

private:
	bool GetCurrentBlendedNodePose(int SourceNodeId, int TargetNodeId, RMatrix4* OutMatrix) const;

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

	void SetName(const std::string& name) { m_Name = name; }
	const std::string& GetName() const { return m_Name; }

	void SetBitFlags(int flags) { m_Flags = flags; }
	int GetBitFlags() const { return m_Flags; }

	bool HasRootMotion() const;
	bool IsLooping() const;

	void Serialize(RSerializer& serializer);

	void AddNodePose(int nodeId, int frameId, const RMatrix4* matrix);
	void GetNodePose(int NodeId, float Time, RMatrix4* OutMatrix) const;
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
	std::string					m_Name;
	int						m_Flags;
	int						m_FrameCount;
	float					m_StartTime, m_EndTime, m_FrameRate;

	std::vector<std::string>			m_NodeNames;
	std::vector<int>				m_NodeParents;
	std::vector<RMatrix4*>		m_NodeKeyFrames;

	std::vector<RVec3>			m_RootDisplacement;
	int						m_RootNode;
};


FORCEINLINE RAnimation* RAnimationBlender::GetSourceAnimation() const
{
	return m_SourceAnimation.Animation;
}

FORCEINLINE float RAnimationBlender::GetSourcePlaybackTime() const
{
	return m_SourceAnimation.CurrentPlaybackTime;
}

FORCEINLINE RAnimation* RAnimationBlender::GetTargetAnimation() const
{
	return m_TargetAnimation.Animation;
}

FORCEINLINE float RAnimationBlender::GetTargetPlaybackTime() const
{
	return m_TargetAnimation.CurrentPlaybackTime;
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

