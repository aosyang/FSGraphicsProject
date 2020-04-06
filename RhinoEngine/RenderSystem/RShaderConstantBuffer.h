//=============================================================================
// RShaderConstantBuffer.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "../Shaders/ConstBufferVS.h"
#include "../Shaders/ConstBufferPS.h"

#include "D3DUtil.h"
#include "RRenderSystem.h"

enum EConstantBufferShaderType
{
	CBST_VS	= 1 << 0,
	CBST_PS	= 1 << 1,
	CBST_GS	= 1 << 2,
};

template <typename TStructType, int SHADER_TYPE, int SLOT>
class RShaderConstantBuffer
{
public:
	TStructType Data;

	RShaderConstantBuffer()
		: m_ConstBuffer(nullptr)
	{}

	void Initialize()
	{
		D3D11_BUFFER_DESC cbDesc;
		ZeroMemory(&cbDesc, sizeof(cbDesc));
		cbDesc.ByteWidth = sizeof(TStructType);
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;

		GRenderer.D3DDevice()->CreateBuffer(&cbDesc, NULL, &m_ConstBuffer);

		// Initialize buffer values with zero
		ClearData();
		UpdateBufferData();
		BindBuffer();
	}

	void Release()
	{
		SAFE_RELEASE(m_ConstBuffer);
	}

	/// Copy data to GPU buffer
	void UpdateBufferData()
	{
		D3D11_MAPPED_SUBRESOURCE subres;
		GRenderer.D3DImmediateContext()->Map(m_ConstBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
		memcpy(subres.pData, &Data, sizeof(TStructType));
		GRenderer.D3DImmediateContext()->Unmap(m_ConstBuffer, 0);
	}

	/// Bind constant buffer to shader for rendering
	void BindBuffer()
	{
		if (SHADER_TYPE & CBST_VS)
			GRenderer.D3DImmediateContext()->VSSetConstantBuffers(SLOT, 1, &m_ConstBuffer);
		if (SHADER_TYPE & CBST_PS)
			GRenderer.D3DImmediateContext()->PSSetConstantBuffers(SLOT, 1, &m_ConstBuffer);
		if (SHADER_TYPE & CBST_GS)
			GRenderer.D3DImmediateContext()->GSSetConstantBuffers(SLOT, 1, &m_ConstBuffer);
	}

	void ClearData()
	{
		ZeroMemory(&Data, sizeof(Data));
	}

private:
	ID3D11Buffer*		m_ConstBuffer;
};

/// All constant buffers used by render system
class RConstantBuffers
{
public:
	// Constant buffers
	static RShaderConstantBuffer<SHADER_SCENE_BUFFER,		CBST_VS|CBST_GS|CBST_PS, 0>		cbScene;
	static RShaderConstantBuffer<SHADER_GLOBAL_BUFFER,		CBST_VS|CBST_PS, 1>				cbGlobal;
	static RShaderConstantBuffer<SHADER_OBJECT_BUFFER,		CBST_VS, 2>						cbPerObject;
	static RShaderConstantBuffer<SHADER_SKINNED_BUFFER,		CBST_VS, 4>						cbBoneMatrices;
	static RShaderConstantBuffer<SHADER_LIGHT_BUFFER,		CBST_PS, 2>						cbLight;
	static RShaderConstantBuffer<SHADER_MATERIAL_BUFFER,	CBST_PS, 3>						cbMaterial;

	static void Initialize();
	static void Shutdown();
};

FORCEINLINE void RConstantBuffers::Initialize()
{
	cbPerObject.Initialize();
	cbScene.Initialize();
	cbBoneMatrices.Initialize();
	cbLight.Initialize();
	cbMaterial.Initialize();
	cbGlobal.Initialize();
}

FORCEINLINE void RConstantBuffers::Shutdown()
{
	cbPerObject.Release();
	cbScene.Release();
	cbBoneMatrices.Release();
	cbLight.Release();
	cbMaterial.Release();
	cbGlobal.Release();
}

