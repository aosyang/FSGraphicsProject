//=============================================================================
// RRenderSystem.h by Shiyang Ao, 2016 All Rights Reserved.
//
// Render system class
//=============================================================================
#ifndef _RRENDERSYSTEM_H
#define _RRENDERSYSTEM_H

#include "Core/RSingleton.h"

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

struct RShader;

enum BlendState
{
	Blend_Opaque,
	Blend_AlphaBlending,
	Blend_AlphaToCoverage,

	BlendStateCount,
};

enum SamplerState
{
	SamplerState_Texture,
	SamplerState_ShadowDepthComparison,

	SamplerStateCount,
};

class RRenderSystem : public RSingleton<RRenderSystem>
{
	friend class RSingleton<RRenderSystem>;
public:
	bool Initialize(HWND hWnd, int client_width, int client_height, bool enable4xMsaa, bool enableGammaCorrection=true);
	void Shutdown();

	float AspectRatio() const;
	void ResizeClient(int width, int height);

	void Clear(bool clearColor = true, const RColor& color = RColor(0.69f, 0.77f, 0.87f, 1.0f), bool clearDepth = true, float depth = 1.0f, bool clearStencil = true, UINT8 stencil = 0);
	void Present();

	ID3D11Device*			D3DDevice()						{ return m_pD3DDevice; }
	ID3D11DeviceContext*	D3DImmediateContext()			{ return m_pD3DImmediateContext; }

	int	GetClientWidth() const { return m_ClientWidth; }
	int	GetClientHeight() const { return m_ClientHeight; }
	const TCHAR* GetAdapterName() const { return m_AdapterName; }

	void SetRenderTarget(ID3D11RenderTargetView* renderTargetView = DefaultRenderTargetView, ID3D11DepthStencilView* depthStencilView = DefaultDepthStencilView);

	static ID3D11RenderTargetView* DefaultRenderTargetView;
	static ID3D11DepthStencilView* DefaultDepthStencilView;

	void SetBlendState(BlendState state);
	void SetSamplerState(int slot, SamplerState state);

	bool UsingGammaCorrection() const { return m_UseGammaCorrection; }
protected:
	RRenderSystem();
	~RRenderSystem();

	void CreateRenderTargetView();
	void CreateDepthStencilBufferAndView();

	int						m_ClientWidth, m_ClientHeight;
	bool					m_Enable4xMsaa;
	bool					m_UseGammaCorrection;
	UINT					m_4xMsaaQuality;
	TCHAR*					m_AdapterName;

	ID3D11Device*			m_pD3DDevice;
	ID3D11DeviceContext*	m_pD3DImmediateContext;
	IDXGISwapChain*			m_SwapChain;
	ID3D11RenderTargetView*	m_RenderTargetView;
	ID3D11Texture2D*		m_DepthStencilBuffer;
	ID3D11DepthStencilView*	m_DepthStencilView;

	ID3D11RenderTargetView*	m_CurrentRenderTargetView;
	ID3D11DepthStencilView*	m_CurrentDepthStencilView;

	ID3D11BlendState*		m_BlendState[BlendStateCount];
	BlendState				m_CurrBlendState;

	ID3D11SamplerState*		m_SamplerState[SamplerStateCount];
};

#define RRenderer RRenderSystem::Instance()

#endif
