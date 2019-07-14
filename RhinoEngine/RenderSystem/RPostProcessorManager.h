//=============================================================================
// RPostProcessor.h by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

struct RPostProcessingEffect
{
	RPostProcessingEffect(ID3D11PixelShader* InShader)
		: PixelShader(InShader)
	{
	}

	ID3D11PixelShader*	PixelShader;
};

class RPostProcessorManager : public RSingleton<RPostProcessorManager>
{
	friend class RSingleton<RPostProcessorManager>;

public:
	void Initialize();
	void Release();
	void RecreateLostResources();

	RPostProcessingEffect* CreateEffectFromFile(const string& Name, const string& FileName, const char* EntryPoint = "main", const char* ShaderProfile = "ps_4_0");
	RPostProcessingEffect* CreateEffectFromBytecode(const string& Name, const void* pBytecode, SIZE_T BytecodeSize);

	void SetupRenderTarget();
	void Draw(RPostProcessingEffect* Effect);

	ID3D11ShaderResourceView* GetRenderTargetSRV() const;

protected:
	RPostProcessorManager();

private:
	void CreateRenderTargetResources();

	ID3D11VertexShader*			m_PPVertexShader;

	ID3D11InputLayout*			m_InputLayout;
	RMeshRenderBuffer			m_ScreenQuad;

	// The color buffer resource of render target
	ID3D11Texture2D*			m_RTBuffer;

	// The depth and stencil buffer resource of render target
	ID3D11Texture2D*			m_RTDepthBuffer;

	// The resource used to draw into the color buffer of render target
	ID3D11RenderTargetView*		m_RTView;

	// The resource to use render target buffer as an shader texture input
	ID3D11ShaderResourceView*	m_RTSRV;

	// The resource used to draw into the depth and stencil buffer of render target
	ID3D11DepthStencilView*		m_RTDepthStencilView;

	map<string, RPostProcessingEffect*>		PostProcessingEffectList;
};

#define GPostProcessorManager RPostProcessorManager::Instance()
