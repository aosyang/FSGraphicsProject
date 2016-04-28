//=============================================================================
// RAnimation.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"
#include "RAnimation.h"

RAnimation::RAnimation()
{

}

RAnimation::RAnimation(int nodeCount, int frameCount, float startTime, float endTime, float frameRate)
	: m_FrameCount(frameCount), m_StartTime(startTime), m_EndTime(endTime), m_FrameRate(frameRate), m_RootNode(-1)
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

bool RAnimation::LoadFromFile(const char* filename)
{
	ifstream fin;
	fin.open(filename, ios::binary);

	if (!fin.is_open())
		return false;

	char header[4];
	fin.read(header, sizeof(header));

	if (header[0] != 'A' ||
		header[1] != 'N' ||
		header[2] != 'I' ||
		header[3] != 'M')
	{
		fin.close();
		return false;
	}

	fin.read((char*)&m_FrameCount, sizeof(int));
	fin.read((char*)&m_StartTime, sizeof(float));
	fin.read((char*)&m_EndTime, sizeof(float));
	fin.read((char*)&m_FrameRate, sizeof(float));
	fin.read((char*)&m_RootNode, sizeof(int));

	UINT nodeCount;
	fin.read((char*)&nodeCount, sizeof(UINT));

	m_NodeKeyFrames = vector<RMatrix4*>(nodeCount, nullptr);
	m_NodeParents = vector<int>(nodeCount, -1);
	m_NodeNames = vector<string>(nodeCount, "");

	for (UINT i = 0; i < nodeCount; i++)
	{
		UINT nameStrSize;
		fin.read((char*)&nameStrSize, sizeof(UINT));
		char buf[64] = { 0 };
		fin.read(buf, sizeof(char) * nameStrSize);
		m_NodeNames[i] = buf;

		fin.read((char*)&m_NodeParents[i], sizeof(int));

		m_NodeKeyFrames[i] = new RMatrix4[m_FrameCount];
		fin.read((char*)m_NodeKeyFrames[i], sizeof(RMatrix4) * m_FrameCount);
	}

	fin.close();
	return true;
}

void RAnimation::SaveToFile(const char* filename)
{
	ofstream fout;
	fout.open(filename, ios::binary);

	if (!fout.is_open())
	{
		return;
	}

	const char header[4] = { 'A', 'N', 'I', 'M' };
	fout.write(header, sizeof(header));

	fout.write((char*)&m_FrameCount, sizeof(int));
	fout.write((char*)&m_StartTime, sizeof(float));
	fout.write((char*)&m_EndTime, sizeof(float));
	fout.write((char*)&m_FrameRate, sizeof(float));
	fout.write((char*)&m_RootNode, sizeof(int));

	UINT nodeCount = m_NodeNames.size();
	fout.write((char*)&nodeCount, sizeof(UINT));

	for (UINT i = 0; i < nodeCount; i++)
	{
		UINT nameStrSize = m_NodeNames[i].size();
		fout.write((char*)&nameStrSize, sizeof(UINT));
		fout.write(m_NodeNames[i].c_str(), sizeof(char) * m_NodeNames[i].size());

		fout.write((char*)&m_NodeParents[i], sizeof(int));
		fout.write((char*)m_NodeKeyFrames[i], sizeof(RMatrix4) * m_FrameCount);
	}

	fout.close();
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
			return iter - m_NodeNames.begin();
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
		m_RootDisplacement[i].y = 0.0f;
	}
}