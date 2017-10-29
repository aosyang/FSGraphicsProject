//=============================================================================
// RRenderMeshComponent.cpp by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#include "Rhino.h"

#include "RRenderMeshComponent.h"

RRenderMeshComponent::RRenderMeshComponent(RSceneObject* InOwner)
	: Base(InOwner),
	  m_Mesh(nullptr),
	  m_PostponeLoadMaterials(false)
{
	GRenderer.RegisterRenderMeshComponent(this);
}

RRenderMeshComponent::~RRenderMeshComponent()
{
	GRenderer.UnregisterRenderMeshComponent(this);
}

RRenderMeshComponent* RRenderMeshComponent::Create(RSceneObject* InOwner)
{
	RRenderMeshComponent* RenderMeshComponent = new RRenderMeshComponent(InOwner);
	return RenderMeshComponent;
}

void RRenderMeshComponent::Update()
{
	if (m_PostponeLoadMaterials)
	{
		if (m_Mesh && m_Mesh->IsLoaded())
		{
			LoadMaterialsFromMeshResource();
			m_PostponeLoadMaterials = false;
		}
	}
}

void RRenderMeshComponent::Render() const
{
	if (m_Mesh && m_Mesh->IsLoaded() && !m_PostponeLoadMaterials)
	{
		// Set up world matrix in constant buffer
		SHADER_OBJECT_BUFFER cbObject;
		cbObject.worldMatrix = GetOwner()->GetTransformMatrix();

		RConstantBuffers::cbPerObject.UpdateBufferData(&cbObject);
		RConstantBuffers::cbPerObject.BindBuffer();

		const UINT32 NumMeshElements = (UINT32)m_Mesh->GetMeshElements().size();
		const UINT32 NumMaterials = (UINT32)m_Materials.size();

		for (UINT32 i = 0; i < NumMeshElements; i++)
		{
			RShader* shader = nullptr;

			if (i < NumMaterials)
			{
				shader = m_Materials[i].Shader;
			}

			assert(shader);

			const RMeshElement& MeshElement = m_Mesh->GetMeshElements()[i];
			int flag = MeshElement.GetFlag();

			int shaderFeatureMask = 0;
			if ((flag & MEF_Skinned) && !GEngine.IsEditor())
			{
				shaderFeatureMask |= SFM_Skinned;
			}

			if (GRenderer.IsUsingDeferredShading())
			{
				shaderFeatureMask |= SFM_Deferred;
			}

			shader->Bind(shaderFeatureMask);

			ID3D11ShaderResourceView* NullShaderResourceView[] = { nullptr };

			for (int t = 0; t < m_Materials[i].TextureNum; t++)
			{
				RTexture* texture = m_Materials[i].Textures[t];
				GRenderer.D3DImmediateContext()->PSSetShaderResources(t, 1, texture ? texture->GetPtrSRV() : NullShaderResourceView);

				GRenderer.SetSamplerState(t, SamplerState_Texture);
			}

			m_Mesh->GetMeshElements()[i].Draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		}
	}
}

void RRenderMeshComponent::SetMesh(const RMesh* Mesh)
{
	m_Mesh = Mesh;

	if (m_Mesh)
	{
		if (m_Mesh->IsLoaded())
		{
			LoadMaterialsFromMeshResource();
		}
		else
		{
			// Mesh resource may not be fully loaded yet due to multi-threaded loading, postpone material assigning
			m_PostponeLoadMaterials = true;
		}
	}
}

void RRenderMeshComponent::SetMaterial(UINT Index, const RMaterial& Material)
{
	if (m_PostponeLoadMaterials)
	{
		m_PendingAssignedMaterials.push_back({ Index, Material });
	}
	else
	{
		if (m_Materials.size() <= Index)
		{
			m_Materials.resize(Index + 1);
		}

		m_Materials[Index] = Material;
	}
}

void RRenderMeshComponent::LoadMaterialsFromMeshResource()
{
	assert(m_Mesh);

	const UINT32 NumMeshElements = (UINT32)m_Mesh->GetMeshElements().size();
	m_Materials.reserve(NumMeshElements);
	m_Materials = m_Mesh->GetMaterials();

	const UINT32 NumMaterials = (UINT32)m_Materials.size();

	RShader* DefaultShader = RShaderManager::Instance().GetShaderResource("Default");
	assert(DefaultShader);

	RMaterial DefaultMaterial;
	ZeroMemory(&DefaultMaterial, sizeof(DefaultMaterial));
	DefaultMaterial.Shader = DefaultShader;

	// Assign default material to empty material slots
	for (UINT32 i = 0; i < NumMeshElements; i++)
	{
		if (m_Materials[i].Shader == nullptr)
		{
			m_Materials[i] = DefaultMaterial;
		}
		else if (i >= NumMaterials)
		{
			m_Materials.push_back(DefaultMaterial);
		}
	}

	if (m_PendingAssignedMaterials.size() != 0)
	{
		for (PendingAssignedMaterial& Op : m_PendingAssignedMaterials)
		{
			if (m_Materials.size() <= Op.Index)
			{
				m_Materials.resize(Op.Index + 1);
			}

			m_Materials[Op.Index] = Op.Material;
		}

		m_PendingAssignedMaterials.clear();
	}
}
