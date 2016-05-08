//=============================================================================
// RPostProcessor.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#ifndef _RPOSTPROCESSOR_H
#define _RPOSTPROCESSOR_H

#include "Rhino.h"

enum PostProcessingEffect
{
	PPE_DeferredComposition,
	PPE_DeferredPointLightPass,

	PPE_COUNT,
};

class RPostProcessor
{
public:
	RPostProcessor();

	void Initialize();
	void Release();
	void RecreateLostResources();

	void SetupRenderTarget();
	void Draw(PostProcessingEffect effect);

private:
	void CreateRenderTargetResources();

	ID3D11VertexShader*			m_PPVertexShader;
	ID3D11PixelShader*			m_PPPixelShader[PPE_COUNT];

	ID3D11InputLayout*			m_InputLayout;
	RMeshRenderBuffer				m_ScreenQuad;

	ID3D11Texture2D*			m_RTBuffer;
	ID3D11Texture2D*			m_RTDepthBuffer;
	ID3D11RenderTargetView*		m_RTView;
	ID3D11ShaderResourceView*	m_RTSRV;
	ID3D11DepthStencilView*		m_RTDepthStencilView;
};

#endif
