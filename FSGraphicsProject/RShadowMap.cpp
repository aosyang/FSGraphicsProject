//=============================================================================
// RShadowMap.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#include "RShadowMap.h"

RShadowMap::RShadowMap()
	: m_RenderTargetBuffer(nullptr), m_RenderTargetView(nullptr), m_RenderTargetSRV(nullptr),
	  m_DepthBuffer(nullptr), m_DepthView(nullptr)
{
	m_ViewMatrix = RMatrix4::IDENTITY;
	m_ProjMatrix = RMatrix4::IDENTITY;
}

RShadowMap::~RShadowMap()
{
	SAFE_RELEASE(m_RenderTargetDepthSRV);
	SAFE_RELEASE(m_DepthBuffer);
	SAFE_RELEASE(m_DepthView);
	SAFE_RELEASE(m_RenderTargetSRV);
	SAFE_RELEASE(m_RenderTargetView);
	SAFE_RELEASE(m_RenderTargetBuffer);
}

void RShadowMap::Initialize(int width, int height)
{
	m_Width = width;
	m_Height = height;

	D3D11_TEXTURE2D_DESC renderTextureDesc;
	renderTextureDesc.Width = width;
	renderTextureDesc.Height = height;
	renderTextureDesc.MipLevels = 1;
	renderTextureDesc.ArraySize = 1;
	renderTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// No MSAA on depth buffer
	renderTextureDesc.SampleDesc.Count = 1;
	renderTextureDesc.SampleDesc.Quality = 0;

	renderTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	renderTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	renderTextureDesc.CPUAccessFlags = 0;
	renderTextureDesc.MiscFlags = 0;

	RRenderer.D3DDevice()->CreateTexture2D(&renderTextureDesc, 0, &m_RenderTargetBuffer);
	RRenderer.D3DDevice()->CreateRenderTargetView(m_RenderTargetBuffer, NULL, &m_RenderTargetView);

	// Create depth buffer for render target
	renderTextureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	renderTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	RRenderer.D3DDevice()->CreateTexture2D(&renderTextureDesc, 0, &m_DepthBuffer);

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	depthStencilViewDesc.Flags = 0;
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	RRenderer.D3DDevice()->CreateDepthStencilView(m_DepthBuffer, &depthStencilViewDesc, &m_DepthView);

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	RRenderer.D3DDevice()->CreateShaderResourceView(m_DepthBuffer, &shaderResourceViewDesc, &m_RenderTargetDepthSRV);

	shaderResourceViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	RRenderer.D3DDevice()->CreateShaderResourceView(m_RenderTargetBuffer, &shaderResourceViewDesc, &m_RenderTargetSRV);
}

void RShadowMap::SetOrthogonalProjection(float viewWidth, float viewHeight, float nearZ, float farZ)
{
	m_ProjMatrix = RMatrix4::CreateOrthographicProjectionLH(viewWidth, viewHeight, nearZ, farZ);
}

void RShadowMap::SetupRenderTarget()
{
	RRenderer.SetRenderTarget(GetRenderTargetView(), GetDepthView());

	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = static_cast<float>(m_Width);
	vp.Height = static_cast<float>(m_Height);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	RRenderer.D3DImmediateContext()->RSSetViewports(1, &vp);
}