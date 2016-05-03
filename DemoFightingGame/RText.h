//=============================================================================
// RText.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#ifndef _RTEXT_H
#define _RTEXT_H

#include "Rhino.h"

class RText
{
public:
	RText();
	~RText();

	void Initialize(RTexture* fontTexture, UINT rows, UINT columns);
	void SetText(const string& text, const RColor& color=RColor(1, 1, 1));
	void Render();

private:
	RMeshElement	m_VertexBuffer;
	RTexture*		m_FontTexture;
	RShader*		m_FontShader;
	UINT			m_Rows, m_Columns;
};

#endif
