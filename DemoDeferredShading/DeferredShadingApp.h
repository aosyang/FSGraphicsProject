//=============================================================================
// DeferredShadingApp.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#ifndef _DEFERREDSHADINGAPP_H
#define _DEFERREDSHADINGAPP_H

#include "Rhino.h"
#include "RPostProcessor.h"

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

enum EDeferredBuffer
{
	DB_Color,
	DB_Position,
	DB_Normal,

	DeferredBuffer_Count
};

struct PointLight
{
	RVec3 pos;
	float r;
	RColor color;
	RVec3 sin_factor;
	RVec3 sin_offset;
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

	bool Initialize();
	void UpdateScene(const RTimer& timer);
	void RenderScene();

	void OnResize(int width, int height);
	TCHAR* WindowTitle() { return L"Deferred Shading Demo"; }

private:
	DeferredRenderBuffer CreateRenderTarget(DXGI_FORMAT format);
	DepthStencilBuffer CreateDepthStencilBuffer();

	RCamera						m_Camera;
	float						m_CamPitch, m_CamYaw;

	RScene						m_Scene;

	RPostProcessor				m_PostProcessor;
	ID3D11RasterizerState*		m_RasterizerStates[RasterizerState_Count];

	RShaderConstantBuffer<SHADER_DEFERRED_POINT_LIGHT_BUFFER, CBST_PS, 2>
								m_cbDeferredPointLight;
	DeferredRenderBuffer		m_DeferredBuffers[DeferredBuffer_Count];
	DepthStencilBuffer			m_DepthBuffer;

	PointLight					m_PointLights[MAX_LIGHT_COUNT];
	float						m_TotalTime;
	RDebugRenderer				m_DebugRenderer;
};

#endif
