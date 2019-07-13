//=============================================================================
// RMesh.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "RShaderManager.h"
#include "Resource/RResourceBase.h"
#include "../../Shaders/ConstBufferVS.h"	// MAX_BONE_COUNT
#include "RMaterial.h"

struct BoneMatrices
{
	RMatrix4 boneMatrix[MAX_BONE_COUNT];
};

class RMesh : public RResourceBase
{
public:
	RMesh(const string& Path);
	RMesh(const string& Path, const vector<RMeshElement>& meshElements, const vector<RMaterial>& materials);
	RMesh(const string& Path, const RMeshElement* meshElements, int numElement, const RMaterial* materials, int numMaterial);
	~RMesh();

	/// Required by RResourceManager::RegisterResourceType
	static vector<string> GetSupportedExtensions();

	void Serialize(RSerializer& serializer);

	virtual bool LoadResourceData(bool bIsAsyncLoading) override;

	const RMaterial& GetMaterial(int index) const;
	const vector<RMaterial>& GetMaterials() const;

	const vector<RMeshElement>& GetMeshElements() const;
	int GetMeshElementCount() const;

	void SetMeshElements(RMeshElement* meshElements, UINT numElement);
	void SetMaterials(RMaterial* materials, UINT numMaterial);

	void UpdateAabb();
	const RAabb& GetLocalSpaceAabb() const;
	const RAabb& GetMeshElementAabb(int index) const;

	void SetAnimation(RAnimation* anim);
	RAnimation* GetAnimation() const;

	void SetBoneInitInvMatrices(vector<RMatrix4>& bonePoses);
	const RMatrix4& GetBoneInitInvMatrices(int index) const { return m_BoneInitInvMatrices[index]; }

	void SetBoneNameList(const vector<string>& boneNameList);
	const string& GetBoneName(int boneId) const;
	int GetBoneCount() const;

	void CacheAnimation(RAnimation* anim);
	bool HasCachedAnimation(RAnimation* anim) const;
	int GetCachedAnimationNodeId(RAnimation* anim, int boneId);

protected:
	virtual vector<RResourceBase*> EnumerateReferencedResources() const override;

	bool TryLoadAsFbxMesh(bool bIsAsyncLoading);
	bool TryLoadAsRmesh(bool bIsAsyncLoading);
private:
	vector<RMeshElement>			m_MeshElements;

	vector<RMaterial>				m_Materials;
	RAabb							m_Aabb;

	RAnimation*						m_Animation;
	vector<RMatrix4>				m_BoneInitInvMatrices;
	vector<string>					m_BoneIdToName;
	map<RAnimation*, vector<int>>	m_AnimationNodeCache;
};

FORCEINLINE const vector<RMaterial>& RMesh::GetMaterials() const
{
	return m_Materials;
}

FORCEINLINE const vector<RMeshElement>& RMesh::GetMeshElements() const
{
	return m_MeshElements;
}

FORCEINLINE int RMesh::GetMeshElementCount() const
{
	return (int)m_MeshElements.size();
}


