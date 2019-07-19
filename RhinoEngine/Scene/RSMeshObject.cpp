//=============================================================================
// RSMeshObject.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"
#include "RSMeshObject.h"

#include "tinyxml2/tinyxml2.h"

IMPLEMENT_SCENE_OBJECT(RSMeshObject);

RSMeshObject::RSMeshObject(const RConstructingParams& Params)
	: RSceneObject(Params),
	  m_Mesh(nullptr),
	  m_OverridingShader(nullptr),
	  m_OverridingShaderFeatures(-1),
	  m_bNeedUpdateMaterial(true)
{

}

RSMeshObject::~RSMeshObject()
{

}

RSceneObject* RSMeshObject::Clone() const
{
	RSMeshObject* pClone = new RSMeshObject(*this);
	return pClone;
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

void RSMeshObject::SetMaterial(RMaterial* materials, int materialNum)
{
	m_Materials.clear();

	for (int i = 0; i < materialNum; i++)
	{
		m_Materials.push_back(materials[i]);
	}

	m_bNeedUpdateMaterial = false;
}

RMaterial* RSMeshObject::GetMaterial(int index)
{
	UpdateMaterialsFromResource();

	return &m_Materials[index];
}

void RSMeshObject::SaveMaterialsToFile()
{
	if (!m_Mesh)
		return;

	while (!m_Mesh->IsLoaded())
	{
		Sleep(10);
	}

	UpdateMaterialsFromResource();

	tinyxml2::XMLDocument* doc = new tinyxml2::XMLDocument();
	doc->InsertEndChild(doc->NewComment((string("Mesh path: ") + m_Mesh->GetFileSystemPath()).c_str()));

	tinyxml2::XMLElement* elem_mat = doc->NewElement("Material");

	SerializeMaterialsToXML(doc, elem_mat);
	doc->InsertEndChild(elem_mat);

	string filepath = m_Mesh->GetFileSystemPath();
	filepath = filepath.substr(0, filepath.length() - 3);
	filepath += "rmtl";

	doc->SaveFile(filepath.c_str());

	delete doc;
}

void RSMeshObject::SerializeMaterialsToXML(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* elem_mat)
{
	for (int i = 0; i < m_Mesh->GetMeshElementCount(); i++)
	{
		tinyxml2::XMLElement* elem_submesh = doc->NewElement("MeshElement");
		elem_submesh->SetAttribute("Name", m_Mesh->GetMeshElements()[i].GetName().c_str());

		if (i < (int)m_Materials.size())
		{
			RMaterial& material = m_Materials[i];
			string shaderName = material.Shader->GetName();

			elem_submesh->SetAttribute("Shader", shaderName.c_str());
			for (int t = 0; t < material.TextureNum; t++)
			{
				string texturePath = "";

				if (material.Textures[t])
					texturePath = material.Textures[t]->GetAssetPath();

				tinyxml2::XMLElement* elem_texture = doc->NewElement("Texture");
				elem_texture->SetAttribute("Slot", t);
				elem_texture->SetText(texturePath.c_str());
				elem_submesh->InsertEndChild(elem_texture);
			}
		}
		elem_mat->InsertEndChild(elem_submesh);
	}
}

void RSMeshObject::SetOverridingShader(RShader* shader, int features)
{
	m_OverridingShader = shader;
	m_OverridingShaderFeatures = features;
}

const RAabb& RSMeshObject::GetAabb()
{
	// Update aabb for mesh with current transform
	if (m_Mesh)
	{
		m_MeshAABB = m_Mesh->GetLocalSpaceAabb().GetTransformedAabb(m_NodeTransform.GetMatrix());
		return m_MeshAABB;
	}

	return RAabb::Default;
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

	UpdateMaterialsFromResource();

	for (UINT32 i = 0; i < m_Mesh->GetMeshElements().size(); i++)
	{
		RShader* shader = nullptr;

		if (m_OverridingShader)
			shader = m_OverridingShader;
		else if (i < m_Materials.size())
			shader = m_Materials[i].Shader;

		if (shader)
		{
			int flag = m_Mesh->GetMeshElements()[i].GetFlag();

			int shaderFeatureMask = 0;
			if ((flag & MEF_Skinned) && !GEngine.IsEditor())
			{
				shaderFeatureMask |= SFM_Skinned;
			}
			else if (instanced)
				shaderFeatureMask |= SFM_Instanced;

			if (GRenderer.IsUsingDeferredShading())
				shaderFeatureMask |= SFM_Deferred;

			if (m_OverridingShader && m_OverridingShaderFeatures != -1)
				shader->Bind(m_OverridingShaderFeatures);
			else
				shader->Bind(shaderFeatureMask);

			// Hack: for shaders bound separately, consider textures loaded from mesh
			if (m_OverridingShader)
			{
				for (int t = 0; t < m_Mesh->GetMaterial(i).TextureNum; t++)
				{
					GRenderer.D3DImmediateContext()->PSSetShaderResources(t, 1, m_Mesh->GetMaterial(i).Textures[t]->GetPtrSRV());
				}
			}
			else
			{
				ID3D11ShaderResourceView* NullShaderResourceView[] = { nullptr };

				for (int t = 0; t < m_Materials[i].TextureNum; t++)
				{
					RTexture* texture = m_Materials[i].Textures[t];
					GRenderer.D3DImmediateContext()->PSSetShaderResources(t, 1, texture ? texture->GetPtrSRV() : NullShaderResourceView);
				}
			}
		}

		if (instanced)
			m_Mesh->GetMeshElements()[i].DrawInstanced(instanceCount, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		else
			m_Mesh->GetMeshElements()[i].Draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
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

	static RShader* DefaultShader = RShaderManager::Instance().GetShaderResource("Depth");

	for (UINT32 i = 0; i < m_Mesh->GetMeshElements().size(); i++)
	{
		RShader* shader = DefaultShader;

		int flag = m_Mesh->GetMeshElements()[i].GetFlag();
		int shaderFeatureMask = 0;

		if ((flag & MEF_Skinned) && !GEngine.IsEditor())
			shaderFeatureMask |= SFM_Skinned;
		else if (instanced)
			shaderFeatureMask |= SFM_Instanced;

		shader->Bind(shaderFeatureMask);

		if (instanced)
			m_Mesh->GetMeshElements()[i].DrawInstanced(instanceCount, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		else
			m_Mesh->GetMeshElements()[i].Draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
}

void RSMeshObject::DrawWithShader(RShader* shader, bool instanced, int instanceCount)
{
	if (!m_Mesh || !m_Mesh->IsLoaded() || !shader)
		return;

	//RRenderer.D3DImmediateContext()->IASetInputLayout(m_Mesh->GetInputLayout());

	for (UINT32 i = 0; i < m_Mesh->GetMeshElements().size(); i++)
	{
		int flag = m_Mesh->GetMeshElements()[i].GetFlag();
		int shaderFeatureMask = 0;

		if (flag & MEF_Skinned)
			shaderFeatureMask |= SFM_Skinned;
		else if (instanced)
			shaderFeatureMask |= SFM_Instanced;

		if (GRenderer.IsUsingDeferredShading())
			shaderFeatureMask |= SFM_Deferred;

		shader->Bind(shaderFeatureMask);

		if (instanced)
			m_Mesh->GetMeshElements()[i].DrawInstanced(instanceCount, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		else
			m_Mesh->GetMeshElements()[i].Draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
}

float RSMeshObject::GetResourceTimestamp()
{
	if (m_Mesh)
		return m_Mesh->GetResourceTimestamp();

	return 0.0f;
}

void RSMeshObject::UpdateMaterialsFromResource()
{
	if (m_bNeedUpdateMaterial)
	{
		m_Materials = m_Mesh->GetMaterials();

		for (unsigned int i = 0; i < m_Materials.size(); i++)
		{
			if (m_Materials[i].Shader == nullptr)
				m_Materials[i].Shader = RShaderManager::Instance().GetShaderResource("Default");
		}

		m_bNeedUpdateMaterial = false;
	}
}
