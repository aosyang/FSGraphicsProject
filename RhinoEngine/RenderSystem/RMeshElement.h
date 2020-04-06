//=============================================================================
// RMeshElement.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "RVertexDeclaration.h"

enum MeshElementFlag
{
	MEF_Skinned = 1 << 0,
};

struct VBoneIds
{
	int boneId[4];
};

class RMeshRenderBuffer
{
protected:
	ID3D11Buffer*		m_VertexBuffer;
	ID3D11Buffer*		m_IndexBuffer;
	ID3D11InputLayout*	m_InputLayout;

	UINT				m_Stride;
	UINT				m_VertexCount;
	UINT				m_IndexCount;
public:
	RMeshRenderBuffer();

	void Release();

	void CreateVertexBuffer(void* data, UINT vertexTypeSize, UINT vertexCount, ID3D11InputLayout* inputLayout, bool dynamic = false, const char* debugResourceName = nullptr);
	void CreateIndexBuffer(void* data, UINT indexTypeSize, UINT indexCount, bool dynamic = false, const char* debugResourceName = nullptr);

	void UpdateDynamicVertexBuffer(void* data, UINT vertexTypeSize, UINT vertexCount);

	void Draw(D3D11_PRIMITIVE_TOPOLOGY topology) const;
	void DrawInstanced(int instanceCount, D3D11_PRIMITIVE_TOPOLOGY topology) const;

	UINT GetVertexCount() const;
	UINT GetIndexCount() const;
};

FORCEINLINE UINT RMeshRenderBuffer::GetVertexCount() const
{
	return m_VertexCount;
}

FORCEINLINE UINT RMeshRenderBuffer::GetIndexCount() const
{
	return m_IndexCount;
}

class RMeshElement
{
public:
	RMeshElement();

	void Release();

	void Serialize(RSerializer& serializer);

	void SetTriangles(const std::vector<UINT>& triIndices);
	void SetVertices(const std::vector<RVertexType::MeshLoader>& vertices, int vertexComponentMask);
	void SetVertexComponentMask(int mask) { m_VertexComponentMask = mask; }
	int GetVertexComponentMask() const { return m_VertexComponentMask; }
	
	void UpdateRenderBuffer();

	void Draw(D3D11_PRIMITIVE_TOPOLOGY topology) const;
	void DrawInstanced(int instanceCount, D3D11_PRIMITIVE_TOPOLOGY topology) const;


	void SetName(const char* name) { m_Name = name; }
	const std::string& GetName() const { return m_Name; }

	const RAabb& GetAabb() const { return m_Aabb; }

	void SetFlag(int flag) { m_Flag = flag; }
	int GetFlag() const { return m_Flag; }

	std::vector<UINT>				TriangleIndices;
	std::vector<RVertexType::Vec3Data>	PositionArray;
	std::vector<RVertexType::Vec2Data>	UV0Array;
	std::vector<RVertexType::Vec3Data>	NormalArray;
	std::vector<RVertexType::Vec3Data>	TangentArray;
	std::vector<RVertexType::Vec2Data>	UV1Array;
	std::vector<VBoneIds>			BoneIdArray;
	std::vector<RVertexType::Vec4Data>	BoneWeightArray;

private:
	std::string				m_Name;
	RAabb				m_Aabb;
	UINT				m_Flag;

	RMeshRenderBuffer	m_RenderBuffer;

	int					m_VertexComponentMask;
};

