//=============================================================================
// RMeshElement.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#include "Rhino.h"

#include "RMeshElement.h"

RMeshRenderBuffer::RMeshRenderBuffer()
	: m_VertexBuffer(nullptr), m_IndexBuffer(nullptr), m_InputLayout(nullptr)
{

}

void RMeshRenderBuffer::Release()
{
	SAFE_RELEASE(m_VertexBuffer);
	SAFE_RELEASE(m_IndexBuffer);
}

void RMeshRenderBuffer::CreateVertexBuffer(void* data, UINT vertexTypeSize, UINT vertexCount, ID3D11InputLayout* inputLayout, bool dynamic /*= false*/, const char* debugResourceName /*= nullptr*/)
{
	m_InputLayout = inputLayout;

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

		GRenderer.D3DDevice()->CreateBuffer(&vbd, &initVertexData, &m_VertexBuffer);
	}
	else
	{
		GRenderer.D3DDevice()->CreateBuffer(&vbd, NULL, &m_VertexBuffer);
	}

	m_VertexCount = vertexCount;
	m_Stride = vertexTypeSize;

#if _DEBUG
	if (debugResourceName)
		m_VertexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)strlen(debugResourceName), debugResourceName);
#endif
}

void RMeshRenderBuffer::CreateIndexBuffer(void* data, UINT indexTypeSize, UINT indexCount, bool dynamic /*= false*/, const char* debugResourceName/*=nullptr*/)
{
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = indexTypeSize * indexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initIndexData;
	ZeroMemory(&initIndexData, sizeof(initIndexData));
	initIndexData.pSysMem = data;

	GRenderer.D3DDevice()->CreateBuffer(&ibd, &initIndexData, &m_IndexBuffer);
	m_IndexCount = indexCount;

#if _DEBUG
	if (debugResourceName)
		m_IndexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)strlen(debugResourceName), debugResourceName);
#endif
}

void RMeshRenderBuffer::UpdateDynamicVertexBuffer(void* data, UINT vertexTypeSize, UINT vertexCount)
{
	D3D11_MAPPED_SUBRESOURCE subres;
	GRenderer.D3DImmediateContext()->Map(m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
	memcpy(subres.pData, data, vertexTypeSize * vertexCount);
	GRenderer.D3DImmediateContext()->Unmap(m_VertexBuffer, 0);

	m_VertexCount = vertexCount;
}

void RMeshRenderBuffer::Draw(D3D11_PRIMITIVE_TOPOLOGY topology) const
{
	UINT offset = 0;

	if (m_IndexBuffer)
	{
		assert(m_InputLayout);
		GRenderer.D3DImmediateContext()->IASetInputLayout(m_InputLayout);
		GRenderer.D3DImmediateContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &m_Stride, &offset);
		GRenderer.D3DImmediateContext()->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		GRenderer.D3DImmediateContext()->IASetPrimitiveTopology(topology);
		GRenderer.D3DImmediateContext()->DrawIndexed(m_IndexCount, 0, 0);
	}
	else if (m_VertexBuffer)
	{
		assert(m_InputLayout);
		GRenderer.D3DImmediateContext()->IASetInputLayout(m_InputLayout);
		GRenderer.D3DImmediateContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &m_Stride, &offset);
		GRenderer.D3DImmediateContext()->IASetPrimitiveTopology(topology);
		GRenderer.D3DImmediateContext()->Draw(m_VertexCount, 0);
	}
}

void RMeshRenderBuffer::DrawInstanced(int instanceCount, D3D11_PRIMITIVE_TOPOLOGY topology) const
{
	UINT offset = 0;

	if (m_IndexBuffer)
	{
		GRenderer.D3DImmediateContext()->IASetInputLayout(m_InputLayout);
		GRenderer.D3DImmediateContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &m_Stride, &offset);
		GRenderer.D3DImmediateContext()->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		GRenderer.D3DImmediateContext()->IASetPrimitiveTopology(topology);
		GRenderer.D3DImmediateContext()->DrawIndexedInstanced(m_IndexCount, instanceCount, 0, 0, 0);
	}
	else if (m_VertexBuffer)
	{
		GRenderer.D3DImmediateContext()->IASetInputLayout(m_InputLayout);
		GRenderer.D3DImmediateContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &m_Stride, &offset);
		GRenderer.D3DImmediateContext()->IASetPrimitiveTopology(topology);
		GRenderer.D3DImmediateContext()->DrawInstanced(m_VertexCount, instanceCount, 0, 0);
	}
}

RMeshElement::RMeshElement()
	: m_Flag(0), m_VertexComponentMask(0)
{

}

void RMeshElement::Release()
{
	m_RenderBuffer.Release();
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

void RMeshElement::SetTriangles(const vector<UINT>& triIndices)
{
	TriangleIndices = triIndices;
}

void RMeshElement::SetVertices(const vector<RVertex::MESH_LOADER_VERTEX>& vertices, int vertexComponentMask)
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
	m_RenderBuffer.Release();

	int stride = RVertexDeclaration::GetVertexStride(m_VertexComponentMask);
	ID3D11InputLayout* inputLayout = RVertexDeclaration::Instance().GetInputLayoutByVertexComponents(m_VertexComponentMask);

	BYTE* compactVertexData = new BYTE[PositionArray.size() * stride];
	
	int offset = 0;

#define COPY_VERTEX_COMPONENT(Mask, VArray, Type) \
	if (m_VertexComponentMask & Mask) \
	{ \
		memcpy(compactVertexData + offset, &VArray[i], sizeof(Type)); \
		offset += sizeof(Type); \
	}

	for (size_t i = 0; i < PositionArray.size(); i++)
	{
		COPY_VERTEX_COMPONENT(VCM_BoneId,		BoneIdArray,		VBoneIds)
		COPY_VERTEX_COMPONENT(VCM_BoneWeights,	BoneWeightArray,	RVertex::Vec4Data)
		COPY_VERTEX_COMPONENT(VCM_Pos,			PositionArray,		RVertex::Vec3Data)
		COPY_VERTEX_COMPONENT(VCM_UV0,			UV0Array,			RVertex::Vec2Data)
		COPY_VERTEX_COMPONENT(VCM_Normal,		NormalArray,		RVertex::Vec3Data)
		COPY_VERTEX_COMPONENT(VCM_Tangent,		TangentArray,		RVertex::Vec3Data)
		COPY_VERTEX_COMPONENT(VCM_UV1,			UV1Array,			RVertex::Vec2Data)
	}
#undef COPY_VERTEX_COMPONENT

	m_RenderBuffer.CreateVertexBuffer(compactVertexData, stride, (UINT)PositionArray.size(), inputLayout, false, m_Name.c_str());
	m_RenderBuffer.CreateIndexBuffer(TriangleIndices.data(), sizeof(UINT), (UINT)TriangleIndices.size(), false, m_Name.c_str());

	delete[] compactVertexData;

	// Update aabb
	m_Aabb = RAabb::Default;
	for (size_t i = 0; i < PositionArray.size(); i++)
	{
		m_Aabb.Expand(RVec3((float*)&PositionArray[i]));
	}
}

void RMeshElement::Draw(D3D11_PRIMITIVE_TOPOLOGY topology) const
{
	m_RenderBuffer.Draw(topology);
}

void RMeshElement::DrawInstanced(int instanceCount, D3D11_PRIMITIVE_TOPOLOGY topology) const
{
	m_RenderBuffer.DrawInstanced(instanceCount, topology);
}

