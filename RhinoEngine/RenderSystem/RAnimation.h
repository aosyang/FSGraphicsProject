//=============================================================================
// RAnimation.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "Core/CoreTypes.h"

enum AnimationBitFlag
{
	AnimBitFlag_Loop			= 1 << 0,
	AnimBitFlag_HasRootMotion	= 1 << 1,
};

class RAnimation;

/// Plays a single animation
class RAnimationPlayer
{
public:
	RAnimationPlayer();

	RAnimation*		Animation;
	float			CurrentPlaybackTime;
	float			TimeScale;
	
	RVec3			RootOffset;
	bool			IsAnimDone;

	void Reset();

	/// Proceed the animation playback
	void Proceed(float DeltaTime);

	/// Start the animation from beginning
	void Rewind();

	bool EvaluatePose(const RMesh& SkinnedMesh, RMatrix4* OutMatrices) const;
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
	bool EvaluatePose(const RMesh& SkinnedMesh, RMatrix4* OutBoneMatrices) const;

	/// Get the root offset at current state
	RVec3 GetCurrentRootOffset() const;

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

/// The animation node data
struct RAnimNodeData
{
	RAnimNodeData()
		: ParentId(-1)
	{
	}

	void Serialize(RSerializer& Serializer)
	{
		Serializer.SerializeData(BoneName);
		Serializer.SerializeData(ParentId);
		Serializer.SerializeVector(FrameMatrices);
	}

	/// Name of the bone node
	std::string				BoneName;

	/// Parent bone node id
	int						ParentId;

	/// Poses of each frame
	std::vector<RMatrix4>	FrameMatrices;
};


/// The animation class.
/// Contains a set of frames with each frame made up by the matrices of each bone node from the skeletal.
class RAnimation
{
public:
	RAnimation();
	RAnimation(int nodeCount, int frameCount, float startTime, float endTime, float frameRate);
	~RAnimation();

	void SetName(const std::string& name) { m_Name = name; }
	const std::string& GetName() const { return m_Name; }

	/// Set flags for animation
	/// TODO: Flags should be loaded with the animation in the future. Remove this function later
	void SetBitFlags(int flags) { m_Flags = flags; }

	/// Does animation use any root motion?
	bool HasRootMotion() const;

	/// Is animation looping?
	bool IsLooping() const;

	void Serialize(RSerializer& serializer);

	/// Add a pose matrix for a bone node at given frame
	void AddNodePoseAtFrame(int nodeId, int frameId, const RMatrix4* matrix);

	/// Get the pose matrix of a bone node at given time
	void GetNodePoseAtTime(int NodeId, float Time, RMatrix4* OutMatrix) const;

	/// Get total number of bone nodes
	int GetNodeCount() const { return (int)BoneNodeData.size(); }

	/// Get the root displacement at the initial frame
	RVec3 GetInitRootPosition() const;

	/// Get the root displacement at given time
	RVec3 GetRootPosition(float time) const;

	/// Set the parent id for a node
	void SetNodeParentId(int NodeId, int ParentId);

	/// Get the parent id for a node
	int GetNodeParentId(int NodeId) const;

	/// Set name for node
	void SetNodeName(int NodeId, const char* NodeName);

	/// Get id of node with given name
	int GetNodeIdByName(const char* nodeName) const;

	/// Set the id of root node
	void SetRootNode(int nodeId) { m_RootNode = nodeId; }

	/// Get the id of root node
	int GetRootNode() const { return m_RootNode; }

	/// Build root displacements for each frame
	void BuildRootDisplacements();

	float GetStartTime() const { return m_StartTime; }
	float GetEndTime() const { return m_EndTime; }

	/// Get frame rate in frames per second
	float GetFrameRate() const { return m_FrameRate; }

private:
	int GetBitFlags() const;

private:
	/// Name of the animation
	std::string				m_Name;

	/// Animation flags. See definition of AnimationBitFlag
	int						m_Flags;

	/// Total number of frames
	int						m_FrameCount;

	/// Time at the beginning frame
	float					m_StartTime;

	/// Time at the last frame
	float					m_EndTime;

	/// The playback rate
	float					m_FrameRate;

	/// Frame data of each bone node
	std::vector<RAnimNodeData>	BoneNodeData;

	/// Root displacements of each frame
	std::vector<RVec3>			m_RootDisplacement;

	/// Index of the root node
	int							m_RootNode;
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

FORCEINLINE int RAnimation::GetBitFlags() const
{
	return m_Flags;
}

