//=============================================================================
// RMesh.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"
#include "RMesh.h"
#include "Resource/RFbxMeshLoader.h"

// Whether to export .fbx as .rmesh after loading
#define EXPORT_FBX_AS_BINARY_MESH 1

RMesh::RMesh(const std::string& Path)
	: RResourceBase(RT_Mesh, Path),
	  m_Animation(nullptr)
{
}

RMesh::RMesh(const std::string& Path, const std::vector<RMeshElement>& meshElements, const std::vector<RMaterial>& materials)
	: RResourceBase(RT_Mesh, Path),
	  m_Animation(nullptr)
{
	m_MeshElements = meshElements;
	m_Materials = materials;
}

RMesh::RMesh(const std::string& Path, const RMeshElement* meshElements, int numElement, const RMaterial* materials, int numMaterial)
	: RResourceBase(RT_Mesh, Path),
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

std::vector<std::string> RMesh::GetSupportedExtensions()
{
	static const std::vector<std::string> MeshExts{ ".fbx"/*, ".rmesh"*/ };
	return MeshExts;
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

bool RMesh::LoadResourceData(bool bIsAsyncLoading)
{
	if (TryLoadAsRmesh(bIsAsyncLoading))
	{
		return true;
	}

	return TryLoadAsFbxMesh(bIsAsyncLoading);
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

void RMesh::SetBoneInitInvMatrices(std::vector<RMatrix4>& bonePoses)
{
	m_BoneInitInvMatrices = move(bonePoses);
}

void RMesh::SetBoneNameList(const std::vector<std::string>& boneNameList)
{
	m_BoneIdToName = boneNameList;
}

const std::string& RMesh::GetBoneName(int boneId) const
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

	std::vector<int> nodeIdMap;
	nodeIdMap.resize(m_BoneIdToName.size());
	for (int i = 0; i < (int)m_BoneIdToName.size(); i++)
	{
		nodeIdMap[i] = anim->GetNodeIdByName(m_BoneIdToName[i].data());
	}

	m_AnimationNodeCache[anim] = nodeIdMap;
}

bool RMesh::HasCachedAnimation(RAnimation* anim) const
{
	if (anim)
	{
		return m_AnimationNodeCache.find(anim) != m_AnimationNodeCache.end();
	}

	return false;
}

int RMesh::GetCachedAnimationNodeId(RAnimation* anim, int boneId)
{
	if (!anim || !m_BoneIdToName.size())
		return -1;

	auto Iter = m_AnimationNodeCache.find(anim);
	if (Iter == m_AnimationNodeCache.end())
		return -1;

	return Iter->second[boneId];
}

std::vector<RResourceBase*> RMesh::EnumerateReferencedResources() const
{
	std::vector<RResourceBase*> ReferencedResources;

	for (const auto& Material : m_Materials)
	{
		for (int i = 0; i < Material.TextureNum; i++)
		{
			RTexture* Texture = Material.Textures[i];
			if (Texture != nullptr)
			{
				// Add unique resources to the list
				if (find(ReferencedResources.begin(), ReferencedResources.end(), Texture) == ReferencedResources.end())
				{
					ReferencedResources.push_back(Texture);
				}
			}
		}
	}

	return ReferencedResources;
}

bool RMesh::TryLoadAsFbxMesh(bool bIsAsyncLoading)
{
	std::unique_ptr<RFbxMeshLoader> FbxMeshLoader(new RFbxMeshLoader);

	if (FbxMeshLoader->LoadDataForMeshResource(this, GetFileSystemPath()))
	{
		// Notify mesh has been loaded
		OnLoadingFinished(bIsAsyncLoading);

#if EXPORT_FBX_AS_BINARY_MESH == 1
		std::string rmeshName = RFileUtil::ReplaceExtension(GetFileSystemPath(), "rmesh");
		RSerializer serializer;
		serializer.Open(rmeshName, ESerializeMode::Write);
		if (serializer.IsOpen())
		{
			Serialize(serializer);
			serializer.Close();
		}
#endif
	}

	return true;
}

bool RMesh::TryLoadAsRmesh(bool bIsAsyncLoading)
{
	std::vector<RMeshElement> meshElements;
	std::vector<RMaterial> materials;

	RLog("Loading mesh [%s]...\n", GetFileSystemPath().data());

	const std::string BinaryMeshPath = RFileUtil::ReplaceExtension(GetFileSystemPath(), "rmesh");

	RSerializer serializer;
	serializer.Open(BinaryMeshPath, ESerializeMode::Read);
	if (!serializer.IsOpen())
		return false;
	Serialize(serializer);
	serializer.Close();

	//RAnimation* animation = new RAnimation();
	//std::string animFilename = RFileUtil::ReplaceExt(task->Filename.size(), "ranim");

	// Load material from file
	{
		std::string mtlFilename = RFileUtil::ReplaceExtension(GetFileSystemPath(), "rmtl");
		RMaterial::LoadFromXmlFile(mtlFilename, materials);

		if (materials.size())
		{
			SetMaterials(materials.data(), (UINT)materials.size());
		}
	}

	OnLoadingFinished(bIsAsyncLoading);

	return true;
}
