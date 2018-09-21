//=============================================================================
// RPostProcessor.cpp by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RPostProcessorManager.h"

#include "Core/RVector.h"
#include "RRenderSystem.h"
#include "RShaderManager.h"
#include "RVertexDeclaration.h"
#include "d3dUtil.h"
#include <d3d11.h>

struct PP_QUAD
{
	RVec3 pos;
};


RPostProcessorManager::RPostProcessorManager()
	: m_RTBuffer(nullptr)
	, m_RTDepthBuffer(nullptr)
	, m_RTView(nullptr)
	, m_RTSRV(nullptr)
	, m_RTDepthStencilView(nullptr)
{

}

void RPostProcessorManager::Initialize()
{
	// Create vertex shader for post processing
	m_PPVertexShader = RShaderManager::Instance().GetShaderResource("PostProcessor")->VertexShader;

	// Find vertex declaration for screen quad
	m_InputLayout = RVertexDeclaration::Instance().GetInputLayout(RVertex::SKYBOX_VERTEX::GetTypeName());

	// Create vertex buffer for screen quad
	{
		PP_QUAD quad[] =
		{
			RVec3(-1.0f, -1.0f, 1.0f),
			RVec3(-1.0f,  1.0f, 1.0f),
			RVec3(1.0f,  1.0f, 1.0f),

			RVec3(-1.0f, -1.0f, 1.0f),
			RVec3(1.0f,  1.0f, 1.0f),
			RVec3(1.0f, -1.0f, 1.0f),
		};
		m_ScreenQuad.CreateVertexBuffer(quad, sizeof(PP_QUAD), 6, m_InputLayout);
	}

	CreateRenderTargetResources();
}

void RPostProcessorManager::Release()
{
	m_ScreenQuad.Release();
	SAFE_RELEASE(m_RTDepthBuffer);
	SAFE_RELEASE(m_RTDepthStencilView);
	SAFE_RELEASE(m_RTBuffer);
	SAFE_RELEASE(m_RTView);
	SAFE_RELEASE(m_RTSRV);

	for (auto Iter : PostProcessingEffectList)
	{
		SAFE_RELEASE(Iter.second->PixelShader);
		delete Iter.second;
	}
}

void RPostProcessorManager::RecreateLostResources()
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

RPostProcessingEffect* RPostProcessorManager::CreateEffect(const string& Name, const void* pBytecode, SIZE_T BytecodeSize)
{
	// Assume we don't have a post processing effect with the same name
	assert(PostProcessingEffectList.find(Name) == PostProcessingEffectList.end());

	ID3D11PixelShader* PixelShader = RShaderManager::Instance().CreatePixelShaderFromBytecode(pBytecode, BytecodeSize);
	if (PixelShader)
	{
		RPostProcessingEffect* Effect = new RPostProcessingEffect(PixelShader);
		PostProcessingEffectList[Name] = Effect;
		return Effect;
	}

	RLogWarning("Failed to create post processing effect with name \"%s\"\n", Name.c_str());
	return nullptr;
}

void RPostProcessorManager::SetupRenderTarget()
{
	// Prepare to draw onto render target buffers (color and depth-stencil)
	GRenderer.SetRenderTargets(1, &m_RTView, m_RTDepthStencilView);

	// Set the size of viewport as full window buffer
	D3D11_VIEWPORT vp = { 0.0f, 0.0f, (FLOAT)GRenderer.GetClientWidth(), (FLOAT)GRenderer.GetClientHeight(), 0.0f, 1.0f };
	GRenderer.D3DImmediateContext()->RSSetViewports(1, &vp);
}

void RPostProcessorManager::Draw(RPostProcessingEffect* Effect)
{
	if (Effect)
	{
		GRenderer.SetPixelShader(Effect->PixelShader);
		GRenderer.SetVertexShader(m_PPVertexShader);
		GRenderer.SetGeometryShader(nullptr);

		// Do not set shader resource view in deferred rendering
		//GRenderer.D3DImmediateContext()->PSSetShaderResources(0, 1, &m_RTSRV);

		GRenderer.D3DImmediateContext()->IASetInputLayout(m_InputLayout);
		m_ScreenQuad.Draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
}

ID3D11ShaderResourceView* RPostProcessorManager::GetRenderTargetSRV() const
{
	return m_RTSRV;
}

void RPostProcessorManager::CreateRenderTargetResources()
{
	// Create render target
	D3D11_TEXTURE2D_DESC renderTargetTextureDesc;
	renderTargetTextureDesc.Width = GRenderer.GetClientWidth();
	renderTargetTextureDesc.Height = GRenderer.GetClientHeight();
	renderTargetTextureDesc.MipLevels = 1;
	renderTargetTextureDesc.ArraySize = 1;
	renderTargetTextureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	renderTargetTextureDesc.SampleDesc.Count = 1;
	renderTargetTextureDesc.SampleDesc.Quality = 0;
	renderTargetTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	renderTargetTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	renderTargetTextureDesc.CPUAccessFlags = 0;
	renderTargetTextureDesc.MiscFlags = 0;

	GRenderer.D3DDevice()->CreateTexture2D(&renderTargetTextureDesc, 0, &m_RTBuffer);
	GRenderer.D3DDevice()->CreateRenderTargetView(m_RTBuffer, 0, &m_RTView);

	D3D11_SHADER_RESOURCE_VIEW_DESC rtsrvDesc;
	rtsrvDesc.Format = renderTargetTextureDesc.Format;
	rtsrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	rtsrvDesc.Texture2D.MostDetailedMip = 0;
	rtsrvDesc.Texture2D.MipLevels = 1;

	GRenderer.D3DDevice()->CreateShaderResourceView(m_RTBuffer, &rtsrvDesc, &m_RTSRV);

	renderTargetTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	renderTargetTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	GRenderer.D3DDevice()->CreateTexture2D(&renderTargetTextureDesc, 0, &m_RTDepthBuffer);
	GRenderer.D3DDevice()->CreateDepthStencilView(m_RTDepthBuffer, 0, &m_RTDepthStencilView);
}
