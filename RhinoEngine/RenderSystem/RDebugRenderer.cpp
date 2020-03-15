//=============================================================================
// RDebugRenderer.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"
#include "RDebugRenderer.h"


RDebugRenderer::RDebugRenderer()
	: m_PrimitiveColor			(0.0f, 1.0f, 0.0f)
{
}


RDebugRenderer::~RDebugRenderer()
{
}

void RDebugRenderer::Initialize(int maxVertexCount)
{
	LineBuffer.Initialize(D3D11_PRIMITIVE_TOPOLOGY_LINELIST, maxVertexCount);
	TriangleBuffer.Initialize(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, maxVertexCount);
}

void RDebugRenderer::Release()
{
	LineBuffer.Release();
	TriangleBuffer.Release();
}

void RDebugRenderer::DrawLine(const RVec3& start, const RVec3& end, const RColor& color1, const RColor& color2)
{
	LineBuffer.AppendVertex(start, color1);
	LineBuffer.AppendVertex(end, color2);
}

void RDebugRenderer::DrawBox(const RVec3& Dimensions, const RMatrix4& transform, const RColor& color /*= GDebugRenderer.GetPrimitiveColor()*/)
{
	RVec3 cornerPoints[] =
	{
		RVec3(-Dimensions.X(), -Dimensions.Y(), -Dimensions.Z()) / 2,
		RVec3(-Dimensions.X(), -Dimensions.Y(),  Dimensions.Z()) / 2,
		RVec3(-Dimensions.X(),  Dimensions.Y(),  Dimensions.Z()) / 2,
		RVec3(-Dimensions.X(),  Dimensions.Y(), -Dimensions.Z()) / 2,

		RVec3(Dimensions.X(), -Dimensions.Y(), -Dimensions.Z()) / 2,
		RVec3(Dimensions.X(), -Dimensions.Y(),  Dimensions.Z()) / 2,
		RVec3(Dimensions.X(),  Dimensions.Y(),  Dimensions.Z()) / 2,
		RVec3(Dimensions.X(),  Dimensions.Y(), -Dimensions.Z()) / 2,
	};

	int wiredCubeIdx[] =
	{
		0, 1, 1, 2, 2, 3, 3, 0,
		4, 5, 5, 6, 6, 7, 7, 4,
		0, 4, 1, 5, 2, 6, 3, 7,
	};

	for (int i = 0; i < 24; i++)
	{
		RVertexType::PositionColor v =
		{
			RVec4(cornerPoints[wiredCubeIdx[i]]) * transform,
			color,
		};
		LineBuffer.AppendVertex(v);
	}
}

void RDebugRenderer::DrawAabb(const RAabb& aabb, const RColor& color /*= GDebugRenderer.GetPrimitiveColor() */)
{
	DrawBox(aabb.GetLocalDimension(), RMatrix4::CreateTranslation(aabb.GetCenter()), color);
}

void RDebugRenderer::DrawFrustum(const RFrustum& frustum, const RColor& color /*= GDebugRenderer.GetPrimitiveColor() */)
{
	int wiredCubeIdx[] =
	{
		0, 1, 1, 3, 3, 2, 2, 0,
		4, 5, 5, 7, 7, 6, 6, 4,
		0, 4, 1, 5, 2, 6, 3, 7,
	};

	for (int i = 0; i < 24; i++)
	{
		RVertexType::PositionColor v =
		{
			RVec4(frustum.corners[wiredCubeIdx[i]]),
			color,
		};
		LineBuffer.AppendVertex(v);
	}
}

void RDebugRenderer::DrawSphere(const RVec3& center, float radius, const RColor& color, int segment)
{
	// Draw latitude lines (vertical)
	for (int i = 0; i < segment; i++)
	{
		for (int j = 1; j < segment; j++)
		{
			float y = cosf(PI * j / segment) * radius + center.Y();
			float r = sinf(PI * j / segment) * radius;

			float x0 = sinf(2.0f * PI * i / segment) * r + center.X();
			float z0 = cosf(2.0f * PI * i / segment) * r + center.Z();

			int i1 = (i + 1) % segment;
			float x1 = sinf(2.0f * PI * i1 / segment) * r + center.X();
			float z1 = cosf(2.0f * PI * i1 / segment) * r + center.Z();

			RVertexType::PositionColor v0 = { RVec4(x0, y, z0), color };
			RVertexType::PositionColor v1 = { RVec4(x1, y, z1), color };
			LineBuffer.AppendVertex(v0);
			LineBuffer.AppendVertex(v1);
		}
	}

	// Draw longitude lines (horizontal)
	for (int i = 0; i < segment; i++)
	{
		for (int j = 0; j < segment; j++)
		{
			float y0 = cosf(PI * j / segment) * radius + center.Y();
			float r0 = sinf(PI * j / segment) * radius;

			int j1 = j + 1;
			float y1 = cosf(PI * j1 / segment) * radius + center.Y();
			float r1 = sinf(PI * j1 / segment) * radius;

			float x0 = sinf(2.0f * PI * i / segment) * r0 + center.X();
			float z0 = cosf(2.0f * PI * i / segment) * r0 + center.Z();

			float x1 = sinf(2.0f * PI * i / segment) * r1 + center.X();
			float z1 = cosf(2.0f * PI * i / segment) * r1 + center.Z();

			RVertexType::PositionColor v0 = { RVec4(x0, y0, z0), color };
			RVertexType::PositionColor v1 = { RVec4(x1, y1, z1), color };
			LineBuffer.AppendVertex(v0);
			LineBuffer.AppendVertex(v1);
		}
	}
}

