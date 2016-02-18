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

void RSMeshObject::Draw(bool instanced, int instanceCount)
{
	if (!m_Mesh)
		return;

	RRenderer.D3DImmediateContext()->IASetInputLayout(m_Mesh->GetInputLayout());

	for (UINT32 i = 0; i < m_Mesh->GetMeshElements().size(); i++)
	{
		RShader* shader = nullptr;

		if (i < m_Materials.size())
			shader = m_Materials[i].Shader;

		if (shader)
		{
			shader->Bind();
			for (int t = 0; t < m_Materials[i].TextureNum; t++)
			{
				RRenderer.D3DImmediateContext()->PSSetShaderResources(t, 1, &m_Materials[i].Textures[t]);
			}
		}

		if (instanced)
			m_Mesh->GetMeshElements()[i].DrawInstanced(instanceCount, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		else
			m_Mesh->GetMeshElements()[i].Draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
}

void RSMeshObject::DrawWithShader(RShader* shader, bool instanced, int instanceCount)
{
	if (!m_Mesh || !shader)
		return;

	RRenderer.D3DImmediateContext()->IASetInputLayout(m_Mesh->GetInputLayout());

	for (UINT32 i = 0; i < m_Mesh->GetMeshElements().size(); i++)
	{
		shader->Bind();

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
