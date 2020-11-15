//=============================================================================
// DeferredShadingApp.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Rhino.h"

enum EPostProcessingEffect
{
	PPE_DeferredComposition,
	PPE_DeferredPointLightPass,
	PPE_ScreenSpaceRayTracing,

	PPE_COUNT,
};

struct DeferredRenderBuffer
{
	ID3D11Texture2D*			Buffer;
	ID3D11RenderTargetView*		View;
	ID3D11ShaderResourceView*	SRV;

	DeferredRenderBuffer()
		:Buffer(nullptr), View(nullptr), SRV(nullptr)
	{
	}

	void Release()
	{
		SAFE_RELEASE(Buffer);
		SAFE_RELEASE(View);
		SAFE_RELEASE(SRV);
	}
};

struct DepthStencilBuffer
{
	ID3D11Texture2D*			Buffer;
	ID3D11DepthStencilView*		View;

	DepthStencilBuffer()
		: Buffer(nullptr), View(nullptr)
	{
	}

	void Release()
	{
		SAFE_RELEASE(Buffer);
		SAFE_RELEASE(View);
	}
};

struct CubeDepthBuffer
{
	ID3D11Texture2D*			Buffer;
	ID3D11RenderTargetView*		View[6];
	ID3D11ShaderResourceView*	SRV;
	ID3D11Texture2D*			DepthBuffer;
	ID3D11DepthStencilView*		DepthView[6];
	ID3D11ShaderResourceView*	DepthSRV;

	CubeDepthBuffer()
		: Buffer(nullptr), SRV(nullptr), DepthBuffer(nullptr), DepthSRV(nullptr)
	{
		for (int i = 0; i < 6; i++)
		{
			View[i] = nullptr;
			DepthView[i] = nullptr;
		}
	}

	void Release()
	{
		SAFE_RELEASE(Buffer);
		SAFE_RELEASE(SRV);
		for (int i = 0; i < 6; i++)
		{
			SAFE_RELEASE(View[i]);
			SAFE_RELEASE(DepthView[i]);
		}
		SAFE_RELEASE(DepthBuffer);
		SAFE_RELEASE(DepthSRV);
	}
};

enum EDeferredBuffer
{
	DB_Color,
	DB_Position,
	DB_WorldSpaceNormal,
	DB_ViewSpaceNormal,

	DeferredBuffer_Count
};

struct PointLight
{
	RVec3 pos;
	float r;
	RColor color;
	RVec3 sin_factor;
	RVec3 sin_offset;
	bool castShadow;
};

#define MAX_LIGHT_COUNT 500

enum ERasterizerState
{
	RS_Default,
	RS_Scissor,

	RasterizerState_Count,
};

class DeferredShadingApp : public IApp
{
public:
	DeferredShadingApp();
	~DeferredShadingApp();

	virtual bool Initialize() override;
	virtual void UpdateScene(const RTimer& timer) override;

	virtual bool UsingCustomRenderPipeline() override { return true; }
	virtual void RenderScene() override;

	virtual void OnResize(int width, int height) override;
	virtual TCHAR* WindowTitle() override { return L"Deferred Shading Demo"; }

private:
	void CreateGBuffers();

	DeferredRenderBuffer CreateRenderTarget(DXGI_FORMAT format, const char* debugResourceName=nullptr);
	DepthStencilBuffer CreateDepthStencilBuffer();
	CubeDepthBuffer CreateCubeDepthBuffer();

	void RenderPointLightCubemapDepth(const RVec3& position, float radius);

	RCamera*					m_Camera;
	float						m_CamPitch, m_CamYaw;

	RPostProcessingEffect*		m_PostProcessingEffects[PPE_COUNT];
	ID3D11RasterizerState*		m_RasterizerStates[RasterizerState_Count];

	RShaderConstantBuffer<SHADER_DEFERRED_POINT_LIGHT_BUFFER, CBST_PS, 4>
								m_cbDeferredPointLight;
	RShaderConstantBuffer<SHADER_SSR_BUFFER, CBST_PS, 5>
								m_cbSSR;
	SHADER_SSR_BUFFER			cbSSR;

	DeferredRenderBuffer		m_DeferredBuffers[DeferredBuffer_Count];
	DeferredRenderBuffer		m_ScenePassBuffer;
	DepthStencilBuffer			m_DepthBuffer;

	PointLight					m_PointLights[MAX_LIGHT_COUNT];
	float						m_TotalTime;
	RDebugMenu					m_DebugMenu;

	bool						m_EnableDeferredShading;
	bool						m_EnableSSR;
	bool						m_EnablePointLightShadow;

	int							m_LightCount;
	float						m_LightRadius;
	bool						m_RenderLightPos;
	float						m_AmbientIntensity;
	RTexture*					m_EnvCube;
	CubeDepthBuffer				m_CubeDepthBuffer;
	RSkybox						m_Skybox;
};

