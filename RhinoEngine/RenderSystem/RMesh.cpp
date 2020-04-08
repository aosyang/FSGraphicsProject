//=============================================================================
// RMesh.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "RMesh.h"

#include "Resource/RFbxMeshLoader.h"
#include "Resource/RResourceManager.h"
#include "RAnimation.h"
#include "RTexture.h"

#include "Core/RLog.h"

#include "tinyxml2/tinyxml2.h"
#include "D3DUtil.h"

// Whether to export .fbx as .rmesh after loading
#define EXPORT_FBX_AS_BINARY_MESH 1

RMesh::RMesh(const std::string& Path)
	: RResourceBase(Path),
	  m_Animation(nullptr)
{
}

//RMesh::RMesh(const std::string& Path, const std::vector<RMeshElement>& meshElements, const std::vector<RMeshMaterialData>& materials)
//	: RMesh(Path)
//{
//	m_MeshElements = meshElements;
//	m_Materials = materials;
//}
//
//RMesh::RMesh(const std::string& Path, const RMeshElement* meshElements, int numElement, const RMeshMaterialData* materials, int numMaterial)
//	: RMesh(Path)
//{
//	assert(meshElements && numElement);
//	m_MeshElements.assign(meshElements, meshElements + numElement);
//
//	if (materials && numMaterial)
//		m_Materials.assign(materials, materials + numMaterial);
//	else
//	{
//		RMeshMaterialData emptyMaterial;
//		ZeroMemory(&emptyMaterial, sizeof(emptyMaterial));
//		for (int i = 0; i < numElement; i++)
//		{
//			m_Materials.push_back(emptyMaterial);
//		}
//	}
//}

RMesh::~RMesh()
{
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

	if (serializer.IsReading())
	{
		UINT NumElements = 0;
		serializer.SerializeData(NumElements);
		m_MeshElements.resize(NumElements);
		for (UINT i = 0; i < NumElements; i++)
		{
			m_MeshElements[i] = std::make_unique<RMeshElement>();
			serializer.SerializeObject(*m_MeshElements[i]);
		}
	}
	else
	{
		UINT NumElements = (UINT)m_MeshElements.size();
		serializer.SerializeData(NumElements);
		for (UINT i = 0; i < NumElements; i++)
		{
			serializer.SerializeObject(*m_MeshElements[i]);
		}
	}

	if (serializer.IsReading())
	{
		size_t NumMaterials = 0;
		serializer.SerializeData(NumMaterials);
		m_Materials.resize(NumMaterials);
		for (int i = 0; i < (int)NumMaterials; i++)
		{
			std::string MaterialPath;
			serializer.SerializeData(MaterialPath);

			// Sanity check
			assert(MaterialPath.size() < MAX_PATH);

			RMaterial* ReferencedMaterial = nullptr;

			if (MaterialPath == "" || MaterialPath == "DefaultMaterial")
			{
				ReferencedMaterial = RMaterial::GetDefault();
			}
			else
			{
				ReferencedMaterial = RResourceManager::Instance().FindResource<RMaterial>(MaterialPath);
			}

			if (!ReferencedMaterial)
			{
				ReferencedMaterial = RResourceManager::Instance().LoadResource<RMaterial>(MaterialPath, EResourceLoadMode::Immediate);
			}

			m_Materials[i] = ReferencedMaterial;
		}
	}
	else
	{
		size_t NumMaterials = m_Materials.size();
		serializer.SerializeData(NumMaterials);
		for (int i = 0; i < (int)m_Materials.size(); i++)
		{
			std::string AssetPath = m_Materials[i]->GetAssetPath();

			// TODO Fix-me: Wrong template function is called if the parameter is a type of const std::string&
			//				This will lead to corrupted data during loading
			serializer.SerializeData(AssetPath);
		}
	}

	serializer.SerializeObjectPtr(&m_Animation);
	if (serializer.IsReading() && m_Animation)
	{
		m_Animation->SetName(GetAssetPath());

		std::string SkeletalMeshName = GetMetaData()["SkeletalMesh"];
		if (SkeletalMeshName != "")
		{
			RMesh* SkeletalMesh = RResourceManager::Instance().LoadResource<RMesh>(SkeletalMeshName, EResourceLoadMode::Immediate);
			if (SkeletalMesh)
			{
				SkeletalMesh->CacheAnimation(m_Animation);
			}
		}
	}

	serializer.SerializeVector(m_BoneInitInvMatrices);
	serializer.SerializeVector(m_BoneIdToName, &RSerializer::SerializeData);

	if (serializer.IsReading())
	{
		UpdateAabb();
	}
}

