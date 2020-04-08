//=============================================================================
// RDebugRenderBuffer.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "RRenderSystemTypes.h"
#include "RVertexDeclaration.h"

struct RShader;
class RMeshRenderBuffer;

struct ID3D11InputLayout;

namespace RVertexType
{
	struct PositionColor;
}

class RDebugRenderBuffer
{
public:
	RDebugRenderBuffer();
	~RDebugRenderBuffer();

	void Initialize(EPrimitiveTopology InTopology, int InMaxVertexCount);

	void AppendVertex(const RVec3& Position, const RColor& Color);
	void AppendVertex(const RVertexType::PositionColor& InVertex);

	void Render();
	void Reset();

private:
	std::unique_ptr<RMeshRenderBuffer>			m_PrimitiveMeshBuffer;
	ID3D11InputLayout*							m_PrimitiveInputLayout;
	RShader*									m_ColorShader;

	std::vector<RVertexType::PositionColor>		m_PrimitiveVertices;
	bool										m_bDirtyBuffer;
	UINT										m_MaxNumVertices;
};

FORCEINLINE void RDebugRenderBuffer::AppendVertex(const RVec3& Position, const RColor& Color)
{
	m_PrimitiveVertices.push_back({ RVec4(Position), Color });
}

FORCEINLINE void RDebugRenderBuffer::AppendVertex(const RVertexType::PositionColor& InVertex)
{
	m_PrimitiveVertices.push_back(InVertex);
}
