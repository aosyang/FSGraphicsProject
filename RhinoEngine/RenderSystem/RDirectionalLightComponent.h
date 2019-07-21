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

#define MAX_CASCADED_SHADOW_SPLITS_NUM 3

class RDirectionalLightComponent : public RSceneComponentBase, public ILight, public IShadowCaster
{
	DECLARE_SCENE_COMPONENT(RDirectionalLightComponent, RSceneComponentBase);
public:
	virtual ~RDirectionalLightComponent() override;

	virtual ELightType GetLightType() override;

	// Override IShadowCaster methods
	virtual bool CanCastShadow() override;
	virtual void PrepareDepthPass(int PassIndex, const RenderViewInfo& View) override;
	virtual void PrepareRenderPass() override;
	virtual int GetDepthPassesNum() const override;
	virtual RVec4 GetShadowDepth(RCamera* ViewCamera) override;
	virtual RFrustum GetFrustum(int PassIndex) override;
	virtual ID3D11ShaderResourceView* GetRTDepthSRV(int PassIndex) override;
	// End of IShadowCaster methods

	void SetParameters(const DirectionalLightParam& Parameters);

	// Returns normalized light direction
	const RVec3& GetLightDirection() const;
	
	// Returns color of light
	const RColor& GetLightColor() const;

protected:
	// Calculate the bounding sphere from a frustum
	RSphere CalculateBoundingSphereFromFrustum(const RFrustum& frustum, float start, float end);

private:
	RDirectionalLightComponent(RSceneObject* InOwner);

	// Normalized light direction
	RVec3 m_LightDirection;

	// Color of light
	RColor m_LightColor;

	RShadowMap	m_ShadowMap[MAX_CASCADED_SHADOW_SPLITS_NUM];

	static const RMatrix4 ShadowBiasTransform;
};

FORCEINLINE const RVec3& RDirectionalLightComponent::GetLightDirection() const
{
	return m_LightDirection;
}

FORCEINLINE const RColor& RDirectionalLightComponent::GetLightColor() const
{
	return m_LightColor;
}
