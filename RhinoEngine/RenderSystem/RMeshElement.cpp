//=============================================================================
// RMeshElement.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "RMeshElement.h"

#include "D3DUtil.h"
#include "RRenderSystem.h"
#include "Core/RSerializer.h"

#include "D3DCommonPrivate.h"

namespace
{
	// Convert primitive topology types to D3D11 ones
	D3D11_PRIMITIVE_TOPOLOGY GetD3D11PrimitiveTopology(EPrimitiveTopology Topology)
	{
		switch (Topology)
		{
		case EPrimitiveTopology::PointList:
			return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
		case EPrimitiveTopology::LineList:
			return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
		case EPrimitiveTopology::TriangleList:
			return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		default:
			// Unimplemented conversation
			assert(0);
			return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
		}
	}
}

struct RMeshRenderBuffer::RBufferData
{
	ComPtr<ID3D11Buffer>		m_VertexBuffer;
	ComPtr<ID3D11Buffer>		m_IndexBuffer;
};

RMeshRenderBuffer::RMeshRenderBuffer()
	: BufferData(std::make_unique<RBufferData>())
	, m_InputLayout(nullptr)
	, m_PrimitiveTopology(EPrimitiveTopology::TriangleList)
{

}

RMeshRenderBuffer::~RMeshRenderBuffer()
{

}

void RMeshRenderBuffer::CreateVertexBuffer(void* data, UINT vertexTypeSize, UINT vertexCount, ID3D11InputLayout* inputLayout, EPrimitiveTopology PrimitiveTopology, bool dynamic /*= false*/)
{
	m_InputLayout = inputLayout;
	m_PrimitiveTopology = PrimitiveTopology;

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

		GRenderer.D3DDevice()->CreateBuffer(&vbd, &initVertexData, BufferData->m_VertexBuffer.GetAddressOf());
	}
	else
	{
		GRenderer.D3DDevice()->CreateBuffer(&vbd, NULL, BufferData->m_VertexBuffer.GetAddressOf());
	}

	m_VertexCount = vertexCount;
	m_Stride = vertexTypeSize;
}

void RMeshRenderBuffer::CreateIndexBuffer(void* data, UINT indexTypeSize, UINT indexCount, bool dynamic /*= false*/)
{
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = indexTypeSize * indexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initIndexData;
	ZeroMemory(&initIndexData, sizeof(initIndexData));
	initIndexData.pSysMem = data;

	GRenderer.D3DDevice()->CreateBuffer(&ibd, &initIndexData, BufferData->m_IndexBuffer.GetAddressOf());
	m_IndexCount = indexCount;
}

void RMeshRenderBuffer::UpdateDynamicVertexBuffer(void* data, UINT vertexTypeSize, UINT vertexCount)
{
	D3D11_MAPPED_SUBRESOURCE subres;
	HRESULT hr = GRenderer.D3DImmediateContext()->Map(BufferData->m_VertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
	assert(SUCCEEDED(hr));

	memcpy(subres.pData, data, vertexTypeSize * vertexCount);
	GRenderer.D3DImmediateContext()->Unmap(BufferData->m_VertexBuffer.Get(), 0);

	m_VertexCount = vertexCount;
}

void RMeshRenderBuffer::Draw() const
{
	if (BufferData->m_VertexBuffer)
	{
		assert(m_InputLayout);
		UINT offset = 0;

		GRenderer.D3DImmediateContext()->IASetInputLayout(m_InputLayout);
		GRenderer.D3DImmediateContext()->IASetVertexBuffers(0, 1, BufferData->m_VertexBuffer.GetAddressOf(), &m_Stride, &offset);
		GRenderer.D3DImmediateContext()->IASetPrimitiveTopology(GetD3D11PrimitiveTopology(m_PrimitiveTopology));
		
		if (BufferData->m_IndexBuffer)
		{
			GRenderer.D3DImmediateContext()->IASetIndexBuffer(BufferData->m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
			GRenderer.D3DImmediateContext()->DrawIndexed(m_IndexCount, 0, 0);
		}
		else
		{
			GRenderer.D3DImmediateContext()->Draw(m_VertexCount, 0);
		}

		GRenderer.Stats.DrawCalls++;
	}
}

void RMeshRenderBuffer::DrawInstanced(int instanceCount) const
{
	UINT offset = 0;

	if (BufferData->m_VertexBuffer)
	{
		assert(m_InputLayout);
		UINT offset = 0;
		
		GRenderer.D3DImmediateContext()->IASetInputLayout(m_InputLayout);
		GRenderer.D3DImmediateContext()->IASetVertexBuffers(0, 1, BufferData->m_VertexBuffer.GetAddressOf(), &m_Stride, &offset);
		GRenderer.D3DImmediateContext()->IASetPrimitiveTopology(GetD3D11PrimitiveTopology(m_PrimitiveTopology));

		if (BufferData->m_IndexBuffer)
		{
			GRenderer.D3DImmediateContext()->IASetIndexBuffer(BufferData->m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
			GRenderer.D3DImmediateContext()->DrawIndexedInstanced(m_IndexCount, instanceCount, 0, 0, 0);
		}
		else
		{
			GRenderer.D3DImmediateContext()->DrawInstanced(m_VertexCount, instanceCount, 0, 0);
		}

		GRenderer.Stats.DrawCalls++;
	}
}

#if _DEBUG
void SetDebuggerObjectName(RMeshRenderBuffer& RenderBuffer, const char* ObjectName)
{
	if (RenderBuffer.BufferData->m_VertexBuffer)
	{
		RenderBuffer.BufferData->m_VertexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)strlen(ObjectName), ObjectName);
	}

	if (RenderBuffer.BufferData->m_IndexBuffer)
	{
		RenderBuffer.BufferData->m_IndexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)strlen(ObjectName), ObjectName);
	}
}
#endif	// _DEBUG


