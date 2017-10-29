//=============================================================================
// RRenderSystem.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#include "Rhino.h"

#include "RRenderSystem.h"
#include <comdef.h>

ID3D11DepthStencilView* RRenderSystem::DefaultDepthStencilView = nullptr;
ID3D11RenderTargetView* RRenderSystem::DefaultRenderTargetView = nullptr;

RRenderSystem::RRenderSystem()
	: m_AdapterName(nullptr), m_RenderTargetViewNum(0), m_bIsUsingDeferredShading(false)
{
}


RRenderSystem::~RRenderSystem()
{
	// Should have unregistered all components by now
	assert(m_RegisteredRenderMeshComponents.size() == 0);
}

bool RRenderSystem::Initialize(HWND hWnd, int client_width, int client_height, bool enable4xMsaa, bool enableGammaCorrection)
{
	m_ClientWidth = client_width;
	m_ClientHeight = client_height;
	m_Enable4xMsaa = enable4xMsaa;
	m_UseGammaCorrection = enableGammaCorrection;

	UINT createDeviceFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// Enumerate adaptors
	IDXGIFactory1* dxgiFactory = 0;
	HR(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&dxgiFactory));

	IDXGIAdapter* dxgiAdapter = 0;
	std::vector<IDXGIAdapter*> vAdapters;

	// Select best adapter with most VRAM
	UINT bestAdapterIndex = 0;
	size_t bestAdapterMem = 0;

	for (UINT i = 0;
		dxgiFactory->EnumAdapters(i, &dxgiAdapter) != DXGI_ERROR_NOT_FOUND;
		++i)
	{
		vAdapters.push_back(dxgiAdapter);
		DXGI_ADAPTER_DESC desc;
		dxgiAdapter->GetDesc(&desc);

		if (desc.DedicatedVideoMemory > bestAdapterMem)
		{
			bestAdapterIndex = i;
			bestAdapterMem = desc.DedicatedVideoMemory;
		}
	}

	// Store adapter's name
	DXGI_ADAPTER_DESC desc;
	vAdapters[bestAdapterIndex]->GetDesc(&desc);
	size_t desc_len = _tcslen(desc.Description);
	m_AdapterName = new TCHAR[desc_len + 1];
	_tcscpy_s(m_AdapterName, desc_len + 1, desc.Description);

	// Create d3d11 device
	D3D_FEATURE_LEVEL featureLevel;
	HRESULT hr = D3D11CreateDevice(
		vAdapters[bestAdapterIndex],
		D3D_DRIVER_TYPE_UNKNOWN,
		0,
		createDeviceFlags,
		0, 0,
		D3D11_SDK_VERSION,
		&m_pD3DDevice,
		&featureLevel,
		&m_pD3DImmediateContext);

	if (FAILED(hr))
	{
		TCHAR* szErrMsg;

		if (FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&szErrMsg, 0, NULL) != 0)
		{
			TCHAR buffer[1024];
			_snwprintf_s(buffer, sizeof(buffer) / sizeof(TCHAR), 1024, L"Failed to create D3D11 device: %s", szErrMsg);

			MessageBox(0, buffer, 0, MB_ICONERROR);

			LocalFree(szErrMsg);
		}
		else
		{
			MessageBox(0, L"Failed to create D3D11 device: Unknown error.", 0, MB_ICONERROR);
		}
		return false;
	}

	if (featureLevel != D3D_FEATURE_LEVEL_11_0)
	{
		MessageBox(0, L"Direct3D Feature Level 11 unsupported.", 0, MB_ICONERROR);
		return false;
	}

	DXGI_FORMAT backbuffer_format = m_UseGammaCorrection ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;

	// Check 4X MSAA quality support
	HR(m_pD3DDevice->CheckMultisampleQualityLevels(
		backbuffer_format, 4, &m_4xMsaaQuality));
	assert(m_4xMsaaQuality > 0);

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = client_width;
	sd.BufferDesc.Height = client_height;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = backbuffer_format;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	if (enable4xMsaa)
	{
		sd.SampleDesc.Count = 4;
		sd.SampleDesc.Quality = m_4xMsaaQuality - 1;
	}
	else
	{
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
	}

	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = hWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	// Create swap chain
	HR(dxgiFactory->CreateSwapChain(m_pD3DDevice, &sd, &m_SwapChain));

	// Release COM objects
	dxgiFactory->Release();

	// Create render target view
	CreateRenderTargetView();

	// Create depth/stencil buffer and view
	CreateDepthStencilBufferAndView();

	// Bind views to the output merger stage
	m_pD3DImmediateContext->OMSetRenderTargets(1, &m_RenderTargetView, m_DepthStencilView);

	// Setup viewport
	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = static_cast<float>(client_width);
	vp.Height = static_cast<float>(client_height);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	m_pD3DImmediateContext->RSSetViewports(1, &vp);

	RVertexDeclaration::Instance().Initialize();

	// Create blend states
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	GRenderer.D3DDevice()->CreateBlendState(&blendDesc, &m_BlendState[Blend_Opaque]);

