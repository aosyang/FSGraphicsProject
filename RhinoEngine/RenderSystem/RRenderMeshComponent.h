//=============================================================================
// RRenderMeshComponent.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "Scene/RSceneComponentBase.h"

#include "RMaterial.h"

struct PendingAssignedMaterial
{
	UINT		Index;
	RMaterial	Material;
};

struct RenderViewInfo
{
	RFrustum&&	Frustum;
};

class RRenderMeshComponent : public RSceneComponentBase
{
	typedef RSceneComponentBase Base;
public:
	virtual ~RRenderMeshComponent() override;

	/// Static creator function
	static RRenderMeshComponent* Create(RSceneObject* InOwner);

	virtual void Update() override;

	/// Render the component
	void Render(const RenderViewInfo& View) const;

	/// Set mesh resource for the component to render
	void SetMesh(const RMesh* Mesh);

	void SetMaterial(UINT Index, const RMaterial& Material);

private:
	RRenderMeshComponent(RSceneObject* InOwner);

	void LoadMaterialsFromMeshResource();

	const RMesh*			m_Mesh;
	vector<RMaterial>		m_Materials;

	bool					m_PostponeLoadMaterials;
	vector<PendingAssignedMaterial>	m_PendingAssignedMaterials;
};
