//=============================================================================
// RShadowMap.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#ifndef _RSHADOWMAP_H
#define _RSHADOWMAP_H

class RShadowMap
{
public:
	RShadowMap();
	~RShadowMap();

	void Initialize(int width, int height);
	void SetViewMatrix(const RMatrix4& view) { m_ViewMatrix = view; }
	void SetOrthogonalProjection(float viewWidth, float viewHeight, float nearZ, float farZ);
	void SetupRenderTarget();

	RMatrix4 GetViewMatrix() const { return m_ViewMatrix; }
	RMatrix4 GetProjectionMatrix() const { return m_ProjMatrix; }

	ID3D11RenderTargetView* GetRenderTargetView()		{ return m_RenderTargetView; }
	ID3D11DepthStencilView* GetDepthView()				{ return m_DepthView; }
	ID3D11ShaderResourceView* GetRenderTargetSRV()		{ return m_RenderTargetSRV; }
	ID3D11ShaderResourceView* GetRenderTargetDepthSRV()	{ return m_RenderTargetDepthSRV; }

private:
	ID3D11Texture2D*			m_RenderTargetBuffer;
	ID3D11Texture2D*			m_DepthBuffer;
	ID3D11RenderTargetView*		m_RenderTargetView;
	ID3D11DepthStencilView*		m_DepthView;
	ID3D11ShaderResourceView*	m_RenderTargetSRV;
	ID3D11ShaderResourceView*	m_RenderTargetDepthSRV;

	RMatrix4					m_ViewMatrix;
	RMatrix4					m_ProjMatrix;

	int							m_Width, m_Height;
};

#endif