//=============================================================================
// RMeshElement.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#include "Rhino.h"

#include "RMeshElement.h"

RMeshElement::RMeshElement()
	: m_VertexBuffer(nullptr), m_IndexBuffer(nullptr)
{

}

void RMeshElement::Release()
{
	SAFE_RELEASE(m_VertexBuffer);
	SAFE_RELEASE(m_IndexBuffer);
}

void RMeshElement::CreateVertexBuffer(void* data, UINT vertexTypeSize, UINT vertexCount, bool dynamic /*= false*/)
{
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = vertexTypeSize * vertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	if (dynamic)
	{
		vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}

	if (data)
	{
		D3D11_SUBRESOURCE_DATA initVertexData;
		ZeroMemory(&initVertexData, sizeof(initVertexData));
		initVertexData.pSysMem = data;

		RRenderer.D3DDevice()->CreateBuffer(&vbd, &initVertexData, &m_VertexBuffer);
	}
	else
	{
		RRenderer.D3DDevice()->CreateBuffer(&vbd, NULL, &m_VertexBuffer);
	}

	m_VertexCount = vertexCount;
	m_Stride = vertexTypeSize;
}

void RMeshElement::CreateIndexBuffer(void* data, UINT indexTypeSize, UINT indexCount, bool dynamic /*= false*/)
{
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = indexTypeSize * indexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initIndexData;
	ZeroMemory(&initIndexData, sizeof(initIndexData));
	initIndexData.pSysMem = data;

	RRenderer.D3DDevice()->CreateBuffer(&ibd, &initIndexData, &m_IndexBuffer);
	m_IndexCount = indexCount;
}

void RMeshElement::UpdateDynamicVertexBuffer(void* data, UINT vertexTypeSize, UINT vertexCount)
{
	D3D11_MAPPED_SUBRESOURCE subres;
	RRenderer.D3DImmediateContext()->Map(m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
	memcpy(subres.pData, data, vertexTypeSize * vertexCount);
	RRenderer.D3DImmediateContext()->Unmap(m_VertexBuffer, 0);

	m_VertexCount = vertexCount;
}

void RMeshElement::Draw(D3D11_PRIMITIVE_TOPOLOGY topology)
{
	UINT offset = 0;

	if (m_IndexBuffer)
	{
		RRenderer.D3DImmediateContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &m_Stride, &offset);
		RRenderer.D3DImmediateContext()->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		RRenderer.D3DImmediateContext()->IASetPrimitiveTopology(topology);

		RRenderer.D3DImmediateContext()->DrawIndexed(m_IndexCount, 0, 0);
	}
	else if (m_VertexBuffer)
	{
		RRenderer.D3DImmediateContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &m_Stride, &offset);

		RRenderer.D3DImmediateContext()->IASetPrimitiveTopology(topology);

		RRenderer.D3DImmediateContext()->Draw(m_VertexCount, 0);
	}
}

void RMeshElement::DrawInstanced(int instanceCount, D3D11_PRIMITIVE_TOPOLOGY topology)
{
	UINT offset = 0;

	if (m_IndexBuffer)
	{
		RRenderer.D3DImmediateContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &m_Stride, &offset);
		RRenderer.D3DImmediateContext()->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		RRenderer.D3DImmediateContext()->IASetPrimitiveTopology(topology);

		RRenderer.D3DImmediateContext()->DrawIndexedInstanced(m_IndexCount, instanceCount, 0, 0, 0);
	}
	else if (m_VertexBuffer)
	{
		RRenderer.D3DImmediateContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &m_Stride, &offset);

		RRenderer.D3DImmediateContext()->IASetPrimitiveTopology(topology);

		RRenderer.D3DImmediateContext()->DrawInstanced(m_VertexCount, instanceCount, 0, 0);
	}
}