RMeshElement::RMeshElement()
	: m_Flag(0)
	, m_VertexComponentMask(0)
	, m_RenderBuffer(std::make_unique<RMeshRenderBuffer>())
{

}

RMeshElement::~RMeshElement()
{

}

void RMeshElement::Serialize(RSerializer& serializer)
{
	serializer.SerializeData(m_Name);
	serializer.SerializeData(m_Flag);
	serializer.SerializeData(m_VertexComponentMask);
	serializer.SerializeVector(TriangleIndices);
	serializer.SerializeVector(PositionArray);
	serializer.SerializeVector(UV0Array);
	serializer.SerializeVector(NormalArray);
	serializer.SerializeVector(TangentArray);
	serializer.SerializeVector(UV1Array);
	serializer.SerializeVector(BoneIdArray);
	serializer.SerializeVector(BoneWeightArray);

	if (serializer.IsReading())
		UpdateRenderBuffer();
}

void RMeshElement::SetTriangles(const std::vector<UINT>& triIndices)
{
	TriangleIndices = triIndices;
}

void RMeshElement::SetVertices(const std::vector<RVertexType::MeshLoader>& vertices, int vertexComponentMask)
{
	m_VertexComponentMask = vertexComponentMask;

	TriangleIndices.clear();
	PositionArray.clear();
	UV0Array.clear();
	NormalArray.clear();
	TangentArray.clear();
	UV1Array.clear();
	BoneIdArray.clear();
	BoneWeightArray.clear();

	for (size_t i = 0; i < vertices.size(); i++)
	{
		if (vertexComponentMask & VCM_Pos)
			PositionArray.push_back(vertices[i].pos);
		if (vertexComponentMask & VCM_UV0)
			UV0Array.push_back(vertices[i].uv0);
		if (vertexComponentMask & VCM_Normal)
			NormalArray.push_back(vertices[i].normal);
		if (vertexComponentMask & VCM_Tangent)
			TangentArray.push_back(vertices[i].tangent);
		if (vertexComponentMask & VCM_UV1)
			UV1Array.push_back(vertices[i].uv1);
		if (vertexComponentMask & VCM_BoneId)
			BoneIdArray.push_back(*((VBoneIds*)vertices[i].boneId));
		if (vertexComponentMask & VCM_BoneWeights)
			BoneWeightArray.push_back(vertices[i].weight);
	}
}

void RMeshElement::UpdateRenderBuffer()
{
	if (!GRenderer.HasInitialized())
	{
		return;
	}

	m_RenderBuffer = std::make_unique<RMeshRenderBuffer>();

	int VertexComponentMask = m_VertexComponentMask;

	// TODO: For now, I'm rendering skeletal meshes without bone id and bone weights in the editor.
	//		 This needs to be further changed once we have something like an animation editor.
	if (GEngine.IsEditor())
	{
		VertexComponentMask &= ~(VCM_BoneId | VCM_BoneWeights);
	}

	int stride = RVertexDeclaration::GetVertexStride(VertexComponentMask);
	ID3D11InputLayout* inputLayout = RVertexDeclaration::Instance().GetInputLayoutByVertexComponents(VertexComponentMask);

	BYTE* compactVertexData = new BYTE[PositionArray.size() * stride];
	
	int offset = 0;

#define COPY_VERTEX_COMPONENT(Mask, VArray, Type) \
	if (VertexComponentMask & Mask) \
	{ \
		memcpy(compactVertexData + offset, &VArray[i], sizeof(Type)); \
		offset += sizeof(Type); \
	}

	for (size_t i = 0; i < PositionArray.size(); i++)
	{
		COPY_VERTEX_COMPONENT(VCM_BoneId,		BoneIdArray,		VBoneIds)
		COPY_VERTEX_COMPONENT(VCM_BoneWeights,	BoneWeightArray,	RVertexType::Vec4Data)
		COPY_VERTEX_COMPONENT(VCM_Pos,			PositionArray,		RVertexType::Vec3Data)
		COPY_VERTEX_COMPONENT(VCM_Normal,		NormalArray,		RVertexType::Vec3Data)
		COPY_VERTEX_COMPONENT(VCM_Tangent,		TangentArray,		RVertexType::Vec3Data)
		COPY_VERTEX_COMPONENT(VCM_UV0,			UV0Array,			RVertexType::Vec2Data)
		COPY_VERTEX_COMPONENT(VCM_UV1,			UV1Array,			RVertexType::Vec2Data)
	}
#undef COPY_VERTEX_COMPONENT

	m_RenderBuffer->CreateVertexBuffer(compactVertexData, stride, (UINT)PositionArray.size(), inputLayout, EPrimitiveTopology::TriangleList, false);
	m_RenderBuffer->CreateIndexBuffer(TriangleIndices.data(), sizeof(UINT), (UINT)TriangleIndices.size(), false);
	SetDebuggerObjectName(*m_RenderBuffer, m_Name.c_str());

	delete[] compactVertexData;

	// Update aabb
	m_Aabb = RAabb::Default;
	for (size_t i = 0; i < PositionArray.size(); i++)
	{
		m_Aabb.Expand(RVec3((float*)&PositionArray[i]));
	}
}

void RMeshElement::Draw() const
{
	m_RenderBuffer->Draw();
}

void RMeshElement::DrawInstanced(int instanceCount) const
{
	m_RenderBuffer->DrawInstanced(instanceCount);
}

