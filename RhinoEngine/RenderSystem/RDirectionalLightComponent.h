//=============================================================================
// RDirectionalLightComponent.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "Scene/RSceneComponentBase.h"
#include "ILight.h"
#include "RShadowMap.h"

struct DirectionalLightParam
{
	RVec3	Direction;
	RColor	Color;
};

class RDirectionalLightComponent : public RSceneComponentBase, public ILight, public IShadowCaster
{
	typedef RSceneComponentBase Base;
public:
	virtual ~RDirectionalLightComponent() override;

	static RDirectionalLightComponent* Create(RSceneObject* InOwner);

	virtual ELightType GetLightType() override;

	// Override IShadowCaster methods
	virtual bool CanCastShadow() override;
	virtual void PrepareShadowPass() override;
	virtual void PrepareRenderPass(SHADER_SCENE_BUFFER* SceneConstBuffer) override;
	virtual RFrustum GetFrustum() override;
	virtual ID3D11ShaderResourceView* GetRTDepthSRV() override;
	// End of IShadowCaster methods

	void SetParameters(const DirectionalLightParam& Parameters);

	const RVec3& GetDirection() const;
	const RColor& GetColor() const;

private:
	RDirectionalLightComponent(RSceneObject* InOwner);

	RVec3 m_Direction;
	RColor m_Color;

	RShadowMap	m_ShadowMap;
};

FORCEINLINE const RVec3& RDirectionalLightComponent::GetDirection() const
{
	return m_Direction;
}

FORCEINLINE const RColor& RDirectionalLightComponent::GetColor() const
{
	return m_Color;
}
