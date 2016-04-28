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
	RAnimation();
	RAnimation(int nodeCount, int frameCount, float startTime, float endTime, float frameRate);
	~RAnimation();

	bool LoadFromFile(const char* filename);
	void SaveToFile(const char* filename);

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
	int						m_FrameCount;
	float					m_StartTime, m_EndTime, m_FrameRate;

	vector<string>			m_NodeNames;
	vector<int>				m_NodeParents;
	vector<RMatrix4*>		m_NodeKeyFrames;

	vector<RVec3>			m_RootDisplacement;
	int						m_RootNode;
};

#endif
