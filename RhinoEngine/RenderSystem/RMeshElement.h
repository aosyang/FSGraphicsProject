//=============================================================================
// RMeshElement.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#ifndef _RMESHELEMENT_H
#define _RMESHELEMENT_H

#include <d3d11.h>

class RMeshElement
{
protected:
	ID3D11Buffer*		m_VertexBuffer;
	ID3D11Buffer*		m_IndexBuffer;

	UINT				m_Stride;
	UINT				m_VertexCount;
	UINT				m_IndexCount;

	string				m_Name;
public:
	RMeshElement();

	void Release();

	void CreateVertexBuffer(void* data, UINT vertexTypeSize, UINT vertexCount, bool dynamic = false);
	void CreateIndexBuffer(void* data, UINT indexTypeSize, UINT indexCount, bool dynamic = false);

	void UpdateDynamicVertexBuffer(void* data, UINT vertexTypeSize, UINT vertexCount);

	void Draw(D3D11_PRIMITIVE_TOPOLOGY topology);
	void DrawInstanced(int instanceCount, D3D11_PRIMITIVE_TOPOLOGY topology);

	void SetName(const char* name) { m_Name = name; }
	string GetName() const { return m_Name; }
};

#endif
