//=============================================================================
// RSMeshObject.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "RSMeshObject.h"

#include "RScene.h"
#include "RenderSystem/RMesh.h"
#include "RenderSystem/RRenderSystem.h"

#include "Resource/RResourceManager.h"
#include "Core/RFileUtil.h"

#include "tinyxml2/tinyxml2.h"

IMPLEMENT_SCENE_OBJECT(RSMeshObject);

RSMeshObject::RSMeshObject(const RConstructingParams& Params)
	: RSceneObject(Params),
	  m_Mesh(nullptr),
	  m_bNeedUpdateMaterial(true)
{

}

RSMeshObject::~RSMeshObject()
{

}

RSceneObject* RSMeshObject::Clone()
{
	if (m_Scene)
	{
		RSMeshObject* pClone = m_Scene->CreateMeshObject(m_Mesh);
		pClone->m_Name = m_Scene->GenerateUniqueObjectNameForClone(m_Name);
		pClone->SetTransform(*GetTransform());
		pClone->m_Materials = m_Materials;
		pClone->m_bNeedUpdateMaterial = false;

		return pClone;
	}

	return nullptr;
}

void RSMeshObject::SetMesh(RMesh* mesh)
{
	m_Mesh = mesh;
	m_bNeedUpdateMaterial = true;
}

RMesh* RSMeshObject::GetMesh() const
{
	return m_Mesh;
}

int RSMeshObject::GetMeshElementCount() const
{
	if (m_Mesh)
		return (int)m_Mesh->GetMeshElementCount();
	return 0;
}

void RSMeshObject::SetMaterials(const std::vector<RMaterial*>& materials)
{
	m_Materials.clear();

	for (int i = 0; i < (int)materials.size(); i++)
	{
		assert(materials[i] != nullptr);
		m_Materials.push_back(materials[i]);
	}

	m_bNeedUpdateMaterial = false;
}

void RSMeshObject::SetMaterialSlot(int SlotId, RMaterial* Material)
{
	if (SlotId >= m_Materials.size())
	{
		m_Materials.resize(SlotId + 1);
	}

	m_Materials[SlotId] = Material;
}

RMaterial* RSMeshObject::GetMaterial(int index)
{
	SetupMaterialsFromMeshResource();

	if (index >= (int)m_Materials.size())
	{
		return RMaterial::GetDefault();
	}

	return m_Materials[index];
}

void RSMeshObject::SaveMaterialsToDiskAsDefaults()
{
	// No mesh is assigned to the mesh object, stop
	if (!m_Mesh)
	{
		return;
	}

	// Hack: when a mesh is still in async loading, wait until it's done
	while (!m_Mesh->IsLoaded())
	{
		Sleep(10);
	}

	SetupMaterialsFromMeshResource();

	std::unique_ptr<tinyxml2::XMLDocument> doc = std::make_unique<tinyxml2::XMLDocument>();

	// Save original mesh path in the comment
	doc->InsertEndChild(doc->NewComment((std::string("Mesh path: ") + m_Mesh->GetAssetPath()).c_str()));

	tinyxml2::XMLElement* elem_mat = doc->NewElement("Material");
	SerializeXmlMaterials_Save(doc.get(), elem_mat);
	doc->InsertEndChild(elem_mat);

	std::string filepath = RFileUtil::StripExtension(m_Mesh->GetFileSystemPath()) + ".rmtl";
	doc->SaveFile(filepath.c_str());
}

