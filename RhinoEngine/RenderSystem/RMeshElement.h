//=============================================================================
// RMeshElement.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#ifndef _RMESHELEMENT_H
#define _RMESHELEMENT_H

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

	void CreateVertexBuffer(void* data, UINT vertexTypeSize, UINT vertexCount, ID3D11InputLayout* inputLayout, bool dynamic = false);
	void CreateIndexBuffer(void* data, UINT indexTypeSize, UINT indexCount, bool dynamic = false);

	void UpdateDynamicVertexBuffer(void* data, UINT vertexTypeSize, UINT vertexCount);

	void Draw(D3D11_PRIMITIVE_TOPOLOGY topology);
	void DrawInstanced(int instanceCount, D3D11_PRIMITIVE_TOPOLOGY topology);
};

class RMeshElement
{
public:
	RMeshElement();

	void Release();

	void Serialize(RSerializer& serializer);

	void SetTriangles(const vector<UINT>& triIndices);
	void SetVertices(const vector<RVertex::MESH_LOADER_VERTEX>& vertices, int vertexComponentMask);
	void SetVertexComponentMask(int mask) { m_VertexComponentMask = mask; }
	int GetVertexComponentMask() const { return m_VertexComponentMask; }
	
	void UpdateRenderBuffer();

	void Draw(D3D11_PRIMITIVE_TOPOLOGY topology);
	void DrawInstanced(int instanceCount, D3D11_PRIMITIVE_TOPOLOGY topology);


	void SetName(const char* name) { m_Name = name; }
	string GetName() const { return m_Name; }

	const RAabb& GetAabb() const { return m_Aabb; }

	void SetFlag(int flag) { m_Flag = flag; }
	int GetFlag() const { return m_Flag; }

	vector<UINT>		TriangleIndices;
	vector<RVec3>		PositionArray;
	vector<RVec2>		UV0Array;
	vector<RVec3>		NormalArray;
	vector<RVec3>		TangentArray;
	vector<RVec2>		UV1Array;
	vector<VBoneIds>	BoneIdArray;
	vector<RVec4>		BoneWeightArray;

private:
	string				m_Name;
	RAabb				m_Aabb;
	UINT				m_Flag;

	RMeshRenderBuffer	m_RenderBuffer;

	int					m_VertexComponentMask;
};

#endif
