//=============================================================================
// RSMeshObject.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"
#include "RSMeshObject.h"

RSMeshObject::RSMeshObject()
	: RSceneObject()
{

}

RSMeshObject::~RSMeshObject()
{

}

void RSMeshObject::SetMesh(RMesh* mesh)
{
	m_Mesh = mesh;
	m_Materials = mesh->GetMaterials();
}

int RSMeshObject::GetSubmeshCount() const
{
	if (m_Mesh)
		return (int)m_Mesh->GetSubmeshCount();
	return 0;
}

void RSMeshObject::SetMaterial(RMaterial* materials, int materialNum)
{
	m_Materials.clear();

	for (int i = 0; i < materialNum; i++)
	{
		m_Materials.push_back(materials[i]);
	}
}

RMaterial RSMeshObject::GetMaterial(int index) const
{
	return m_Materials[index];
}

void RSMeshObject::Draw()
{
	if (!m_Mesh)
		return;

	RRenderer.D3DImmediateContext()->IASetInputLayout(m_Mesh->GetInputLayout());

	for (UINT32 i = 0; i < m_Mesh->GetMeshElements().size(); i++)
	{
		if (m_Materials[i].Shader)
		{
			m_Materials[i].Shader->Bind();
			for (int t = 0; t < m_Materials[i].TextureNum; t++)
			{
				RRenderer.D3DImmediateContext()->PSSetShaderResources(t, 1, &m_Materials[i].Textures[t]);
			}
		}

		m_Mesh->GetMeshElements()[i].Draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
}