void RDebugRenderer::DrawSphere(const RSphere& Sphere, int segment /*= 12*/)
{
	DrawSphere(Sphere.center, Sphere.radius, segment);
}

void RDebugRenderer::DrawSphere(const RSphere& Sphere, const RColor& color, int segment /*= 12*/)
{
	DrawSphere(Sphere.center, Sphere.radius, color, segment);
}

void RDebugRenderer::DrawCapsule(const RVec3& Center, float Height, float Radius, const RColor& Color /*= GDebugRenderer.GetPrimitiveColor()*/, int Segment /*= 12*/)
{
	RVec3 UpperCenter = Center + RVec3(0, 0.5f, 0) * Height;
	RVec3 LowerCenter = Center + RVec3(0, -0.5f, 0) * Height;
	DrawHemisphereRange(UpperCenter, Radius, 0.0f, 0.5f, Color, Segment);
	DrawHemisphereRange(LowerCenter, Radius, 0.5f, 1.0f, Color, Segment);

	// Cylindrical vertical lines
	for (int i = 0; i < Segment; i++)
	{
		float y0 = UpperCenter.Y();
		float y1 = LowerCenter.Y();

		float x0 = sinf(2.0f * PI * i / Segment) * Radius + Center.X();
		float z0 = cosf(2.0f * PI * i / Segment) * Radius + Center.Z();

		float x1 = sinf(2.0f * PI * i / Segment) * Radius + Center.X();
		float z1 = cosf(2.0f * PI * i / Segment) * Radius + Center.Z();

		RVertexType::PositionColor v0 = { RVec4(x0, y0, z0), Color };
		RVertexType::PositionColor v1 = { RVec4(x1, y1, z1), Color };
		LineBuffer.AppendVertex(v0);
		LineBuffer.AppendVertex(v1);
	}
}

void RDebugRenderer::DrawTriangle(const RVec3& v0, const RVec3& v1, const RVec3& v2, const RColor& Color /*= GDebugRenderer.GetPrimitiveColor() */)
{
	TriangleBuffer.AppendVertex({ RVec4(v0), Color });
	TriangleBuffer.AppendVertex({ RVec4(v1), Color });
	TriangleBuffer.AppendVertex({ RVec4(v2), Color });
}

void RDebugRenderer::Render()
{
	GRenderer.SetBlendState(BlendState::AlphaBlending);
	LineBuffer.Render();
	TriangleBuffer.Render();
	GRenderer.SetBlendState(BlendState::Opaque);
}

void RDebugRenderer::Reset()
{
	LineBuffer.Reset();
	TriangleBuffer.Reset();
}

void RDebugRenderer::DrawHemisphereRange(const RVec3 Center, float Radius, float Start, float End, const RColor& Color /*= GDebugRenderer.GetPrimitiveColor()*/, int Segment /*= 12*/)
{
	int StartIdx = RMath::Max(int(Start * Segment), 1);
	int EndIdx = int(End * Segment);

	// Horizontal steps
	for (int i = 0; i < Segment; i++)
	{
		for (int j = StartIdx; j <= EndIdx; j++)
		{
			float y = cosf(PI * j / Segment) * Radius + Center.Y();
			float r = sinf(PI * j / Segment) * Radius;

			float x0 = sinf(2.0f * PI * i / Segment) * r + Center.X();
			float z0 = cosf(2.0f * PI * i / Segment) * r + Center.Z();

			int i1 = (i + 1) % Segment;
			float x1 = sinf(2.0f * PI * i1 / Segment) * r + Center.X();
			float z1 = cosf(2.0f * PI * i1 / Segment) * r + Center.Z();

			RVertexType::PositionColor v0 = { RVec4(x0, y, z0), Color };
			RVertexType::PositionColor v1 = { RVec4(x1, y, z1), Color };
			LineBuffer.AppendVertex(v0);
			LineBuffer.AppendVertex(v1);
		}
	}

	StartIdx = RMath::Max(int(Start * Segment), 0);

	// Vertical lines
	for (int i = 0; i < Segment; i++)
	{
		for (int j = StartIdx; j < EndIdx; j++)
		{
			float y0 = cosf(PI * j / Segment) * Radius + Center.Y();
			float r0 = sinf(PI * j / Segment) * Radius;

			int j1 = j + 1;
			float y1 = cosf(PI * j1 / Segment) * Radius + Center.Y();
			float r1 = sinf(PI * j1 / Segment) * Radius;

			float x0 = sinf(2.0f * PI * i / Segment) * r0 + Center.X();
			float z0 = cosf(2.0f * PI * i / Segment) * r0 + Center.Z();

			float x1 = sinf(2.0f * PI * i / Segment) * r1 + Center.X();
			float z1 = cosf(2.0f * PI * i / Segment) * r1 + Center.Z();

			RVertexType::PositionColor v0 = { RVec4(x0, y0, z0), Color };
			RVertexType::PositionColor v1 = { RVec4(x1, y1, z1), Color };
			LineBuffer.AppendVertex(v0);
			LineBuffer.AppendVertex(v1);
		}
	}
}
