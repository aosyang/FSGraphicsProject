//=============================================================================
// DeferredShadingApp.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#ifndef _DEFERREDSHADINGAPP_H
#define _DEFERREDSHADINGAPP_H

#include "Rhino.h"

struct DeferredRenderBuffer
{
	ID3D11Texture2D*			Buffer;
	ID3D11RenderTargetView*		View;
	ID3D11ShaderResourceView*	SRV;

	void Release()
	{
		Buffer->Release();
		View->Release();
		SRV->Release();
	}
};

struct DepthStencilBuffer
{
	ID3D11Texture2D*			Buffer;
	ID3D11DepthStencilView*		View;

	void Release()
	{
		Buffer->Release();
		View->Release();
	}
};

enum EDeferredBuffer
{
	DB_Color,
	DB_Position,
	DB_Normal,

	DeferredBuffer_Count
};

class DeferredShadingApp : public IApp
{
public:
	DeferredShadingApp();
	~DeferredShadingApp();

	bool Initialize();
	void UpdateScene(const RTimer& timer);
	void RenderScene();

	void OnResize(int width, int height);
	TCHAR* WindowTitle() { return L"Deferred Shading Demo"; }

private:
	DeferredRenderBuffer CreateRenderTarget();
	DepthStencilBuffer CreateDepthStencilBuffer();

	RCamera						m_Camera;
	float						m_CamPitch, m_CamYaw;

	RScene						m_Scene;
	RSMeshObject				m_MeshObj;

	DeferredRenderBuffer		m_DeferredBuffers[DeferredBuffer_Count];
	DepthStencilBuffer			m_DepthBuffer;
};

#endif
