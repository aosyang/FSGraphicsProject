//=============================================================================
// RMeshElement.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "RVertexDeclaration.h"

#include "RRenderSystemTypes.h"
#include <d3d11.h>

class RSerializer;

enum MeshElementFlag
{
	MEF_Skinned = 1 << 0,
};

struct VBoneIds
{
	int boneId[4];
};

/// Hardware vertex/index buffer for triangle mesh
class RMeshRenderBuffer
{
#if _DEBUG
	friend void SetDebuggerObjectName(RMeshRenderBuffer& RenderBuffer, const char* ObjectName);
#endif	// _DEBUG 

public:
	RMeshRenderBuffer();
	~RMeshRenderBuffer();

	void CreateVertexBuffer(void* data, UINT vertexTypeSize, UINT vertexCount, ID3D11InputLayout* inputLayout, EPrimitiveTopology PrimitiveTopology, bool dynamic = false);
	void CreateIndexBuffer(void* data, UINT indexTypeSize, UINT indexCount, bool dynamic = false);

	void UpdateDynamicVertexBuffer(void* data, UINT vertexTypeSize, UINT vertexCount);

	void Draw() const;
	void DrawInstanced(int instanceCount) const;

	UINT GetVertexCount() const;
	UINT GetIndexCount() const;

private:
	struct RBufferData;
	std::unique_ptr<RBufferData> BufferData;

	ID3D11InputLayout*	m_InputLayout;
	EPrimitiveTopology	m_PrimitiveTopology;

	UINT				m_Stride;
	UINT				m_VertexCount;
	UINT				m_IndexCount;
};


#if _DEBUG
// Set object name in graphics debugger (debug builds only)
void SetDebuggerObjectName(RMeshRenderBuffer& RenderBuffer, const char* ObjectName);
#else
#define SetDebuggerObjectName(RenderBuffer, Name)
#endif	// _DEBUG


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
	~RMeshElement();

	void Serialize(RSerializer& serializer);

	void SetTriangles(const std::vector<UINT>& triIndices);
	void SetVertices(const std::vector<RVertexType::MeshLoader>& vertices, int vertexComponentMask);
	void SetVertexComponentMask(int mask) { m_VertexComponentMask = mask; }
	int GetVertexComponentMask() const { return m_VertexComponentMask; }
	
	void UpdateRenderBuffer();

	void Draw() const;
	void DrawInstanced(int instanceCount) const;

	void SetName(const char* name)		{ m_Name = name; }
	const std::string& GetName() const	{ return m_Name; }

	const RAabb& GetAabb() const		{ return m_Aabb; }

	void SetFlag(int flag)				{ m_Flag = flag; }
	int GetFlag() const					{ return m_Flag; }

	std::vector<UINT>					TriangleIndices;
	std::vector<RVertexType::Vec3Data>	PositionArray;
	std::vector<RVertexType::Vec2Data>	UV0Array;
	std::vector<RVertexType::Vec3Data>	NormalArray;
	std::vector<RVertexType::Vec3Data>	TangentArray;
	std::vector<RVertexType::Vec2Data>	UV1Array;
	std::vector<VBoneIds>				BoneIdArray;
	std::vector<RVertexType::Vec4Data>	BoneWeightArray;

private:
	std::string			m_Name;
	RAabb				m_Aabb;
	UINT				m_Flag;

	std::unique_ptr<RMeshRenderBuffer>	m_RenderBuffer;

	int					m_VertexComponentMask;
};

