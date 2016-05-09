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

	void Proceed(float time);
	void Reset();
};

class RAnimationBlender
{
public:
	RAnimationBlender();

	void Play(RAnimation* anim, float time, float timeScale);
	void Play(RAnimation* anim, float timeScale = 1.0f);
	void Blend(RAnimation* start, float startTime, float startTimeScale = 1.0f,
			   RAnimation* end = nullptr, float endTime = 0.0f, float endTimeScale = 0.0f,
			   float blendTime = 0.0f);
	void BlendTo(RAnimation* target, float targetTime, float targetTimeScale = 1.0f, float blendTime = 0.0f);

	void ProceedAnimation(float time);
	void GetCurrentBlendedNodePose(int startNodeId, int endNodeId, RMatrix4* matrix);
	RVec3 GetCurrentRootOffset();
	bool IsAnimationDone();
	RAnimation* GetStartAnimation();
	float GetStartAnimationTime() const;
	RAnimation* GetEndAnimation();
	float GetEndAnimationTime() const;
	float GetElapsedBlendTime() const;
private:
	RAnimationPlayer	m_BlendStartAnim;
	RAnimationPlayer	m_BlendEndAnim;
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
	float GetFrameRate() { return m_FrameRate; }
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

#endif
