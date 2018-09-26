//=============================================================================
// ILight.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

class RCamera;

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

	// Set up everything for shadow depth pass rendering
	virtual void PrepareDepthPass(int PassIndex, const RenderViewInfo& View) = 0;

	// Set up everything for view pass rendering
	virtual void PrepareRenderPass(SHADER_SCENE_BUFFER* SceneConstBuffer) = 0;

	// Get the number of depth passes
	virtual int GetDepthPassesNum() const = 0;

	virtual RVec4 GetShadowDepth(RCamera* ViewCamera) = 0;

	// Get the frustum of shadow pass
	virtual RFrustum GetFrustum(int PassIndex) = 0;

	// Get depth buffer as shader texture resource
	virtual ID3D11ShaderResourceView* GetRTDepthSRV(int PassIndex) = 0;
};
