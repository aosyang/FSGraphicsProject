//=============================================================================
// RDebugRenderBuffer.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

class RDebugRenderBuffer
{
public:
	RDebugRenderBuffer();

	void Initialize(D3D11_PRIMITIVE_TOPOLOGY InTopology, int InMaxVertexCount);
	void Release();

	void AppendVertex(const RVec3& Position, const RColor& Color);
	void AppendVertex(const RVertexType::PositionColor& InVertex);

	void Render();
	void Reset();

private:
	RMeshRenderBuffer							m_PrimitiveMeshBuffer;
	ID3D11InputLayout*							m_PrimitiveInputLayout;
	RShader*									m_ColorShader;

	std::vector<RVertexType::PositionColor>		m_PrimitiveVertices;
	bool										m_bDirtyBuffer;
	UINT										m_MaxNumVertices;

	D3D11_PRIMITIVE_TOPOLOGY					Topology;
};

FORCEINLINE void RDebugRenderBuffer::AppendVertex(const RVec3& Position, const RColor& Color)
{
	m_PrimitiveVertices.push_back({ RVec4(Position), Color });
}

FORCEINLINE void RDebugRenderBuffer::AppendVertex(const RVertexType::PositionColor& InVertex)
{
	m_PrimitiveVertices.push_back(InVertex);
}
