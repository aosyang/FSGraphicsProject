//=============================================================================
// RText.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "RText.h"

#include "Resource/RResourceManager.h"
#include "RVertexDeclaration.h"
#include "RShaderManager.h"
#include "RTexture.h"


RText::RText()
	: m_FontShader(nullptr), m_bNeedUpdateBuffer(true)
{
	GRenderer.RegisterOverlayRenderable(this);
}

RText::~RText()
{
	GRenderer.UnregisterOverlayRenderable(this);
}

void RText::Initialize(RTexture* fontTexture, UINT rows, UINT columns)
{
	if (!fontTexture)
	{
		m_FontTexture = RResourceManager::Instance().LoadResource<RTexture>("/Fonts/Fixedsys_9c.DDS", EResourceLoadMode::Immediate);
		m_Rows = m_Columns = 16;
	}
	else
	{
		m_FontTexture = fontTexture;
		m_Rows = rows;
		m_Columns = columns;
	}

	ID3D11InputLayout* pInputLayout = RVertexDeclaration::Instance().GetInputLayout<RVertexType::Font>();
	m_VertexBuffer.CreateVertexBuffer(nullptr, sizeof(RVertexType::Font), 65536, pInputLayout, EPrimitiveTopology::TriangleList, true);

	m_FontShader = GShaderManager.FindShaderByName("Font");
}

void RText::AddText(const char* text, UINT start_x, UINT start_y, const RColor& fg, const RColor& bg)
{
	float glyph_width = (float)m_FontTexture->GetWidth() / m_Columns;
	float glyph_height = (float)m_FontTexture->GetHeight() / m_Rows;

	float x0 = glyph_width * start_x, y0 = glyph_height * start_y;

	bool isEscape = false;

	for (UINT i = 0; text[i]; i++)
	{
		unsigned char ch = text[i];

		if (isEscape)
		{
			if (ch == 'n')
			{
				x0 = glyph_width * start_x;
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
				x0 = glyph_width * start_x;
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

		RVertexType::Font v[6] =
		{
			{ RVertexType::Vec4Data(x0, y0, 0, 1), fg, bg, RVertexType::Vec2Data(u0, v0) },
			{ RVertexType::Vec4Data(x1, y0, 0, 1), fg, bg, RVertexType::Vec2Data(u1, v0) },
			{ RVertexType::Vec4Data(x0, y1, 0, 1), fg, bg, RVertexType::Vec2Data(u0, v1) },

			{ RVertexType::Vec4Data(x0, y1, 0, 1), fg, bg, RVertexType::Vec2Data(u0, v1) },
			{ RVertexType::Vec4Data(x1, y0, 0, 1), fg, bg, RVertexType::Vec2Data(u1, v0) },
			{ RVertexType::Vec4Data(x1, y1, 0, 1), fg, bg, RVertexType::Vec2Data(u1, v1) },
		};

		for (int j = 0; j < 6; j++)
		{
			m_Vertices.push_back(v[j]);
		}

		x0 += glyph_width;
	}

	m_bNeedUpdateBuffer = true;
}

void RText::SetText(const char* text, const RColor& fg, const RColor& bg)
{
	m_Vertices.clear();

	AddText(text, 0, 0, fg, bg);
}

void RText::Render()
{
	if (m_bNeedUpdateBuffer)
	{
		m_VertexBuffer.UpdateDynamicVertexBuffer(m_Vertices.data(), sizeof(RVertexType::Font), (UINT)m_Vertices.size());
		m_bNeedUpdateBuffer = false;
	}

	GRenderer.SetBlendState(BlendState::AlphaBlending);
	m_FontShader->Bind();
	GRenderer.D3DImmediateContext()->PSSetShaderResources(0, 1, m_FontTexture->GetPtrSRV());
	m_VertexBuffer.Draw();
}
