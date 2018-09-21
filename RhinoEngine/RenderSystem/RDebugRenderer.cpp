//=============================================================================
// RDebugRenderer.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"
#include "RDebugRenderer.h"


RDebugRenderer::RDebugRenderer()
	: m_PrimitiveInputLayout	(nullptr),
	  m_ColorShader				(nullptr),
	  m_PrimitiveColor			(0.0f, 1.0f, 0.0f),
	  m_bDirtyBuffer			(true)
{
}


RDebugRenderer::~RDebugRenderer()
{
}

void RDebugRenderer::Initialize(int maxVertexCount)
{
	m_ColorShader = RShaderManager::Instance().GetShaderResource("Color");
	m_PrimitiveInputLayout = RVertexDeclaration::Instance().GetInputLayout<RVertexType::PositionColor>();
	m_PrimitiveMeshBuffer.CreateVertexBuffer(nullptr, sizeof(RVertexType::PositionColor), maxVertexCount, m_PrimitiveInputLayout, true);
}

void RDebugRenderer::Release()
{
	m_PrimitiveMeshBuffer.Release();
}

void RDebugRenderer::SetPrimitiveColor(const RColor& color)
{
	m_PrimitiveColor = color;
}

void RDebugRenderer::DrawLine(const RVec3& start, const RVec3& end, const RColor& color1, const RColor& color2)
{
	m_PrimitiveVertices.push_back({ RVec4(start), color1 });
	m_PrimitiveVertices.push_back({ RVec4(end), color2 });

	m_bDirtyBuffer = true;
}

void RDebugRenderer::DrawAabb(const RAabb& aabb, const RColor& color)
{
	RVec3 cornerPoints[] =
	{
		RVec3(aabb.pMin.X(), aabb.pMin.Y(), aabb.pMin.Z()),
		RVec3(aabb.pMin.X(), aabb.pMin.Y(), aabb.pMax.Z()),
		RVec3(aabb.pMin.X(), aabb.pMax.Y(), aabb.pMax.Z()),
		RVec3(aabb.pMin.X(), aabb.pMax.Y(), aabb.pMin.Z()),

		RVec3(aabb.pMax.X(), aabb.pMin.Y(), aabb.pMin.Z()),
		RVec3(aabb.pMax.X(), aabb.pMin.Y(), aabb.pMax.Z()),
		RVec3(aabb.pMax.X(), aabb.pMax.Y(), aabb.pMax.Z()),
		RVec3(aabb.pMax.X(), aabb.pMax.Y(), aabb.pMin.Z()),
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
			RVec4(cornerPoints[wiredCubeIdx[i]]),
			RColor(0.0f, 1.0f, 0.0f),
		};
		m_PrimitiveVertices.push_back(v);
	}
}

void RDebugRenderer::DrawFrustum(const RFrustum& frustum, const RColor& color)
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
		m_PrimitiveVertices.push_back(v);
	}
}

void RDebugRenderer::DrawSphere(const RVec3& center, float radius, const RColor& color, int segment)
{
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
			m_PrimitiveVertices.push_back(v0);
			m_PrimitiveVertices.push_back(v1);
		}
	}

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
			m_PrimitiveVertices.push_back(v0);
			m_PrimitiveVertices.push_back(v1);
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

void RDebugRenderer::Render()
{
	if (m_bDirtyBuffer)
	{
		m_PrimitiveMeshBuffer.UpdateDynamicVertexBuffer(m_PrimitiveVertices.data(), sizeof(RVertexType::PositionColor), (UINT)m_PrimitiveVertices.size());
		m_bDirtyBuffer = false;
	}

	if (m_PrimitiveMeshBuffer.GetVertexCount() != 0)
	{
		SHADER_OBJECT_BUFFER cbObject;
		cbObject.worldMatrix = RMatrix4::IDENTITY;

		RConstantBuffers::cbPerObject.UpdateBufferData(&cbObject);
		RConstantBuffers::cbPerObject.BindBuffer();

		m_ColorShader->Bind();
		GRenderer.D3DImmediateContext()->IASetInputLayout(m_PrimitiveInputLayout);
		m_PrimitiveMeshBuffer.Draw(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	}
}

void RDebugRenderer::Reset()
{
	m_PrimitiveVertices.clear();
	m_bDirtyBuffer = true;
}