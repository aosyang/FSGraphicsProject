//=============================================================================
// RShadowMap.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "Core/CoreTypes.h"

struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11ShaderResourceView;
struct ID3D11Texture2D;

class RShadowMap
{
public:
	RShadowMap();
	~RShadowMap();

	void Initialize(int width, int height);
	void SetViewMatrix(const RMatrix4& view) { m_ViewMatrix = view; m_bNeedUpdateFrustum = true; }
	void SetOrthogonalProjection(float viewWidth, float viewHeight, float nearZ, float farZ);
	void SetupRenderTarget();

	RMatrix4 GetViewMatrix() const { return m_ViewMatrix; }
	RMatrix4 GetProjectionMatrix() const { return m_ProjMatrix; }
	RFrustum GetFrustum();

	ID3D11RenderTargetView* GetRenderTargetView()		{ return m_RenderTargetView; }
	ID3D11DepthStencilView* GetDepthView()				{ return m_DepthView; }
	ID3D11ShaderResourceView* GetRenderTargetSRV()		{ return m_RenderTargetSRV; }
	ID3D11ShaderResourceView* GetRenderTargetDepthSRV()	{ return m_RenderTargetDepthSRV; }

	// Texture slot number which matches register of ShadowDepthTexture in LightShaderCommon.hlsli
	static int ShaderResourceSlot()						{ return 5; }

private:
	ID3D11Texture2D*			m_RenderTargetBuffer;
	ID3D11Texture2D*			m_DepthBuffer;
	ID3D11RenderTargetView*		m_RenderTargetView;
	ID3D11DepthStencilView*		m_DepthView;
	ID3D11ShaderResourceView*	m_RenderTargetSRV;
	ID3D11ShaderResourceView*	m_RenderTargetDepthSRV;

	RMatrix4					m_ViewMatrix;
	RMatrix4					m_ProjMatrix;

	int							m_BufferWidth, m_BufferHeight;
	float						m_ViewWidth, m_ViewHeight;
	float						m_ViewNear, m_ViewFar;
	bool						m_bNeedUpdateFrustum;
	RFrustum					m_Frustum;
};

