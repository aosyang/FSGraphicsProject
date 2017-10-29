//=============================================================================
// RMesh.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"
#include "RMesh.h"

RMesh::RMesh(string path)
	: RResourceBase(RT_Mesh, path),
	  m_Animation(nullptr)
{
}

RMesh::RMesh(string path, const vector<RMeshElement>& meshElements, const vector<RMaterial>& materials)
	: RResourceBase(RT_Mesh, path),
	  m_Animation(nullptr)
{
	m_MeshElements = meshElements;
	m_Materials = materials;
}

RMesh::RMesh(string path, const RMeshElement* meshElements, int numElement, const RMaterial* materials, int numMaterial)
	: RResourceBase(RT_Mesh, path),
	  m_Animation(nullptr)
{
	assert(meshElements && numElement);
	m_MeshElements.assign(meshElements, meshElements + numElement);

	if (materials && numMaterial)
		m_Materials.assign(materials, materials + numMaterial);
	else
	{
		RMaterial emptyMaterial;
		ZeroMemory(&emptyMaterial, sizeof(emptyMaterial));
		for (int i = 0; i < numElement; i++)
		{
			m_Materials.push_back(emptyMaterial);
		}
	}
}

RMesh::~RMesh()
{
	for (UINT32 i = 0; i < m_MeshElements.size(); i++)
	{
		m_MeshElements[i].Release();
	}

	SAFE_DELETE(m_Animation);
}

void RMesh::Serialize(RSerializer& serializer)
{
	if (!serializer.EnsureHeader("RMSH", 4))
		return;

	serializer.SerializeVector(m_MeshElements, &RSerializer::SerializeObject);
	serializer.SerializeVector(m_Materials, &RSerializer::SerializeObject);
	serializer.SerializeObjectPtr(&m_Animation);
	serializer.SerializeVector(m_BoneInitInvMatrices);
	serializer.SerializeVector(m_BoneIdToName, &RSerializer::SerializeData);

	if (serializer.IsReading())
	{
		UpdateAabb();
	}
}

const RMaterial& RMesh::GetMaterial(int index) const
{
	assert(index >= 0 && index < (int)m_Materials.size());
	return m_Materials[index];
}

void RMesh::SetMeshElements(RMeshElement* meshElements, UINT numElement)
{
	// TODO: use mutex
	assert(meshElements && numElement);
	m_MeshElements.assign(meshElements, meshElements + numElement);
}

void RMesh::SetMaterials(RMaterial* materials, UINT numMaterial)
{
	assert(materials && numMaterial);
	m_Materials.assign(materials, materials + numMaterial);
}

void RMesh::UpdateAabb()
{
	m_Aabb = RAabb::Default;
	for (UINT32 i = 0; i < m_MeshElements.size(); i++)
	{
		m_Aabb.Expand(m_MeshElements[i].GetAabb());
	}
}

const RAabb& RMesh::GetLocalSpaceAabb() const
{
	return m_Aabb;
}

const RAabb& RMesh::GetMeshElementAabb(int index) const
{
	return m_MeshElements[index].GetAabb();
}

void RMesh::SetAnimation(RAnimation* anim)
{
	m_Animation = anim;
}

RAnimation* RMesh::GetAnimation() const
{
	return m_Animation;
}

void RMesh::SetBoneInitInvMatrices(vector<RMatrix4>& bonePoses)
{
	m_BoneInitInvMatrices = move(bonePoses);
}

void RMesh::SetBoneNameList(const vector<string>& boneNameList)
{
	m_BoneIdToName = boneNameList;
}

const string& RMesh::GetBoneName(int boneId) const
{
	return m_BoneIdToName[boneId];
}

int RMesh::GetBoneCount() const
{
	return (int)m_BoneIdToName.size();
}

void RMesh::CacheAnimation(RAnimation* anim)
{
	if (!anim || !m_BoneIdToName.size() || m_AnimationNodeCache.find(anim) != m_AnimationNodeCache.end())
		return;

	const char* rootNodeName = m_BoneIdToName[0].data();
	anim->SetRootNode(anim->GetNodeIdByName(rootNodeName));
	anim->BuildRootDisplacementArray();

	vector<int> nodeIdMap;
	nodeIdMap.resize(m_BoneIdToName.size());
	for (int i = 0; i < (int)m_BoneIdToName.size(); i++)
	{
		nodeIdMap[i] = anim->GetNodeIdByName(m_BoneIdToName[i].data());
	}

	m_AnimationNodeCache[anim] = nodeIdMap;
}

int RMesh::GetCachedAnimationNodeId(RAnimation* anim, int boneId)
{
	if (!anim || !m_BoneIdToName.size())
		return -1;

	map<RAnimation*, vector<int>>::iterator iter = m_AnimationNodeCache.find(anim);
	if (iter == m_AnimationNodeCache.end())
		return -1;

	return iter->second[boneId];
}
