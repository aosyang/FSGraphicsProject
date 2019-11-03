//=============================================================================
// RDebugRenderer.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

class RDebugRenderer : public RSingleton<RDebugRenderer>
{
	friend class RSingleton<RDebugRenderer>;
public:
	void Initialize(int maxVertexCount = 1048576);
	void Release();

	void SetPrimitiveColor(const RColor& color);
	void DrawLine(const RVec3& start, const RVec3& end);
	void DrawLine(const RVec3& start, const RVec3& end, const RColor& color);
	void DrawLine(const RVec3& start, const RVec3& end, const RColor& color1, const RColor& color2);
	void DrawAabb(const RAabb& aabb);
	void DrawAabb(const RAabb& aabb, const RColor& color);
	void DrawFrustum(const RFrustum& frustum);
	void DrawFrustum(const RFrustum& frustum, const RColor& color);

	void DrawSphere(const RVec3& center, float radius, int segment = 12);
	void DrawSphere(const RSphere& Sphere, int segment = 12);
	void DrawSphere(const RVec3& center, float radius, const RColor& color, int segment = 12);
	void DrawSphere(const RSphere& Sphere, const RColor& color, int segment = 12);

	// Present primitive to the screen
	void Render();

	// Clear debug renderer buffer for next frame
	void Reset();

protected:
	RDebugRenderer();
	~RDebugRenderer();

private:
	RMeshRenderBuffer						m_PrimitiveMeshBuffer;
	ID3D11InputLayout*						m_PrimitiveInputLayout;
	RShader*								m_ColorShader;

	RColor									m_PrimitiveColor;
	std::vector<RVertexType::PositionColor>		m_PrimitiveVertices;
	bool									m_bDirtyBuffer;
	UINT									m_MaxNumVertices;
};

#define GDebugRenderer RDebugRenderer::Instance()


FORCEINLINE void RDebugRenderer::DrawLine(const RVec3& start, const RVec3& end)
{
	DrawLine(start, end, m_PrimitiveColor, m_PrimitiveColor);
}

FORCEINLINE void RDebugRenderer::DrawLine(const RVec3& start, const RVec3& end, const RColor& color)
{
	DrawLine(start, end, color, color);
}

FORCEINLINE void RDebugRenderer::DrawAabb(const RAabb& aabb)
{
	DrawAabb(aabb, m_PrimitiveColor);
}

FORCEINLINE void RDebugRenderer::DrawFrustum(const RFrustum& frustum)
{
	DrawFrustum(frustum, m_PrimitiveColor);
}

FORCEINLINE void RDebugRenderer::DrawSphere(const RVec3& center, float radius, int segment)
{
	DrawSphere(center, radius, m_PrimitiveColor, segment);
}


