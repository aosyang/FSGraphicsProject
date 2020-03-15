//=============================================================================
// RDirectionalLightComponent.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "Scene/RSceneComponent.h"
#include "ILight.h"
#include "RShadowMap.h"

#define MAX_CASCADED_SHADOW_SPLITS_NUM 3

class RDirectionalLightComponent : public RSceneComponent, public RLight, public IShadowCaster
{
	DECLARE_SCENE_COMPONENT(RDirectionalLightComponent, RSceneComponent);
public:
	virtual ~RDirectionalLightComponent() override;

	virtual ELightType GetLightType() const override;
	virtual RAabb GetEffectiveLightBounds() override;
	virtual void SetupConstantBuffer(int LightIndex) const override;

	// Override IShadowCaster methods
	virtual bool CanCastShadow() override;
	virtual void PrepareDepthPass(int PassIndex, const RenderViewInfo& View) override;
	virtual void PrepareRenderPass() override;
	virtual int GetDepthPassesNum() const override;
	virtual RVec4 GetShadowDepth(RCamera* ViewCamera) override;
	virtual RFrustum GetFrustum(int PassIndex) override;
	virtual ID3D11ShaderResourceView* GetRTDepthSRV(int PassIndex) override;
	// End of IShadowCaster methods

	virtual void LoadComponentFromXmlElement(tinyxml2::XMLElement* ComponentElem) override;
	virtual void SaveComponentToXmlElement(tinyxml2::XMLElement* ComponentElem) const override;

	void SetLightDirection(const RVec3& Direction);

	// Returns normalized light direction
	RVec3 GetLightDirection() const;

protected:
	// Calculate the bounding sphere from a frustum
	RSphere CalculateBoundingSphereFromFrustum(const RFrustum& frustum, float start, float end);

private:
	RDirectionalLightComponent(RSceneObject* InOwner);

	RShadowMap	m_ShadowMap[MAX_CASCADED_SHADOW_SPLITS_NUM];

	static const RMatrix4 ShadowBiasTransform;
};
