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
	PointLight,
};

class ILight
{
public:
	virtual ~ILight() {}

	virtual ELightType GetLightType() const = 0;
	virtual RAabb GetEffectiveLightBounds() = 0;
	virtual void SetupConstantBuffer(int LightIndex) const = 0;
};

class RLight : public ILight
{
public:
	RLight()
		: LightColor(1.0f, 1.0f, 1.0f, 1.0f)
		, LightIntensity(1.0f)
	{}

	void SetLightColor(const RColor& NewColor)
	{
		LightColor = NewColor;
	}

	const RColor& GetLightColor() const
	{
		return LightColor;
	}

	void SetLightIntensity(float NewIntensity)
	{
		LightIntensity = NewIntensity;
	}

	float GetLightIntensity() const
	{
		return LightIntensity;
	}

private:
	RColor LightColor;
	float LightIntensity;
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
	virtual void PrepareRenderPass() = 0;

	// Get the number of depth passes
	virtual int GetDepthPassesNum() const = 0;

	virtual RVec4 GetShadowDepth(RCamera* ViewCamera) = 0;

	// Get the frustum of shadow pass
	virtual RFrustum GetFrustum(int PassIndex) = 0;

	// Get depth buffer as shader texture resource
	virtual ID3D11ShaderResourceView* GetRTDepthSRV(int PassIndex) = 0;
};
