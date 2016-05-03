//=============================================================================
// RText.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "RText.h"


RText::RText()
	: m_FontShader(nullptr)
{
}

RText::~RText()
{
	m_VertexBuffer.Release();
}

void RText::Initialize(RTexture* fontTexture, UINT rows, UINT columns)
{
	m_FontTexture = fontTexture;
	m_Rows = rows;
	m_Columns = columns;

	ID3D11InputLayout* pInputLayout = RVertexDeclaration::Instance().GetInputLayout(RVertex::FONT_VERTEX::GetTypeName());
	m_VertexBuffer.CreateVertexBuffer(nullptr, sizeof(RVertex::FONT_VERTEX), 65536, pInputLayout, true);

	m_FontShader = RShaderManager::Instance().GetShaderResource("Font");
}

void RText::SetText(const string& text, const RColor& color)
{
	vector<RVertex::FONT_VERTEX> vertices;

	float glyph_width = (float)m_FontTexture->GetWidth() / m_Columns;
	float glyph_height = (float)m_FontTexture->GetHeight() / m_Rows;

	float x0 = 0, y0 = 0;

	bool isEscape = false;

	for (UINT i = 0; i < text.length(); i++)
	{
		unsigned char ch = text[i];

		if (isEscape)
		{
			if (ch == 'n')
			{
				x0 = 0;
				y0 += glyph_height;
			}
			isEscape = false;
			continue;
		}
		else
		{
			if (ch == '\\')
			{
				isEscape = true;
				continue;
			}
			else if (ch == '\n')
			{
				x0 = 0;
				y0 += glyph_height;
				continue;
			}
		}

		float u0 = float(ch % m_Rows) / m_Columns;
		float v0 = float(ch / m_Rows) / m_Rows;
		float u1 = u0 + 1.0f / m_Columns;
		float v1 = v0 + 1.0f / m_Rows;

		float x1 = x0 + glyph_width;
		float y1 = y0 + glyph_height;

		RVertex::FONT_VERTEX v[6] =
		{
			{ RVec4(x0, y0, 0), color, RVec2(u0, v0) },
			{ RVec4(x1, y0, 0), color, RVec2(u1, v0) },
			{ RVec4(x0, y1, 0), color, RVec2(u0, v1) },

			{ RVec4(x0, y1, 0), color, RVec2(u0, v1) },
			{ RVec4(x1, y0, 0), color, RVec2(u1, v0) },
			{ RVec4(x1, y1, 0), color, RVec2(u1, v1) },
		};

		for (int j = 0; j < 6; j++)
		{
			vertices.push_back(v[j]);
		}

		x0 += glyph_width;
	}

	m_VertexBuffer.UpdateDynamicVertexBuffer(vertices.data(), sizeof(RVertex::FONT_VERTEX), vertices.size());
}

void RText::Render()
{
	RRenderer.SetBlendState(Blend_AlphaBlending);
	m_FontShader->Bind();
	RRenderer.D3DImmediateContext()->PSSetShaderResources(0, 1, m_FontTexture->GetPtrSRV());
	m_VertexBuffer.Draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
