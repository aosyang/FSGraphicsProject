//=============================================================================
// EditorAxis.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "stdafx.h"
#include "Rhino.h"
#include "EditorAxis.h"

EditorAxis::EditorAxis()
{
}

void EditorAxis::Create()
{
	for (int n = 0; n < 3; n++)
	{
		RVertex::PRIMITIVE_VERTEX axisVerts[] =
		{
			{ RVec4(-0.5f, -0.5f, -0.5f, 1.0f), RColor(1.0f, 0.0f, 0.0f) },
			{ RVec4(-0.5f,  0.5f, -0.5f, 1.0f), RColor(1.0f, 0.0f, 0.0f) },
			{ RVec4( 0.5f,  0.5f, -0.5f, 1.0f), RColor(1.0f, 0.0f, 0.0f) },
			{ RVec4( 0.5f, -0.5f, -0.5f, 1.0f), RColor(1.0f, 0.0f, 0.0f) },

			{ RVec4(-0.5f, -0.5f,  0.5f, 1.0f), RColor(1.0f, 0.0f, 0.0f) },
			{ RVec4(-0.5f,  0.5f,  0.5f, 1.0f), RColor(1.0f, 0.0f, 0.0f) },
			{ RVec4( 0.5f,  0.5f,  0.5f, 1.0f), RColor(1.0f, 0.0f, 0.0f) },
			{ RVec4( 0.5f, -0.5f,  0.5f, 1.0f), RColor(1.0f, 0.0f, 0.0f) },
		};

		if (n == AXIS_X)
		{
			for (int i = 0; i < 8; i++)
			{
				if (axisVerts[i].pos.x > 0.0f)
					axisVerts[i].pos.x = 10.0f;
				axisVerts[i].color = RColor(1.0f, 0.0f, 0.0f);
			}
		}
		else if (n == AXIS_Y)
		{
			for (int i = 0; i < 8; i++)
			{
				if (axisVerts[i].pos.y > 0.0f)
					axisVerts[i].pos.y = 10.0f;
				axisVerts[i].color = RColor(0.0f, 1.0f, 0.0f);
			}
		}
		else if (n == AXIS_Z)
		{
			for (int i = 0; i < 8; i++)
			{
				if (axisVerts[i].pos.z > 0.0f)
					axisVerts[i].pos.z = 10.0f;
				axisVerts[i].color = RColor(0.0f, 0.0f, 1.0f);
			}
		}

		UINT32 axisIndices[] =
		{
			0, 1, 2, 0, 2, 3,
			0, 5, 1, 0, 4, 5,
			3, 2, 6, 3, 6, 7,
			4, 6, 5, 4, 7, 6,
			1, 6, 2, 1, 5, 6,
			0, 7, 4, 0, 3, 7,
		};

		ID3D11InputLayout* pInputLayout = RVertexDeclaration::Instance().GetInputLayout(RVertex::PRIMITIVE_VERTEX::GetTypeName());
		m_AxisMeshBuffer[n].CreateVertexBuffer(axisVerts, sizeof(RVertex::PRIMITIVE_VERTEX), sizeof(axisVerts) / sizeof(RVertex::PRIMITIVE_VERTEX), pInputLayout);
		m_AxisMeshBuffer[n].CreateIndexBuffer(axisIndices, sizeof(UINT), sizeof(axisIndices) / sizeof(UINT));

		for (int i = 0; i < 8; i++)
		{
			m_AxisAabb[n].Expand(axisVerts[i].pos.ToVec3());
		}
	}

	m_ColorInputLayout = RVertexDeclaration::Instance().GetInputLayout(RVertex::PRIMITIVE_VERTEX::GetTypeName());
}

void EditorAxis::Release()
{
	for (int i = 0; i < 3; i++)
		m_AxisMeshBuffer[i].Release();
}

void EditorAxis::Draw()
{
	GRenderer.D3DImmediateContext()->IASetInputLayout(m_ColorInputLayout);
	for (int i = 0; i < 3; i++)
		m_AxisMeshBuffer[i].Draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

const RAabb& EditorAxis::GetAabb(int index) const
{
	return m_AxisAabb[index];
}