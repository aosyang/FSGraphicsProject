//=============================================================================
// ILight.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

enum class ELightType : UINT8
{
	DirectionalLight,
};

class ILight
{
public:
	virtual ~ILight() {}

	virtual ELightType GetLightType() = 0;
};

// Shadow caster interface
class IShadowCaster
{
public:
	virtual ~IShadowCaster() {}

	virtual bool CanCastShadow() = 0;
	virtual void PrepareShadowPass() = 0;
	virtual void PrepareRenderPass(SHADER_SCENE_BUFFER* SceneConstBuffer) = 0;
	virtual RFrustum GetFrustum() = 0;
	virtual ID3D11ShaderResourceView* GetRTDepthSRV() = 0;
};
