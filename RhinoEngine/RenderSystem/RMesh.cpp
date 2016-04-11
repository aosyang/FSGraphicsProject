//=============================================================================
// RMesh.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"
#include "RMesh.h"

RMesh::RMesh(string path)
	: RBaseResource(RT_Mesh, path),
	  m_InputLayout(nullptr)
{
	m_LoadingFinishTime = 0.0f;
}

RMesh::RMesh(string path, const vector<RMeshElement> meshElements, const vector<RMaterial>& materials)
	: RMesh(path)
{
	m_MeshElements = meshElements;
	m_Materials = materials;
}

RMesh::RMesh(string path, RMeshElement* meshElements, int numElement, RMaterial* materials, int numMaterial)
	: RMesh(path)
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
}

RMaterial RMesh::GetMaterial(int index) const
{
	assert(index >= 0 && index < (int)m_Materials.size());
	return m_Materials[index];
}

vector<RMaterial>& RMesh::GetMaterials()
{
	return m_Materials;
}

void RMesh::SetInputLayout(ID3D11InputLayout* inputLayout)
{
	m_InputLayout = inputLayout;
}

ID3D11InputLayout* RMesh::GetInputLayout() const
{
	return m_InputLayout;
}

vector<RMeshElement>& RMesh::GetMeshElements()
{
	return m_MeshElements;
}

int RMesh::GetMeshElementCount() const
{
	return (int)m_MeshElements.size();
}

void RMesh::SetMeshElements(RMeshElement* meshElements, int numElement)
{
	// TODO: use mutex
	assert(meshElements && numElement);
	m_MeshElements.assign(meshElements, meshElements + numElement);
}

void RMesh::SetMaterials(RMaterial* materials, int numMaterial)
{
	assert(materials && numMaterial);
	m_Materials.assign(materials, materials + numMaterial);
}

void RMesh::SetAabb(const RAabb& aabb)
{
	m_Aabb = aabb;
}

const RAabb& RMesh::GetAabb() const
{
	return m_Aabb;
}

const RAabb& RMesh::GetMeshElementAabb(int index) const
{
	return m_MeshElements[index].GetAabb();
}

void RMesh::SetResourceTimestamp(float time)
{
	m_LoadingFinishTime = time;
}

float RMesh::GetResourceTimestamp()
{
	return m_LoadingFinishTime;
}