void RSMeshObject::SerializeXmlMaterials_Load(tinyxml2::XMLElement* XmlElemMaterial)
{
	std::vector<RMaterial*> MeshMaterials;

	if (XmlElemMaterial)
	{
		// Start with filling all materials by ones from the mesh asset
		for (int i = 0; i < GetMeshElementCount(); i++)
		{
			RMaterial* Material = i < GetNumMaterials() ? GetMaterial(i) : nullptr;

			if (Material == nullptr)
			{
				Material = RMaterial::GetDefault();
			}

			MeshMaterials.push_back(Material);
		}

		// <MeshElement Index="0" Name="Element">/Path/To/Material</MeshElement>
		tinyxml2::XMLElement* XmlElemSubmesh = XmlElemMaterial->FirstChildElement("MeshElement");
		while (XmlElemSubmesh)
		{
			int index = -1;
			XmlElemSubmesh->QueryIntAttribute("Index", &index);

			if (index != -1)
			{
				RMaterial* Material = nullptr;
				const char* MaterialPath = XmlElemSubmesh->GetText();
				if (MaterialPath)
				{
					Material = RResourceManager::Instance().FindResource<RMaterial>(MaterialPath);
					if (Material == nullptr)
					{
						Material = RResourceManager::Instance().LoadResource<RMaterial>(MaterialPath, EResourceLoadMode::Immediate);
					}
				}

				if (Material == nullptr)
				{
					Material = RMaterial::GetDefault();
				}

				MeshMaterials[index] = Material;
			}

			XmlElemSubmesh = XmlElemSubmesh->NextSiblingElement();
		}
	}

	if (MeshMaterials.size())
	{
		SetMaterials(MeshMaterials);
	}
}

void RSMeshObject::SerializeXmlMaterials_Save(tinyxml2::XMLDocument* XmlDoc, tinyxml2::XMLElement* XmlElemMaterial)
{
	::SerializeXmlMaterials_Save(m_Materials, XmlDoc, XmlElemMaterial);
}

const RAabb& RSMeshObject::GetMeshElementAabb(int index) const
{
	if (m_Mesh)
		return m_Mesh->GetMeshElementAabb(index);
	return RAabb::Default;
}

void RSMeshObject::Draw()
{
	Draw(false, 0);
}

void RSMeshObject::Draw(bool instanced, int instanceCount)
{
	if (!m_Mesh || !m_Mesh->IsLoaded())
		return;

	SetupMaterialsFromMeshResource();

	for (int i = 0; i < m_Mesh->GetMeshElementCount(); i++)
	{
		const RMeshElement& MeshElement = m_Mesh->GetMeshElement(i);
		bool bSkinned = MeshElement.GetFlag() & MEF_Skinned;
		RMaterial* Material = (i < (int)m_Materials.size()) ? m_Materials[i] : nullptr;
		GRenderer.BindMaterial(Material, bSkinned, instanced);

		if (instanced)
		{
			MeshElement.DrawInstanced(instanceCount);
		}
		else
		{
			MeshElement.Draw();
		}
	}
}

void RSMeshObject::DrawDepthPass()
{
	DrawDepthPass(false, 0);
}

void RSMeshObject::DrawDepthPass(bool instanced, int instanceCount)
{
	if (!m_Mesh || !m_Mesh->IsLoaded())
		return;

	for (int i = 0; i < m_Mesh->GetMeshElementCount(); i++)
	{
		const RMeshElement& MeshElement = m_Mesh->GetMeshElement(i);
		bool bSkinned = MeshElement.GetFlag() & MEF_Skinned;
		RMaterial* Material = RMaterial::GetDepthOnly();
		GRenderer.BindMaterial(Material, bSkinned, instanced);

		if (instanced)
		{
			MeshElement.DrawInstanced(instanceCount);
		}
		else
		{
			MeshElement.Draw();
		}
	}
}

float RSMeshObject::GetResourceTimestamp()
{
	if (m_Mesh)
		return m_Mesh->GetResourceTimestamp();

	return 0.0f;
}

void RSMeshObject::CalculateBounds()
{
	Base::CalculateBounds();

	if (m_Mesh)
	{
		RAabb MeshBounds = m_Mesh->GetLocalSpaceAabb().GetTransformedAabb(m_NodeTransform.GetMatrix());
		Bounds.Expand(MeshBounds);
	}
}

void RSMeshObject::SetupMaterialsFromMeshResource()
{
	if (m_bNeedUpdateMaterial)
	{
		m_Materials = m_Mesh->GetMaterials();

		for (unsigned int i = 0; i < m_Materials.size(); i++)
		{
			if (m_Materials[i] == nullptr)
			{
				m_Materials[i] = RMaterial::GetDefault();
			}
		}

		m_bNeedUpdateMaterial = false;
	}
}
