//=============================================================================
// RDebugRenderer.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "RDebugRenderBuffer.h"

#define GDebugRenderer RDebugRenderer::Instance()

class RDebugRenderer : public RSingleton<RDebugRenderer>
{
	friend class RSingleton<RDebugRenderer>;
public:
	void Initialize(int maxVertexCount = 1048576);

	void DrawLine(const RVec3& start, const RVec3& end, const RColor& color = GDebugRenderer.GetPrimitiveColor());
	void DrawLine(const RVec3& start, const RVec3& end, const RColor& color1, const RColor& color2);

	void DrawBox(const RVec3& Dimensions, const RMatrix4& transform, const RColor& color = GDebugRenderer.GetPrimitiveColor());

	void DrawAabb(const RAabb& aabb, const RColor& color = GDebugRenderer.GetPrimitiveColor());

	void DrawFrustum(const RFrustum& frustum, const RColor& color = GDebugRenderer.GetPrimitiveColor());

	void DrawSphere(const RVec3& center, float radius, int segment = 12);
	void DrawSphere(const RSphere& Sphere, int segment = 12);
	void DrawSphere(const RVec3& center, float radius, const RColor& color, int segment = 12);
	void DrawSphere(const RSphere& Sphere, const RColor& color, int segment = 12);

	void DrawCapsule(const RVec3& Center, float Height, float Radius, const RColor& Color = GDebugRenderer.GetPrimitiveColor(), int Segment = 12);

	void DrawTriangle(const RVec3& v0, const RVec3& v1, const RVec3& v2, const RColor& Color = GDebugRenderer.GetPrimitiveColor());

	void SetPrimitiveColor(const RColor& color);
	const RColor& GetPrimitiveColor() const;

	// Present primitive to the screen
	void Render();

	// Clear debug renderer buffer for next frame
	void Reset();

protected:
	RDebugRenderer();
	~RDebugRenderer();

	void DrawHemisphereRange(const RVec3 Center, float Radius, float Start, float End, const RColor& Color = GDebugRenderer.GetPrimitiveColor(), int Segment = 12);

private:
	RColor									m_PrimitiveColor;
	RDebugRenderBuffer						LineBuffer;
	RDebugRenderBuffer						TriangleBuffer;
};


FORCEINLINE void RDebugRenderer::DrawLine(const RVec3& start, const RVec3& end, const RColor& color /*= GDebugRenderer.GetPrimitiveColor()*/)
{
	DrawLine(start, end, color, color);
}

//FORCEINLINE void RDebugRenderer::DrawAabb(const RAabb& aabb)
//{
//	DrawAabb(aabb, m_PrimitiveColor);
//}

FORCEINLINE void RDebugRenderer::DrawSphere(const RVec3& center, float radius, int segment)
{
	DrawSphere(center, radius, m_PrimitiveColor, segment);
}

FORCEINLINE void RDebugRenderer::SetPrimitiveColor(const RColor& color)
{
	m_PrimitiveColor = color;
}

FORCEINLINE const RColor& RDebugRenderer::GetPrimitiveColor() const
{
	return m_PrimitiveColor;
}

