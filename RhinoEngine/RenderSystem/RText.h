//=============================================================================
// RText.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "RRenderSystem.h"
#include "RMeshElement.h"

class RText : public IOverlayRenderable
{
public:
	RText();
	virtual ~RText();

	void Initialize(RTexture* fontTexture = nullptr, UINT rows = 0, UINT columns = 0);
	void AddText(const char* text, UINT start_x, UINT start_y, const RColor& fg, const RColor& bg);
	void SetText(const char* text, const RColor& fg = RColor(1, 1, 1), const RColor& bg = RColor(0, 0, 0, 1));

	/// Render the text to screen
	virtual void Render() override;

protected:
	std::vector<RVertexType::Font>	m_Vertices;
	RMeshRenderBuffer				m_VertexBuffer;
	RTexture*						m_FontTexture;
	RShader*						m_FontShader;
	UINT							m_Rows, m_Columns;
	bool							m_bNeedUpdateBuffer;
};

