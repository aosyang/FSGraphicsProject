//=============================================================================
// RAnimation.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#ifndef _RANIMATION_H
#define _RANIMATION_H

using namespace std;

class RAnimation
{
public:
	RAnimation(int nodeCount, int frameCount, float startTime, float endTime, float frameRate);
	~RAnimation();

	void AddNodePose(int nodeId, int frameId, const RMatrix4* matrix);
	void GetNodePose(int nodeId, float frameId, RMatrix4* matrix) const;
	int GetNodeCount() const { return (int)m_NodeKeyFrames.size(); }

	void SetParentId(int nodeId, int parentId);
	int GetParentId(int nodeId) const;

	void AddNodeNameToId(const char* nodeName, int nodeId);
	int GetNodeIdByName(const char* nodeName) const;

	float GetStartTime() const { return m_StartTime; }
	float GetEndTime() const { return m_EndTime; }
	float GetFrameRate() { return m_FrameRate; }
private:
	int						m_FrameCount;
	float					m_StartTime, m_EndTime, m_FrameRate;
	vector<RMatrix4*>		m_NodeKeyFrames;
	vector<int>				m_NodeParents;
	map<string, int>		m_NodeNameToIdMap;
};

#endif
