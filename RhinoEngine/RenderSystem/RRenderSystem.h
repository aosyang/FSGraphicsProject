//=============================================================================
// RRenderSystem.h by Shiyang Ao, 2016 All Rights Reserved.
//
// Render system class
//=============================================================================
#pragma once

#include "Core/RSingleton.h"
#include "RRenderMeshComponent.h"
#include "BlendState.h"

#include <d3d11.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

struct RShader;
class RCamera;
class RScene;
class RRasterizerState;

class RLight;
class IShadowCaster;

struct ID3D11RenderTargetView;

enum SamplerState
{
	SamplerState_Texture,
	SamplerState_ShadowDepthComparison,

	SamplerStateCount,
};

struct RenderStats
{
	UINT DrawCalls;

	void Reset() { DrawCalls = 0; }
};

/// Renderable objects used as UI overlays
class IOverlayRenderable
{
public:
	virtual ~IOverlayRenderable() {}
	virtual void Render() = 0;
};

class RRenderSystem : public RSingleton<RRenderSystem>
{
	friend class RSingleton<RRenderSystem>;
public:
	bool Initialize(HWND hWnd, int client_width, int client_height, bool enable4xMsaa, bool enableGammaCorrection = true);
	void Shutdown();

	bool HasInitialized() const;

	float AspectRatio() const;
	void ResizeClient(int width, int height);

	void Clear(bool clearColor = true, const RColor& color = RColor(0.69f, 0.77f, 0.87f, 1.0f), bool clearDepth = true, float depth = 1.0f, bool clearStencil = true, UINT8 stencil = 0);
	void ClearRenderTarget(ID3D11RenderTargetView* rtv, const RColor& color = RColor(0.69f, 0.77f, 0.87f, 1.0f));

	// Present current frame. Called by the engine
	void Present(bool bWaitForVsync = true);

	ID3D11Device*			D3DDevice()						{ return m_pD3DDevice; }
	ID3D11DeviceContext*	D3DImmediateContext()			{ return m_pD3DImmediateContext; }

	int	GetClientWidth() const { return m_ClientWidth; }
	int	GetClientHeight() const { return m_ClientHeight; }
	const TCHAR* GetAdapterName() const { return m_AdapterName; }

	void SetRenderTargets(UINT numViews = 1, ID3D11RenderTargetView* const* renderTargetViews = &DefaultRenderTargetView, ID3D11DepthStencilView* depthStencilView = DefaultDepthStencilView);

	static ID3D11RenderTargetView* DefaultRenderTargetView;
	static ID3D11DepthStencilView* DefaultDepthStencilView;

	void SetBlendState(BlendState state);
	void SetSamplerState(int slot, SamplerState state);

	/// Bind a material to the pipeline for rendering
	void BindMaterial(RMaterial* Material, bool bSkinned = false, bool bInstancing = false);

	bool UsingGammaCorrection() const { return m_UseGammaCorrection; }

	void SetUsingDefferedShading(bool bUseDeferredShading)	{ m_bIsUsingDeferredShading = bUseDeferredShading; }
	bool IsUsingDeferredShading() const						{ return m_bIsUsingDeferredShading; }

	void SetVertexShader(ID3D11VertexShader* vertexShader);
	void SetPixelShader(ID3D11PixelShader* pixelShader);
	void SetGeometryShader(ID3D11GeometryShader* geometryShader);

	void RegisterRenderMeshComponent(RRenderMeshComponent* Component);
	void UnregisterRenderMeshComponent(RRenderMeshComponent* Component);

	void RegisterLight(RLight* Light);
	void UnregisterLight(RLight* Light);
	const std::vector<RLight*>& GetRegisteredLights() const;

	void RegisterShadowCaster(IShadowCaster* ShadowCaster);
	void UnregisterShadowCaster(IShadowCaster* ShadowCaster);

	void RegisterOverlayRenderable(IOverlayRenderable* OverlayRenderable);
	void UnregisterOverlayRenderable(IOverlayRenderable* OverlayRenderable);

	void SetActiveScene(RScene* Scene);
	RScene* GetActiveScene() const;

	RenderStats	Stats;

	/// Render current frame
	void RenderFrame();

protected:
	RRenderSystem();
	~RRenderSystem();

	void CreateRenderTargetView();
	void CreateDepthStencilBufferAndView();

	ID3D11BlendState* CreateD3DBlendState(const D3D11_BLEND_DESC* Desc, char* DebugObjectName = nullptr);

	void UnbindShadowMapShaderResourceViews();

	bool					bInitialized;
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

	UINT					m_RenderTargetViewNum;
	ID3D11RenderTargetView*	m_CurrentRenderTargetViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
	ID3D11DepthStencilView*	m_CurrentDepthStencilView;

	std::unique_ptr<RRasterizerState>	RasterizerState;

	ID3D11BlendState*		m_BlendState[(int)BlendState::Count];
	BlendState				m_CurrBlendState;

	ID3D11SamplerState*		m_SamplerState[SamplerStateCount];

	bool					m_bIsUsingDeferredShading;

	std::vector<RRenderMeshComponent*>	m_RegisteredRenderMeshComponents;
	std::vector<RLight*>				m_RegisteredLights;
	std::vector<IShadowCaster*>			m_RegisteredShadowCasters;
	std::vector<IOverlayRenderable*>	m_OverlayRenderables;

	RScene*					m_ActiveScene;
};

#define GRenderer RRenderSystem::Instance()

FORCEINLINE const std::vector<RLight*>& RRenderSystem::GetRegisteredLights() const
{
	return m_RegisteredLights;
}

