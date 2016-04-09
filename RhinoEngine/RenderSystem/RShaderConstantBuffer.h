//=============================================================================
// RShaderConstantBuffer.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#ifndef _RSHADERCONSTANTBUFFER_H
#define _RSHADERCONSTANTBUFFER_H

template <typename T>
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

	void UpdateContent(const void* data)
	{
		D3D11_MAPPED_SUBRESOURCE subres;
		RRenderer.D3DImmediateContext()->Map(m_ConstBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
		memcpy(subres.pData, data, sizeof(T));
		RRenderer.D3DImmediateContext()->Unmap(m_ConstBuffer, 0);
	}

	ID3D11Buffer* const* GetDeviceBuffer() const { return &m_ConstBuffer; }

private:
	ID3D11Buffer*		m_ConstBuffer;
};

#endif
