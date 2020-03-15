//=============================================================================
// RRenderMeshComponent.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "Scene/RSceneComponent.h"

#include "RRenderSystemTypes.h"
#include "RMaterial.h"

struct PendingAssignedMaterial
{
	UINT		Index;
	RMaterial*	Material;
};

class RRenderMeshComponent : public RSceneComponent
{
	DECLARE_SCENE_COMPONENT(RRenderMeshComponent, RSceneComponent);
public:
	virtual ~RRenderMeshComponent() override;

	virtual void Update(float DeltaTime) override;

	/// Render the component
	void Render(const RenderViewInfo& View) const;

	/// Render the component in depth pass for shadow map
	void RenderDepthPass(const RenderViewInfo& View) const;

	/// Set mesh resource for the component to render
	void SetMesh(const RMesh* Mesh);

	void SetMaterial(UINT Index, RMaterial* Material);

protected:
	void OnMeshLoaded();

private:
	RRenderMeshComponent(RSceneObject* InOwner);

	void LoadMaterialsFromMeshResource();

	const RMesh*			m_Mesh;
	std::vector<RMaterial*>		m_Materials;

	bool					m_PostponeLoadMaterials;
	std::vector<PendingAssignedMaterial>	m_PendingAssignedMaterials;
};
