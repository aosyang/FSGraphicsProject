//=============================================================================
// RMesh.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "RShaderManager.h"
#include "Resource/RResourceBase.h"
#include "../Shaders/ConstBufferVS.h"	// MAX_BONE_COUNT

#include "RMaterial.h"
#include "RMeshElement.h"

class RAnimation;

struct BoneMatrices
{
	RMatrix4 boneMatrix[MAX_BONE_COUNT];
};

enum class EMeshCollisionType
{
	BoundingBox,
	TriangleMesh,
};

namespace tinyxml2
{
	class XMLDocument;
	class XMLElement;
}

/// A helper function to save an array of materials as XML elements
void SerializeXmlMaterials_Save(const std::vector<RMaterial*>& Materials, tinyxml2::XMLDocument* XmlDoc, tinyxml2::XMLElement* XmlElemMaterial);


struct SkeletalData
{
	struct BoneData
	{
		BoneData()
			: ParentId(-1)
		{}

		bool operator==(const BoneData& Other) const
		{
			return BoneName == Other.BoneName && ParentId == Other.ParentId;
		}

		bool operator!=(const BoneData& Other) const
		{
			return !operator==(Other);
		}

		void Serialize(RSerializer& Serializer);

		std::string BoneName;
		int ParentId;
	};

	SkeletalData()
		: RootBone(-1)
	{
	}

	SkeletalData(int InBoneCount)
	{
		SkeletalBones.resize(InBoneCount);
	}

	bool operator==(const SkeletalData& Other) const
	{
		return SkeletalBones == Other.SkeletalBones;
	}

	bool operator!=(const SkeletalData& Other) const
	{
		return !operator==(Other);
	}

	void Serialize(RSerializer& Serializer);

	void SetBoneName(int BoneId, std::string& BoneName)
	{
		SkeletalBones[BoneId].BoneName = BoneName;
	}

	/// Find index of bone by its name
	int FindBoneByName(const std::string& BoneName) const
	{
		for (int i = 0; i < (int)SkeletalBones.size(); i++)
		{
			if (SkeletalBones[i].BoneName == BoneName)
			{
				return i;
			}
		}

		return -1;
	}

	// Find a parent id for a bone.
	// The index here represents the order of bones used in the skinned mesh.
	int FindParentForBone(int BoneId) const
	{
		return SkeletalBones[BoneId].ParentId;
	}

	// Assign a parent id to a bone.
	// The index here represents the order of bones used in the skinned mesh.
	void SetBoneParent(const std::string& BoneName, const std::string& ParentName)
	{
		int BoneId = FindBoneByName(BoneName);
		int ParentId = FindBoneByName(ParentName);

		if (BoneId != -1)
		{
			SkeletalBones[BoneId].ParentId = ParentId;
		}
	}

	void SetRootBone(int BoneId)
	{
		RootBone = BoneId;
	}

	int GetRootBone() const
	{
		return RootBone;
	}

	int RootBone;
	std::vector<BoneData> SkeletalBones;
};


// A two-way bone id map (mesh bone id <-> animation bone id)
struct RBoneIdMap
{
	std::vector<int> MeshToAnim;
	std::vector<int> AnimToMesh;
};


class RMesh : public RResourceBase
{
	DECLARE_RUNTIME_TYPE(RMesh, RResourceBase)
public:
	RMesh(const std::string& Path);
	//RMesh(const std::string& Path, const std::vector<RMeshElement>& meshElements, const std::vector<RMeshMaterialData>& materials);
	//RMesh(const std::string& Path, const RMeshElement* meshElements, int numElement, const RMeshMaterialData* materials, int numMaterial);
	~RMesh();

	/// Required by RResourceManager::RegisterResourceType
	static std::vector<std::string> GetSupportedExtensions();

	void Serialize(RSerializer& serializer);

	const RMaterial* GetMaterial(int index) const;
	const std::vector<RMaterial*>& GetMaterials() const;

	const RMeshElement& GetMeshElement(int i) const;
	int GetMeshElementCount() const;

	void SetMeshElements(std::vector<std::unique_ptr<RMeshElement>>&& Elements);

	void SetMaterialSlot(int SlotId, RMaterial* Material);
	void SetMaterials(const std::vector<RMaterial*> NewMaterials);

	/// Save current materials to a separate .rmtl file
	void SaveMaterialsToDiskAsDefaults();

	void UpdateAabb();
	const RAabb& GetLocalSpaceAabb() const;
	const RAabb& GetMeshElementAabb(int index) const;

	void SetAnimation(RAnimation* anim);
	RAnimation* GetAnimation() const;

	// Check if any mesh elements uses skinning
	bool HasAnySkinnedMeshElements() const;

	void SetSkeletalData(const SkeletalData& InMeshSkeletalData);
	const SkeletalData& GetSkeletalData() const;

	void SetBoneInitInvMatrices(std::vector<RMatrix4>& bonePoses);
	const RMatrix4& GetBoneInitInvMatrices(int index) const { return m_BoneInitInvMatrices[index]; }

	void SetBoneNameList(const std::vector<std::string>& boneNameList);
	const std::string& GetBoneName(int BoneId) const;
	int FindBoneByName(const std::string& BoneName) const;
	int GetBoneCount() const;

	/// TODO: Remove CacheAnimation and use a skeletal data class for the functionality of GetCachedAnimationNodeId
	void CacheAnimation(RAnimation* Animation);
	bool HasCachedAnimation(RAnimation* anim) const;

	/// Map a bone index from animation to skinned mesh
	int ConvertBoneIndex_MeshToAnimation(const RAnimation* Animation, int MeshBoneId) const;
	int ConvertBoneIndex_AnimationToMesh(const RAnimation* Animation, int AnimBoneId) const;
	const RBoneIdMap* GetBoneIdMapForAnimation(const RAnimation* Animation) const;

	EMeshCollisionType GetCollisionType() const;

protected:
	virtual std::vector<RResourceBase*> EnumerateReferencedResources() const override;

	virtual bool LoadResourceImpl() override;

	bool TryLoadAsFbxMesh();
	bool TryLoadAsRmesh();

	// Save mesh data as binary rmesh format
	void SaveBinaryMesh();

private:
	std::vector<std::unique_ptr<RMeshElement>>	m_MeshElements;

	std::vector<RMaterial*>			m_Materials;
	RAabb							m_Aabb;

	RAnimation*						m_Animation;
	std::vector<RMatrix4>			m_BoneInitInvMatrices;
	std::vector<std::string>		m_BoneIdToName;

	SkeletalData					MeshSkeletalData;

	/// TODO: Store the node cache in a shared skeletal data
	std::map<const RAnimation*, RBoneIdMap>	m_AnimationNodeCache;
};

FORCEINLINE const std::vector<RMaterial*>& RMesh::GetMaterials() const
{
	return m_Materials;
}

FORCEINLINE const RMeshElement& RMesh::GetMeshElement(int i) const
{
	// Accessing mesh elements during loading may result in incomplete data
	assert(IsLoaded());
	return *m_MeshElements[i];
}

FORCEINLINE int RMesh::GetMeshElementCount() const
{
	// Accessing mesh elements during loading may result in incomplete data
	assert(IsLoaded());
	return (int)m_MeshElements.size();
}

