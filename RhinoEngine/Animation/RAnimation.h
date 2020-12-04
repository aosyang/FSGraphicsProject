//=============================================================================
// RAnimation.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "Core/CoreTypes.h"
#include "Core/RSerializer.h"

#include "RAnimNode_Base.h"
#include "RAnimNode_AnimationPlayer.h"


class RMesh;
class RAnimation;


enum AnimationBitFlag
{
	AnimBitFlag_Loop			= 1 << 0,		// Should animation loop
	AnimBitFlag_HasRootMotion	= 1 << 1,		// Extract root translation and keep root bone at the origin
	AnimBitFlag_LockRootBone	= 1 << 2,		// Keep root bone at the origin
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
	void EvaluatePose(RAnimPoseData& PoseData) const;

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
	RAnimNode_AnimationPlayer	m_SourceAnimation;
	RAnimNode_AnimationPlayer	m_TargetAnimation;
	float				m_BlendTime;
	float				m_ElapsedBlendTime;
};

/// Data for a single bone from an animation. Stores a list of matrices for each frame
struct RAnimBoneData
{
	RAnimBoneData()
	{
	}

	void Serialize(RSerializer& Serializer)
	{
		Serializer.SerializeData(BoneName);
		Serializer.SerializeVector(FrameMatrices_MeshSpace);
		Serializer.SerializeVector(FrameMatrices_LocalSpace);
	}

	/// Name of the bone node
	std::string				BoneName;

	/// Poses of each frame in mesh space
	std::vector<RMatrix4>	FrameMatrices_MeshSpace;

	/// Poses of each frame in local space
	std::vector<RMatrix4>	FrameMatrices_LocalSpace;
};


/// The animation class.
/// Contains a set of frames with each frame made up by the matrices of each bone node from the skeletal.
class RAnimation
{
public:
	RAnimation();
	RAnimation(const std::string& InName, RMesh* InSkeletalMesh, int nodeCount, int frameCount, float startTime, float endTime, float frameRate);
	~RAnimation();

	void SetName(const std::string& name)	{ m_Name = name; }
	const std::string& GetName() const		{ return m_Name; }

	/// Set flags for animation
	/// TODO: Flags should be loaded with the animation in the future. Remove this function later
	void SetBitFlags(int flags)		{ m_Flags = flags; }
	void EnableBitFlags(int Flags)	{ m_Flags |= Flags; }

	/// Does animation use any root motion?
	bool HasRootMotion() const;

	/// Is animation looping?
	bool IsLooping() const;

	/// Should keep root bone at the origin
	bool IsRootLocked() const;

	/// Get speed of root bone in unit/sec
	float GetRootSpeed() const;

	void Serialize(RSerializer& serializer);

	/// Set a mesh space matrix for a bone node at given frame
	void SetMeshSpaceBoneMatrixAtFrame(int BoneId, int FrameId, const RMatrix4& InMatrix);

	/// Set a local space (parent bone space) matrix for a bone node at given frame
	void SetLocalSpaceBoneMatrixAtFrame(int BoneId, int FrameId, const RMatrix4& InMatrix);

	/// Get the mesh space matrix of a bone node at given time
	void GetMeshSpaceBoneMatrixAtTime(int BoneId, float Time, RMatrix4* OutMatrix) const;

	/// Get the local space (parent bone space) matrix of a bone node at given time
	void GetLocalSpaceBoneMatrixAtTime(int BoneId, float Time, RMatrix4* OutMatrix) const;

	/// Get total number of bone nodes
	int GetNodeCount() const { return (int)BoneNodeData.size(); }

	/// Evaluate pose for animation at given time
	void EvaluatePoseAtTime(RAnimPoseData& PoseData, float Time) const;

	/// Get the root displacement at the initial frame
	RVec3 GetInitRootPosition() const;

	/// Get the root displacement at given time
	RVec3 GetRootPosition(float time) const;

	RVec3 GetRootPositionAtFrame(int FrameId) const;

	/// Build root displacements for each frame
	void BuildRootDisplacements();

	float GetStartTime() const { return m_StartTime; }
	float GetEndTime() const { return m_EndTime; }

	/// Get frame rate in frames per second
	float GetFrameRate() const { return m_FrameRate; }

	/// Get bone index in the animation.
	/// Note: This index might be different than the result of finding bone index on a skinned mesh
	int FindAnimBoneIndexByName(const std::string& BoneName) const;

	/// Assign a name to a bone in the animation
	void SetAnimBoneName(int AnimBoneId, const std::string& BoneName);

	/// Checks if a bone is a root bone
	bool IsRootBone(int BoneId) const;

	void SetSkeletalMesh(RMesh* InSkelMesh);

private:
	int GetBitFlags() const;

	/// Get index of closest left and right frames at given time
	/// OutFactor: The lerp factor between two frames
	void GetNeighborFramesAtTime(float Time, int& OutFrame1, int& OutFrame2, float& OutFactor) const;

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
	std::vector<RAnimBoneData>	BoneNodeData;

	/// Root displacements of each frame
	std::vector<RVec3>			m_RootDisplacement;

	float					RootSpeed;

	/// The skeletal mesh to play this animation
	RMesh* SkeletalMesh;
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

FORCEINLINE bool RAnimation::IsRootLocked() const
{
	return (GetBitFlags() & AnimBitFlag_LockRootBone) != 0;
}

FORCEINLINE float RAnimation::GetRootSpeed() const
{
	return RootSpeed;
}

FORCEINLINE int RAnimation::GetBitFlags() const
{
	return m_Flags;
}