#if _DEBUG
	{
		char blendStateName[] = "Opaque";
		m_BlendState[Blend_Opaque]->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(blendStateName) - 1, blendStateName);
	}
#endif

	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	GRenderer.D3DDevice()->CreateBlendState(&blendDesc, &m_BlendState[Blend_AlphaBlending]);

#if _DEBUG
	{
		char blendStateName[] = "Alpha Blending";
		m_BlendState[Blend_AlphaBlending]->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(blendStateName) - 1, blendStateName);
	}
#endif

	blendDesc.AlphaToCoverageEnable = true;
	GRenderer.D3DDevice()->CreateBlendState(&blendDesc, &m_BlendState[Blend_AlphaToCoverage]);

#if _DEBUG
	{
		char blendStateName[] = "Alpha To Coverage Blending";
		m_BlendState[Blend_AlphaToCoverage]->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(blendStateName) - 1, blendStateName);
	}
#endif

	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.AlphaToCoverageEnable = false;
	GRenderer.D3DDevice()->CreateBlendState(&blendDesc, &m_BlendState[Blend_Additive]);
	
#if _DEBUG
	{
		char blendStateName[] = "Additive Blending";
		m_BlendState[Blend_Additive]->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(blendStateName) - 1, blendStateName);
	}
#endif


	m_CurrBlendState = Blend_Opaque;

	// Create sampler states
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	GRenderer.D3DDevice()->CreateSamplerState(&samplerDesc, &m_SamplerState[SamplerState_Texture]);

	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	GRenderer.D3DDevice()->CreateSamplerState(&samplerDesc, &m_SamplerState[SamplerState_ShadowDepthComparison]);

	RConstantBuffers::Initialize();

	return true;
}

void RRenderSystem::Shutdown()
{
	RConstantBuffers::Shutdown();

	for (int i = 0; i < SamplerStateCount; i++)
	{
		SAFE_RELEASE(m_SamplerState[i]);
	}

	for (int i = 0; i < BlendStateCount; i++)
	{
		SAFE_RELEASE(m_BlendState[i]);
	}

	RVertexDeclaration::Instance().Release();

	m_DepthStencilView->Release();
	m_DepthStencilBuffer->Release();
	m_RenderTargetView->Release();
	m_SwapChain->Release();
	m_pD3DImmediateContext->Release();
	m_pD3DDevice->Release();

	delete[] m_AdapterName;
}

float RRenderSystem::AspectRatio() const
{
	return (float)m_ClientWidth / m_ClientHeight;
}

void RRenderSystem::ResizeClient(int width, int height)
{
	if (width && height)
	{
		m_ClientWidth = width;
		m_ClientHeight = height;

		if (m_SwapChain)
		{
			m_pD3DImmediateContext->OMSetRenderTargets(0, 0, 0);
			SAFE_RELEASE(m_DepthStencilBuffer);
			SAFE_RELEASE(m_DepthStencilView);
			SAFE_RELEASE(m_RenderTargetView);

			HR(m_SwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0));

			CreateRenderTargetView();

			CreateDepthStencilBufferAndView();

			// Bind views to the output merger stage
			m_pD3DImmediateContext->OMSetRenderTargets(1, &m_RenderTargetView, m_DepthStencilView);

			// Setup viewport
			D3D11_VIEWPORT vp;
			vp.TopLeftX = 0.0f;
			vp.TopLeftY = 0.0f;
			vp.Width = static_cast<float>(m_ClientWidth);
			vp.Height = static_cast<float>(m_ClientHeight);
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;

			m_pD3DImmediateContext->RSSetViewports(1, &vp);
		}
	}
}

void RRenderSystem::Clear(bool clearColor, const RColor& color, bool clearDepth, float depth, bool clearStencil, UINT8 stencil)
{
	assert(m_pD3DImmediateContext);

	if (m_RenderTargetViewNum && m_CurrentRenderTargetViews && clearColor)
	{
		for (UINT i = 0; i < m_RenderTargetViewNum; i++)
		{
			m_pD3DImmediateContext->ClearRenderTargetView(m_CurrentRenderTargetViews[i],
				reinterpret_cast<const float*>(&color));
		}
	}

	if (m_CurrentDepthStencilView)
	{
		UINT clearFlag = 0;

		if (clearDepth) clearFlag |= D3D11_CLEAR_DEPTH;
		if (clearStencil) clearFlag |= D3D11_CLEAR_STENCIL;

		m_pD3DImmediateContext->ClearDepthStencilView(m_CurrentDepthStencilView,
			clearFlag, depth, stencil);
	}
}

void RRenderSystem::ClearRenderTarget(ID3D11RenderTargetView* rtv, const RColor& color)
{
	m_pD3DImmediateContext->ClearRenderTargetView(rtv, reinterpret_cast<const float*>(&color));
}

void RRenderSystem::Present()
{
	assert(m_SwapChain);

	HR(m_SwapChain->Present(1, 0));
}

