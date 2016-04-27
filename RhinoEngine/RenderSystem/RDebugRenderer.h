//=============================================================================
// RDebugRenderer.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#ifndef RDEBUGRENDERER_H
#define RDEBUGRENDERER_H

class RDebugRenderer
{
public:
	RDebugRenderer();
	~RDebugRenderer();

	void Initialize(int maxVertexCount = 65536);
	void Release();

	void SetColor(const RColor& color);
	void DrawLine(const RVec3& start, const RVec3& end);
	void DrawLine(const RVec3& start, const RVec3& end, const RColor& color1, const RColor& color2);
	void DrawAabb(const RAabb& aabb);
	void DrawAabb(const RAabb& aabb, const RColor& color);
	void DrawFrustum(const RFrustum& frustum);
	void DrawFrustum(const RFrustum& frustum, const RColor& color);
	void DrawSphere(const RVec3& center, float radius, int segment = 12);
	void DrawSphere(const RVec3& center, float radius, const RColor& color, int segment = 12);

	// Present primitive to the screen
	void Render();
	void Reset();

private:
	RMeshElement		m_PrimitiveMeshBuffer;
	ID3D11InputLayout*	m_PrimitiveInputLayout;
	RShader*			m_ColorShader;

	RColor				m_PrimitiveColor;
	vector<RVertex::PRIMITIVE_VERTEX>
						m_PrimitiveVertices;
	bool				m_bDirtyBuffer;
};

#endif
