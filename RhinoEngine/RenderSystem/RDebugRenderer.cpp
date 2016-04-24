//=============================================================================
// RDebugRenderer.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"
#include "RDebugRenderer.h"


RDebugRenderer::RDebugRenderer()
	: m_PrimitiveInputLayout(nullptr), m_ColorShader(nullptr), m_PrimitiveColor(0.0f, 1.0f, 0.0f), m_bDirtyBuffer(true)
{
}


RDebugRenderer::~RDebugRenderer()
{
}

void RDebugRenderer::Initialize(int maxVertexCount)
{
	m_ColorShader = RShaderManager::Instance().GetShaderResource("Color");
	m_PrimitiveInputLayout = RVertexDeclaration::Instance().GetInputLayout(RVertex::PRIMITIVE_VERTEX::GetTypeName());
	m_PrimitiveMeshBuffer.CreateVertexBuffer(nullptr, sizeof(RVertex::PRIMITIVE_VERTEX), maxVertexCount, m_PrimitiveInputLayout, true);
}

void RDebugRenderer::Release()
{
	m_PrimitiveMeshBuffer.Release();
}

void RDebugRenderer::SetColor(const RColor& color)
{
	m_PrimitiveColor = color;
}

void RDebugRenderer::DrawLine(const RVec3& start, const RVec3& end)
{
	DrawLine(start, end, m_PrimitiveColor, m_PrimitiveColor);
}

void RDebugRenderer::DrawLine(const RVec3& start, const RVec3& end, const RColor& color1, const RColor& color2)
{
	m_PrimitiveVertices.push_back({ RVec4(start), color1 });
	m_PrimitiveVertices.push_back({ RVec4(end), color2 });

	m_bDirtyBuffer = true;
}

void RDebugRenderer::DrawAabb(const RAabb& aabb)
{
	DrawAabb(aabb, m_PrimitiveColor);
}

void RDebugRenderer::DrawAabb(const RAabb& aabb, const RColor& color)
{
	RVec3 cornerPoints[] =
	{
		RVec3(aabb.pMin.x, aabb.pMin.y, aabb.pMin.z),
		RVec3(aabb.pMin.x, aabb.pMin.y, aabb.pMax.z),
		RVec3(aabb.pMin.x, aabb.pMax.y, aabb.pMax.z),
		RVec3(aabb.pMin.x, aabb.pMax.y, aabb.pMin.z),

		RVec3(aabb.pMax.x, aabb.pMin.y, aabb.pMin.z),
		RVec3(aabb.pMax.x, aabb.pMin.y, aabb.pMax.z),
		RVec3(aabb.pMax.x, aabb.pMax.y, aabb.pMax.z),
		RVec3(aabb.pMax.x, aabb.pMax.y, aabb.pMin.z),
	};

	int wiredCubeIdx[] =
	{
		0, 1, 1, 2, 2, 3, 3, 0,
		4, 5, 5, 6, 6, 7, 7, 4,
		0, 4, 1, 5, 2, 6, 3, 7,
	};

	for (int i = 0; i < 24; i++)
	{
		RVertex::PRIMITIVE_VERTEX v =
		{
			RVec4(cornerPoints[wiredCubeIdx[i]]),
			RColor(0.0f, 1.0f, 0.0f),
		};
		m_PrimitiveVertices.push_back(v);
	}
}

void RDebugRenderer::DrawFrustum(const RFrustum& frustum)
{
	DrawFrustum(frustum, m_PrimitiveColor);
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
		RVertex::PRIMITIVE_VERTEX v =
		{
			RVec4(frustum.corners[wiredCubeIdx[i]]),
			color,
		};
		m_PrimitiveVertices.push_back(v);
	}
}

void RDebugRenderer::Render()
{
	if (m_bDirtyBuffer)
	{
		m_PrimitiveMeshBuffer.UpdateDynamicVertexBuffer(m_PrimitiveVertices.data(), sizeof(RVertex::PRIMITIVE_VERTEX), m_PrimitiveVertices.size());
		m_bDirtyBuffer = false;
	}

	RRenderer.Clear(false);

	m_ColorShader->Bind();
	RRenderer.D3DImmediateContext()->IASetInputLayout(m_PrimitiveInputLayout);
	m_PrimitiveMeshBuffer.Draw(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}

void RDebugRenderer::Reset()
{
	m_PrimitiveVertices.clear();
	m_bDirtyBuffer = true;
}