void RRenderSystem::SetRenderTargets(UINT numViews, ID3D11RenderTargetView* const* renderTargetView, ID3D11DepthStencilView* depthStencilView)
{
	m_RenderTargetViewNum = numViews;
	for (UINT i = 0; i < numViews; i++)
	{
		m_CurrentRenderTargetViews[i] = renderTargetView[i];
	}
	m_CurrentDepthStencilView = depthStencilView;

	if (renderTargetView == nullptr)
	{
		ID3D11RenderTargetView* nullRTV[] = { nullptr };
		m_pD3DImmediateContext->OMSetRenderTargets(1, nullRTV, depthStencilView);
	}
	else
		m_pD3DImmediateContext->OMSetRenderTargets(numViews, renderTargetView, depthStencilView);
}

void RRenderSystem::SetBlendState(BlendState state)
{
	if (m_CurrBlendState != state)
	{
		float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		GRenderer.D3DImmediateContext()->OMSetBlendState(m_BlendState[state], blendFactor, 0xFFFFFFFF);
		m_CurrBlendState = state;
	}
}

void RRenderSystem::SetSamplerState(int slot, SamplerState state)
{
	GRenderer.D3DImmediateContext()->PSSetSamplers(slot, 1, &m_SamplerState[state]);
}

void RRenderSystem::SetVertexShader(ID3D11VertexShader* vertexShader)
{
	static ID3D11VertexShader* currentVertexShader = nullptr;
	if (currentVertexShader != vertexShader)
	{
		GRenderer.D3DImmediateContext()->VSSetShader(vertexShader, nullptr, 0);
		currentVertexShader = vertexShader;
	}
}

void RRenderSystem::SetPixelShader(ID3D11PixelShader* pixelShader)
{
	static ID3D11PixelShader* currentPixelShader = nullptr;
	if (currentPixelShader != pixelShader)
	{
		GRenderer.D3DImmediateContext()->PSSetShader(pixelShader, nullptr, 0);
		currentPixelShader = pixelShader;
	}
}

void RRenderSystem::SetGeometryShader(ID3D11GeometryShader* geometryShader)
{
	static ID3D11GeometryShader* currentGeometryShader = nullptr;
	if (currentGeometryShader != geometryShader)
	{
		GRenderer.D3DImmediateContext()->GSSetShader(geometryShader, nullptr, 0);
		currentGeometryShader = geometryShader;
	}
}

void RRenderSystem::RegisterRenderMeshComponent(RRenderMeshComponent* Component)
{
	auto Iter = find(m_RegisteredRenderMeshComponents.begin(), m_RegisteredRenderMeshComponents.end(), Component);

	// Component must not be registered already
	assert(Iter == m_RegisteredRenderMeshComponents.end());

	m_RegisteredRenderMeshComponents.push_back(Component);
}

void RRenderSystem::UnregisterRenderMeshComponent(RRenderMeshComponent* Component)
{
	auto Iter = find(m_RegisteredRenderMeshComponents.begin(), m_RegisteredRenderMeshComponents.end(), Component);

	// Shouldn't unregister a component which is not registered
	assert(Iter != m_RegisteredRenderMeshComponents.end());

	m_RegisteredRenderMeshComponents.erase(Iter);
}

void RRenderSystem::RenderFrame()
{
	Clear();

	SHADER_GLOBAL_BUFFER cbGlobal;
	ZeroMemory(&cbGlobal, sizeof(cbGlobal));

	cbGlobal.UseGammaCorrection = UsingGammaCorrection();

	RConstantBuffers::cbGlobal.UpdateBufferData(&cbGlobal);
	RConstantBuffers::cbGlobal.BindBuffer();

	// Shadow map sampler state
	SetSamplerState(2, SamplerState_ShadowDepthComparison);

	for (auto MeshComponent : m_RegisteredRenderMeshComponents)
	{
		MeshComponent->Render();
	}

	GDebugRenderer.Render();
	GDebugRenderer.Reset();

	Present();
}

void RRenderSystem::CreateRenderTargetView()
{
	ID3D11Texture2D* backBuffer;
	m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
		reinterpret_cast<void**>(&backBuffer));
	m_pD3DDevice->CreateRenderTargetView(backBuffer, 0, &m_RenderTargetView);

	DefaultRenderTargetView = m_RenderTargetView;
	m_CurrentRenderTargetViews[0] = m_RenderTargetView;
	m_RenderTargetViewNum = 1;

	backBuffer->Release();
}

void RRenderSystem::CreateDepthStencilBufferAndView()
{
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = m_ClientWidth;
	depthStencilDesc.Height = m_ClientHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	if (m_Enable4xMsaa)
	{
		depthStencilDesc.SampleDesc.Count = 4;
		depthStencilDesc.SampleDesc.Quality = m_4xMsaaQuality - 1;
	}
	else
	{
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}

	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	HR(m_pD3DDevice->CreateTexture2D(&depthStencilDesc, 0, &m_DepthStencilBuffer));
	HR(m_pD3DDevice->CreateDepthStencilView(m_DepthStencilBuffer, 0, &m_DepthStencilView));

	DefaultDepthStencilView = m_DepthStencilView;
	m_CurrentDepthStencilView = m_DepthStencilView;
}
