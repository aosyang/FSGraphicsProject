//=============================================================================
// RShaderConstantBuffer.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#ifndef _RSHADERCONSTANTBUFFER_H
#define _RSHADERCONSTANTBUFFER_H

enum EConstantBufferShaderType
{
	CBST_VS	= 1 << 0,
	CBST_PS	= 1 << 1,
	CBST_GS	= 1 << 2,
};

template <typename T, int SHADER_TYPE, int SLOT>
class RShaderConstantBuffer
{
public:
	RShaderConstantBuffer()
		: m_ConstBuffer(nullptr)
	{}

	void Initialize()
	{
		D3D11_BUFFER_DESC cbDesc;
		ZeroMemory(&cbDesc, sizeof(cbDesc));
		cbDesc.ByteWidth = sizeof(T);
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;

		RRenderer.D3DDevice()->CreateBuffer(&cbDesc, NULL, &m_ConstBuffer);
	}

	void Release()
	{
		SAFE_RELEASE(m_ConstBuffer);
	}

	void UpdateContent(const T* data)
	{
		D3D11_MAPPED_SUBRESOURCE subres;
		RRenderer.D3DImmediateContext()->Map(m_ConstBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
		memcpy(subres.pData, data, sizeof(T));
		RRenderer.D3DImmediateContext()->Unmap(m_ConstBuffer, 0);
	}

	// Send constant buffer data to shaders
	void ApplyToShaders()
	{
		if (SHADER_TYPE & CBST_VS)
			RRenderer.D3DImmediateContext()->VSSetConstantBuffers(SLOT, 1, &m_ConstBuffer);
		if (SHADER_TYPE & CBST_PS)
			RRenderer.D3DImmediateContext()->PSSetConstantBuffers(SLOT, 1, &m_ConstBuffer);
		if (SHADER_TYPE & CBST_GS)
			RRenderer.D3DImmediateContext()->GSSetConstantBuffers(SLOT, 1, &m_ConstBuffer);
	}

private:
	ID3D11Buffer*		m_ConstBuffer;
};

#endif
