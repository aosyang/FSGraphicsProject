//=============================================================================
// RMesh.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"
#include "RMesh.h"

RMesh::RMesh(ID3D11InputLayout* inputLayout)
{
	m_LoadingFinishTime = 0.0f;
	m_InputLayout = inputLayout;
}

RMesh::RMesh(const vector<RMeshElement> meshElements, const vector<RMaterial>& materials, ID3D11InputLayout* inputLayout)
{
	m_LoadingFinishTime = 0.0f;
	m_MeshElements = meshElements;
	m_Materials = materials;
	m_InputLayout = inputLayout;
}

RMesh::RMesh(RMeshElement* meshElements, int numElement, RMaterial* materials, int numMaterial, ID3D11InputLayout* inputLayout)
{
	m_LoadingFinishTime = 0.0f;

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

	m_InputLayout = inputLayout;
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

int RMesh::GetSubmeshCount() const
{
	return (int)m_MeshElements.size();
}

ID3D11InputLayout* RMesh::GetInputLayout() const
{
	return m_InputLayout;
}

vector<RMeshElement>& RMesh::GetMeshElements()
{
	return m_MeshElements;
}

void RMesh::SetMeshElements(RMeshElement* meshElements, int numElement)
{
	// TODO: use mutex
	assert(meshElements && numElement);
	m_MeshElements.assign(meshElements, meshElements + numElement);
}

void RMesh::SetResourceTimestamp(float time)
{
	m_LoadingFinishTime = time;
}

float RMesh::GetResourceTimestamp()
{
	return m_LoadingFinishTime;
}
