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

void RRenderMeshComponent::Update(float DeltaTime)
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

void RRenderMeshComponent::Render(const RenderViewInfo& View) const
{
	if (m_Mesh && m_Mesh->IsLoaded() && !m_PostponeLoadMaterials)
	{
		const RMatrix4& Matrix = GetOwner()->GetTransformMatrix();
		RAabb MeshAabb = m_Mesh->GetLocalSpaceAabb().GetTransformedAabb(Matrix);
		if (RCollision::TestAabbInsideFrustum(*View.Frustum, MeshAabb))
		{
			// Set up world matrix in constant buffer
			RConstantBuffers::cbPerObject.Data.worldMatrix = Matrix;

			RConstantBuffers::cbPerObject.UpdateBufferData();
			RConstantBuffers::cbPerObject.BindBuffer();

			const UINT32 NumMeshElements = (UINT32)m_Mesh->GetMeshElements().size();
			const UINT32 NumMaterials = (UINT32)m_Materials.size();

			for (UINT32 i = 0; i < NumMeshElements; i++)
			{
				const RMeshElement& MeshElement = m_Mesh->GetMeshElements()[i];
				bool bSkinned = MeshElement.GetFlag() & MEF_Skinned;
				RMaterial* Material = (i < (int)m_Materials.size()) ? m_Materials[i] : nullptr;
				GRenderer.BindMaterial(Material, bSkinned);
				m_Mesh->GetMeshElements()[i].Draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			}
		}
	}
}

void RRenderMeshComponent::RenderDepthPass(const RenderViewInfo& View) const
{
	if (m_Mesh)
	{
		if (m_Mesh->IsLoaded())
		{
			const RMatrix4& Matrix = GetOwner()->GetTransformMatrix();
			RAabb MeshAabb = m_Mesh->GetLocalSpaceAabb().GetTransformedAabb(Matrix);
			if (RCollision::TestAabbInsideFrustum(*View.Frustum, MeshAabb))
			{
				// Set up world matrix in constant buffer
				RConstantBuffers::cbPerObject.Data.worldMatrix = Matrix;

				RConstantBuffers::cbPerObject.UpdateBufferData();
				RConstantBuffers::cbPerObject.BindBuffer();

				const UINT32 NumMeshElements = (UINT32)m_Mesh->GetMeshElements().size();
				const UINT32 NumMaterials = (UINT32)m_Materials.size();

				for (UINT32 i = 0; i < NumMeshElements; i++)
				{
					static RShader* DepthShader = RShaderManager::Instance().GetShaderResource("Depth");
					assert(DepthShader);

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

					DepthShader->Bind(shaderFeatureMask);
					m_Mesh->GetMeshElements()[i].Draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				}
			}
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

void RRenderMeshComponent::SetMaterial(UINT Index, RMaterial* Material)
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

	RShader* DefaultShader = RShaderManager::Instance().GetDefaultShader();
	assert(DefaultShader);

	// Assign default material to empty material slots
	for (UINT32 i = 0; i < NumMeshElements; i++)
	{
		//if (m_Materials[i].Shader == nullptr)
		//{
		//	m_Materials[i] = DefaultMaterial;
		//}
		//else 
		if (i >= NumMaterials)
		{
			m_Materials.push_back(nullptr);	// TODO: Add default material
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
