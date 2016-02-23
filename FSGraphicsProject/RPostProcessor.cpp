//=============================================================================
// RPostProcessor.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "RPostProcessor.h"

#include "PostProcessor_VS.csh"
#include "PostProcessor_GammaCorrection.csh"
#include "PostProcessor_ColorEdgeDetection.csh"

struct PP_QUAD
{
	RVec4 pos;
};

RPostProcessor::RPostProcessor()
{
	
}

void RPostProcessor::Initialize()
{
	// Create vertex shader for post processing
	RRenderer.D3DDevice()->CreateVertexShader(PostProcessor_VS, sizeof(PostProcessor_VS), 0, &m_PPVertexShader);
	RRenderer.D3DDevice()->CreatePixelShader(PostProcessor_GammaCorrection, sizeof(PostProcessor_GammaCorrection), 0, &m_PPPixelShader[PPE_GammaCorrection]);
	RRenderer.D3DDevice()->CreatePixelShader(PostProcessor_ColorEdgeDetection, sizeof(PostProcessor_ColorEdgeDetection), 0, &m_PPPixelShader[PPE_ColorEdgeDetection]);

	// Create vertex declaration for screen quad
	D3D11_INPUT_ELEMENT_DESC vsDesc[] = 
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	RRenderer.D3DDevice()->CreateInputLayout(vsDesc, 1, PostProcessor_VS, sizeof(PostProcessor_VS), &m_InputLayout);

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

	// Create render target
	D3D11_TEXTURE2D_DESC renderTargetTextureDesc;
	renderTargetTextureDesc.Width = RRenderer.GetClientWidth();
	renderTargetTextureDesc.Height = RRenderer.GetClientHeight();
	renderTargetTextureDesc.MipLevels = 1;
	renderTargetTextureDesc.ArraySize = 1;
	renderTargetTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
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

void RPostProcessor::Release()
{
	m_ScreenQuad.Release();
	SAFE_RELEASE(m_RTDepthBuffer);
	SAFE_RELEASE(m_RTDepthStencilView);
	SAFE_RELEASE(m_RTBuffer);
	SAFE_RELEASE(m_RTView);
	SAFE_RELEASE(m_RTSRV);
	SAFE_RELEASE(m_InputLayout);
	for (int i = 0; i < PPE_COUNT; i++)
	{
		SAFE_RELEASE(m_PPPixelShader[i]);
	}
	SAFE_RELEASE(m_PPVertexShader);
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