bool RMesh::LoadResourceImpl()
{
	if (TryLoadAsRmesh())
	{
		return true;
	}

	return TryLoadAsFbxMesh();
}

const RMaterial* RMesh::GetMaterial(int index) const
{
	if (index >= 0 && index < (int)m_Materials.size())
	{
		return m_Materials[index];
	}

	return RMaterial::GetDefault();
}

void RMesh::SetMeshElements(std::vector<std::unique_ptr<RMeshElement>>&& Elements)
{
	m_MeshElements = std::move(Elements);
}

void RMesh::SetMaterialSlot(int SlotId, RMaterial* Material)
{
	if (SlotId >= m_Materials.size())
	{
		m_Materials.resize(SlotId + 1);
	}

	m_Materials[SlotId] = Material;
}

void RMesh::SetMaterials(const std::vector<RMaterial*> NewMaterials)
{
	//assert(NewMaterials.size() > 0);
	m_Materials = NewMaterials;
}

void RMesh::SaveMaterialsToDiskAsDefaults()
{
	std::unique_ptr<tinyxml2::XMLDocument> XmlDoc = std::make_unique<tinyxml2::XMLDocument>();

	// Save original mesh path in the comment
	XmlDoc->InsertEndChild(XmlDoc->NewComment((std::string("Mesh path: ") + GetAssetPath()).c_str()));

	tinyxml2::XMLElement* XmlElemMaterial = XmlDoc->NewElement("Material");
	SerializeXmlMaterials_Save(m_Materials, XmlDoc.get(), XmlElemMaterial);
	XmlDoc->InsertEndChild(XmlElemMaterial);

	std::string filepath = RFileUtil::StripExtension(GetFileSystemPath()) + ".rmtl";
	XmlDoc->SaveFile(filepath.c_str());
}

void RMesh::UpdateAabb()
{
	m_Aabb = RAabb::Default;
	for (UINT32 i = 0; i < m_MeshElements.size(); i++)
	{
		m_Aabb.Expand(m_MeshElements[i]->GetAabb());
	}
}

const RAabb& RMesh::GetLocalSpaceAabb() const
{
	return m_Aabb;
}

const RAabb& RMesh::GetMeshElementAabb(int index) const
{
	return m_MeshElements[index]->GetAabb();
}

void RMesh::SetAnimation(RAnimation* anim)
{
	m_Animation = anim;
}

RAnimation* RMesh::GetAnimation() const
{
	return m_Animation;
}

