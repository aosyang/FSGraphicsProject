//=============================================================================
// EditorAxis.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "stdafx.h"
#include "Rhino.h"
#include "EditorAxis.h"

EditorAxis::EditorAxis(const RConstructingParams& Params)
	: RSceneObject(RConstructingParams(Params.Scene, Params.Flags | (CF_InternalObject | CF_NoSerialization)))
{
	// Make editor axis invisible to user and never serialize it

	Create();

	m_ColorShader = RShaderManager::Instance().GetShaderResource("Color");
}

EditorAxis::~EditorAxis()
{
	Release();
}

void EditorAxis::Create()
{
	for (int n = 0; n < 3; n++)
	{
		RVertexType::PositionColor axisVerts[] =
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

		ID3D11InputLayout* pInputLayout = RVertexDeclaration::Instance().GetInputLayout<RVertexType::PositionColor>();
		m_AxisMeshBuffer[n].CreateVertexBuffer(axisVerts, sizeof(RVertexType::PositionColor), sizeof(axisVerts) / sizeof(RVertexType::PositionColor), pInputLayout);
		m_AxisMeshBuffer[n].CreateIndexBuffer(axisIndices, sizeof(UINT), sizeof(axisIndices) / sizeof(UINT));

		for (int i = 0; i < 8; i++)
		{
			m_AxisAabb[n].Expand(RVec3((float*)&axisVerts[i].pos));
		}
	}

	m_ColorInputLayout = RVertexDeclaration::Instance().GetInputLayout<RVertexType::PositionColor>();
}

void EditorAxis::Release()
{
	for (int i = 0; i < 3; i++)
	{
		m_AxisMeshBuffer[i].Release();
	}
}

void EditorAxis::Draw()
{
	if (m_ColorShader)
	{
		m_ColorShader->Bind();

		GRenderer.D3DImmediateContext()->IASetInputLayout(m_ColorInputLayout);
		for (int i = 0; i < 3; i++)
		{
			m_AxisMeshBuffer[i].Draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		}
	}
}

const RAabb& EditorAxis::GetLocalAabb(int index) const
{
	return m_AxisAabb[index];
}