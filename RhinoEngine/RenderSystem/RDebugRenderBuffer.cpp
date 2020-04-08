//=============================================================================
// RDebugRenderBuffer.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "RDebugRenderBuffer.h"

#include "RShaderManager.h"
#include "RShaderConstantBuffer.h"

RDebugRenderBuffer::RDebugRenderBuffer()
{
}

void RDebugRenderBuffer::Initialize(EPrimitiveTopology InTopology, int InMaxVertexCount)
{
	m_MaxNumVertices = InMaxVertexCount;
	m_ColorShader = RShaderManager::Instance().GetShaderResource("Color");
	m_PrimitiveInputLayout = RVertexDeclaration::Instance().GetInputLayout<RVertexType::PositionColor>();
	m_PrimitiveMeshBuffer.CreateVertexBuffer(nullptr, sizeof(RVertexType::PositionColor), m_MaxNumVertices, m_PrimitiveInputLayout, InTopology, true);
}

void RDebugRenderBuffer::Release()
{
	m_PrimitiveMeshBuffer.Release();
}

void RDebugRenderBuffer::Render()
{
	assert(m_MaxNumVertices > 0);

	UINT NumVertices = (UINT)m_PrimitiveVertices.size();
	UINT VertexTypeSize = sizeof(RVertexType::PositionColor);

	if (NumVertices > m_MaxNumVertices)
	{
		// Render vertex numbers exceeded max number allowed, go for multiple draw calls
		RConstantBuffers::cbPerObject.Data.worldMatrix = RMatrix4::IDENTITY;

		RConstantBuffers::cbPerObject.UpdateBufferData();
		RConstantBuffers::cbPerObject.BindBuffer();

		m_ColorShader->Bind();
		GRenderer.D3DImmediateContext()->IASetInputLayout(m_PrimitiveInputLayout);

		UINT StartIndex = 0;

		while (StartIndex < NumVertices)
		{
			m_PrimitiveMeshBuffer.UpdateDynamicVertexBuffer(&m_PrimitiveVertices[StartIndex], VertexTypeSize, RMath::Min(NumVertices - StartIndex, m_MaxNumVertices));
			m_PrimitiveMeshBuffer.Draw();

			StartIndex += m_MaxNumVertices;
		}
	}
	else
	{
		if (m_bDirtyBuffer)
		{
			m_PrimitiveMeshBuffer.UpdateDynamicVertexBuffer(m_PrimitiveVertices.data(), VertexTypeSize, NumVertices);
			m_bDirtyBuffer = false;
		}

		if (m_PrimitiveMeshBuffer.GetVertexCount() != 0)
		{
			RConstantBuffers::cbPerObject.Data.worldMatrix = RMatrix4::IDENTITY;

			RConstantBuffers::cbPerObject.UpdateBufferData();
			RConstantBuffers::cbPerObject.BindBuffer();

			m_ColorShader->Bind();
			GRenderer.D3DImmediateContext()->IASetInputLayout(m_PrimitiveInputLayout);
			m_PrimitiveMeshBuffer.Draw();
		}
	}
}

void RDebugRenderBuffer::Reset()
{
	m_PrimitiveVertices.clear();
	m_bDirtyBuffer = true;
}