bool RMesh::HasAnySkinnedMeshElements() const
{
	for (const auto& MeshElement : m_MeshElements)
	{
		if (MeshElement->GetFlag() & MEF_Skinned)
		{
			return true;
		}
	}

	return false;
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

void RMesh::CacheAnimation(RAnimation* Animation)
{
	if (!Animation)
	{
		RLogWarning("Failed to cache animation for mesh \'%s\': Input animation is null!\n", GetAssetPath().c_str());
		return;
	}

	if (GetResourceState() != RS_Loaded)
	{
		// The resource has not finished loading, skip caching the animation
		return;
	}
		
	if (!m_BoneIdToName.size())
	{
		RLogWarning("Failed to cache animation for mesh \'%s\': Mesh does not have any bones!\n", GetAssetPath().c_str());
		return;
	}

	// Animation is already cached for this mesh, skip.
	if (m_AnimationNodeCache.find(Animation) != m_AnimationNodeCache.end())
	{
		return;
	}

	const char* rootNodeName = m_BoneIdToName[0].data();
	Animation->SetRootNode(Animation->GetNodeIdByName(rootNodeName));
	Animation->BuildRootDisplacements();

	std::vector<int> nodeIdMap;
	nodeIdMap.resize(m_BoneIdToName.size());
	for (int i = 0; i < (int)m_BoneIdToName.size(); i++)
	{
		nodeIdMap[i] = Animation->GetNodeIdByName(m_BoneIdToName[i].data());
	}

	m_AnimationNodeCache[Animation] = nodeIdMap;
}

bool RMesh::HasCachedAnimation(RAnimation* anim) const
{
	if (anim)
	{
		return m_AnimationNodeCache.find(anim) != m_AnimationNodeCache.end();
	}

	return false;
}

int RMesh::GetCachedAnimationNodeId(RAnimation* Animation, int BoneId) const
{
	if (!Animation || !m_BoneIdToName.size())
		return -1;

	auto Iter = m_AnimationNodeCache.find(Animation);
	if (Iter == m_AnimationNodeCache.end())
		return -1;

	return Iter->second[BoneId];
}

EMeshCollisionType RMesh::GetCollisionType() const
{
	const std::string CollisionValue = GetMetaData()["CollisionType"];
	if (CollisionValue == "TriangleMesh")
	{
		return EMeshCollisionType::TriangleMesh;
	}

	return EMeshCollisionType::BoundingBox;
}

std::vector<RResourceBase*> RMesh::EnumerateReferencedResources() const
{
	std::vector<RResourceBase*> ReferencedResources;

	for (const auto& Material : m_Materials)
	{
		for (const RTextureSlotData& SlotData : Material->GetTextureSlots())
		{
			RTexture* Texture = SlotData.Texture;
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

bool RMesh::TryLoadAsFbxMesh()
{
	std::unique_ptr<RFbxMeshLoader> FbxMeshLoader(new RFbxMeshLoader);

	if (FbxMeshLoader->LoadDataForMeshResource(this, GetFileSystemPath()))
	{
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

bool RMesh::TryLoadAsRmesh()
{
	RLog("Loading mesh %s\n", GetAssetPath().c_str());

	std::vector<RMeshElement> meshElements;
	const std::string BinaryMeshPath = RFileUtil::ReplaceExtension(GetFileSystemPath(), "rmesh");

	// Binary mesh file doesn't exist, load fbx instead
	if (!RFileUtil::CheckPathExists(BinaryMeshPath))
	{
		return false;
	}

	// Compare timestamps of .fbx and .rmesh
	ETimestampComparison Result = RFileUtil::CompareFileTimestamp(GetFileSystemPath(), BinaryMeshPath);

	// Both files should be valid
	assert(Result != ETimestampComparison::InvalidFile);

	// If .fbx is newer than binary mesh, stop here. We will load the fbx and regenerate the binary mesh.
	if (Result == ETimestampComparison::EarlierSecond)
	{
		return false;
	}

	// The binary mesh is up-to-date, load it now
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
		std::vector<std::string> MaterialPaths = RMaterial::LoadNameListFromXml(mtlFilename);
		std::vector<RMaterial*> Materials;

		for (auto Iter : MaterialPaths)
		{
			RMaterial* Material = RResourceManager::Instance().FindResource<RMaterial>(Iter);

			if (!Material)
			{
				Material = RResourceManager::Instance().LoadResource<RMaterial>(Iter, EResourceLoadMode::Immediate);
			}

			if (!Material)
			{
				Material = RMaterial::GetDefault();
			}

			assert(Material);
			Materials.push_back(Material);
		}

		if (Materials.size())
		{
			SetMaterials(Materials);
		}
	}

	return true;
}

void SerializeXmlMaterials_Save(const std::vector<RMaterial*>& Materials, tinyxml2::XMLDocument* XmlDoc, tinyxml2::XMLElement* XmlElemMaterial)
{
	// <Material>
	for (int i = 0; i < (int)Materials.size(); i++)
	{
		// <MeshElement Index="0" Name="Element">/Path/To/Material</MeshElement>
		tinyxml2::XMLElement* XmlElemSubmesh = XmlDoc->NewElement("MeshElement");
		XmlElemSubmesh->SetAttribute("Index", i);
		//XmlElemSubmesh->SetAttribute("Name", m_Mesh->GetMeshElement(i).GetName().c_str());

		RMaterial* Material = Materials[i];
		if (Material && Material->GetAssetPath() != "")
		{
			XmlElemSubmesh->SetText(Material->GetAssetPath().c_str());
		}
		XmlElemMaterial->InsertEndChild(XmlElemSubmesh);
	}
}
