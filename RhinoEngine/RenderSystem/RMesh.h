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

enum class EMeshCollisionType
{
	BoundingBox,
	TriangleMesh,
};

class RMesh : public RResourceBase
{
	DECLARE_RUNTIME_TYPE(RMesh, RResourceBase)
public:
	RMesh(const std::string& Path);
	RMesh(const std::string& Path, const std::vector<RMeshElement>& meshElements, const std::vector<RMaterial>& materials);
	RMesh(const std::string& Path, const RMeshElement* meshElements, int numElement, const RMaterial* materials, int numMaterial);
	~RMesh();

	/// Required by RResourceManager::RegisterResourceType
	static std::vector<std::string> GetSupportedExtensions();

	void Serialize(RSerializer& serializer);

	const RMaterial& GetMaterial(int index) const;
	const std::vector<RMaterial>& GetMaterials() const;

	const std::vector<RMeshElement>& GetMeshElements() const;
	int GetMeshElementCount() const;

	void SetMeshElements(RMeshElement* meshElements, UINT numElement);
	void SetMaterials(RMaterial* materials, UINT numMaterial);

	void UpdateAabb();
	const RAabb& GetLocalSpaceAabb() const;
	const RAabb& GetMeshElementAabb(int index) const;

	void SetAnimation(RAnimation* anim);
	RAnimation* GetAnimation() const;

	void SetBoneInitInvMatrices(std::vector<RMatrix4>& bonePoses);
	const RMatrix4& GetBoneInitInvMatrices(int index) const { return m_BoneInitInvMatrices[index]; }

	void SetBoneNameList(const std::vector<std::string>& boneNameList);
	const std::string& GetBoneName(int boneId) const;
	int GetBoneCount() const;

	void CacheAnimation(RAnimation* Animation);
	bool HasCachedAnimation(RAnimation* anim) const;
	int GetCachedAnimationNodeId(RAnimation* Animation, int BoneId) const;

	EMeshCollisionType GetCollisionType() const;

protected:
	virtual std::vector<RResourceBase*> EnumerateReferencedResources() const override;

	virtual bool LoadResourceImpl(bool bIsAsyncLoading) override;

	bool TryLoadAsFbxMesh(bool bIsAsyncLoading);
	bool TryLoadAsRmesh(bool bIsAsyncLoading);
private:
	std::vector<RMeshElement>			m_MeshElements;

	std::vector<RMaterial>				m_Materials;
	RAabb							m_Aabb;

	RAnimation*						m_Animation;
	std::vector<RMatrix4>				m_BoneInitInvMatrices;
	std::vector<std::string>					m_BoneIdToName;
	std::map<RAnimation*, std::vector<int>>	m_AnimationNodeCache;
};

FORCEINLINE const std::vector<RMaterial>& RMesh::GetMaterials() const
{
	return m_Materials;
}

FORCEINLINE const std::vector<RMeshElement>& RMesh::GetMeshElements() const
{
	return m_MeshElements;
}

FORCEINLINE int RMesh::GetMeshElementCount() const
{
	return (int)m_MeshElements.size();
}


