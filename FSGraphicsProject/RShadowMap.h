//=============================================================================
// RShadowMap.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

class RShadowMap
{
public:
	RShadowMap();
	~RShadowMap();

	void Initialize(int width, int height);
	void SetViewMatrix(const XMMATRIX& view) { XMStoreFloat4x4(&m_ViewMatrix, view); }
	void SetOrthogonalProjection(float viewWidth, float viewHeight, float nearZ, float farZ);
	void SetupRenderTarget();

	XMMATRIX GetViewMatrix() const { return XMLoadFloat4x4(&m_ViewMatrix); }
	XMMATRIX GetProjectionMatrix() const { return XMLoadFloat4x4(&m_ProjMatrix); }

	ID3D11RenderTargetView* GetRenderTargetView()		{ return m_RenderTargetView; }
	ID3D11DepthStencilView* GetDepthView()				{ return m_DepthView; }
	ID3D11ShaderResourceView* GetRenderTargetSRV()		{ return m_RenderTargetSRV; }

private:
	ID3D11Texture2D*			m_RenderTargetBuffer;
	ID3D11Texture2D*			m_DepthBuffer;
	ID3D11RenderTargetView*		m_RenderTargetView;
	ID3D11DepthStencilView*		m_DepthView;
	ID3D11ShaderResourceView*	m_RenderTargetSRV;

	XMFLOAT4X4					m_ViewMatrix;
	XMFLOAT4X4					m_ProjMatrix;

	int							m_Width, m_Height;
};