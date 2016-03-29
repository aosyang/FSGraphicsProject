//=============================================================================
// RPostProcessor.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "RPostProcessor.h"

#include "PostProcessor_GammaCorrection.csh"
#include "PostProcessor_ColorEdgeDetection.csh"

struct PP_QUAD
{
	RVec4 pos;
};

RPostProcessor::RPostProcessor()
	: m_RTBuffer(nullptr), m_RTDepthBuffer(nullptr),
	  m_RTView(nullptr), m_RTSRV(nullptr),
	  m_RTDepthStencilView(nullptr)
{
	
}

void RPostProcessor::Initialize()
{
	// Create vertex shader for post processing
	m_PPVertexShader = RShaderManager::Instance().GetShaderResource("PostProcessor")->VertexShader;
	RRenderer.D3DDevice()->CreatePixelShader(PostProcessor_GammaCorrection, sizeof(PostProcessor_GammaCorrection), 0, &m_PPPixelShader[PPE_GammaCorrection]);
	RRenderer.D3DDevice()->CreatePixelShader(PostProcessor_ColorEdgeDetection, sizeof(PostProcessor_ColorEdgeDetection), 0, &m_PPPixelShader[PPE_ColorEdgeDetection]);

	// Find vertex declaration for screen quad
	m_InputLayout = RRenderer.GetInputLayout(RVertex::SKYBOX_VERTEX::GetTypeName());

	// Create vertex buffer for screen quad

	PP_QUAD quad[] =
	{
		RVec4(-1.0f, -1.0f, 1.0f),
		RVec4(-1.0f,  1.0f, 1.0f),
		RVec4( 1.0f,  1.0f, 1.0f),

		RVec4(-1.0f, -1.0f, 1.0f),
		RVec4( 1.0f,  1.0f, 1.0f),
		RVec4( 1.0f, -1.0f, 1.0f),
	};
	m_ScreenQuad.CreateVertexBuffer(quad, sizeof(PP_QUAD), 6);

	CreateRenderTargetResources();
}

void RPostProcessor::Release()
{
	m_ScreenQuad.Release();
	SAFE_RELEASE(m_RTDepthBuffer);
	SAFE_RELEASE(m_RTDepthStencilView);
	SAFE_RELEASE(m_RTBuffer);
	SAFE_RELEASE(m_RTView);
	SAFE_RELEASE(m_RTSRV);
	for (int i = 0; i < PPE_COUNT; i++)
	{
		SAFE_RELEASE(m_PPPixelShader[i]);
	}
}

void RPostProcessor::RecreateLostResources()
{
	if (m_RTView)
	{
		SAFE_RELEASE(m_RTDepthBuffer);
		SAFE_RELEASE(m_RTDepthStencilView);
		SAFE_RELEASE(m_RTBuffer);
		SAFE_RELEASE(m_RTView);
		SAFE_RELEASE(m_RTSRV);

		CreateRenderTargetResources();
	}
}

void RPostProcessor::SetupRenderTarget()
{
	RRenderer.SetRenderTarget(m_RTView, m_RTDepthStencilView);
	
	D3D11_VIEWPORT vp = { 0.0f, 0.0f, (FLOAT)RRenderer.GetClientWidth(), (FLOAT)RRenderer.GetClientHeight(), 0.0f, 1.0f };
	RRenderer.D3DImmediateContext()->RSSetViewports(1, &vp);
}

void RPostProcessor::Draw(PostProcessingEffect effect)
{
	RRenderer.D3DImmediateContext()->PSSetShader(m_PPPixelShader[effect], nullptr, 0);
	RRenderer.D3DImmediateContext()->VSSetShader(m_PPVertexShader, nullptr, 0);
	RRenderer.D3DImmediateContext()->GSSetShader(nullptr, nullptr, 0);
	RRenderer.D3DImmediateContext()->PSSetShaderResources(0, 1, &m_RTSRV);

	RRenderer.D3DImmediateContext()->IASetInputLayout(m_InputLayout);
	m_ScreenQuad.Draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void RPostProcessor::CreateRenderTargetResources()
{
	// Create render target
	D3D11_TEXTURE2D_DESC renderTargetTextureDesc;
	renderTargetTextureDesc.Width = RRenderer.GetClientWidth();
	renderTargetTextureDesc.Height = RRenderer.GetClientHeight();
	renderTargetTextureDesc.MipLevels = 1;
	renderTargetTextureDesc.ArraySize = 1;
	renderTargetTextureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	renderTargetTextureDesc.SampleDesc.Count = 1;
	renderTargetTextureDesc.SampleDesc.Quality = 0;
	renderTargetTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	renderTargetTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	renderTargetTextureDesc.CPUAccessFlags = 0;
	renderTargetTextureDesc.MiscFlags = 0;

	RRenderer.D3DDevice()->CreateTexture2D(&renderTargetTextureDesc, 0, &m_RTBuffer);
	RRenderer.D3DDevice()->CreateRenderTargetView(m_RTBuffer, 0, &m_RTView);

	D3D11_SHADER_RESOURCE_VIEW_DESC rtsrvDesc;
	rtsrvDesc.Format = renderTargetTextureDesc.Format;
	rtsrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	rtsrvDesc.Texture2D.MostDetailedMip = 0;
	rtsrvDesc.Texture2D.MipLevels = 1;

	RRenderer.D3DDevice()->CreateShaderResourceView(m_RTBuffer, &rtsrvDesc, &m_RTSRV);

	renderTargetTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	renderTargetTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	RRenderer.D3DDevice()->CreateTexture2D(&renderTargetTextureDesc, 0, &m_RTDepthBuffer);
	RRenderer.D3DDevice()->CreateDepthStencilView(m_RTDepthBuffer, 0, &m_RTDepthStencilView);
}
