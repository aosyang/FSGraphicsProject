//=============================================================================
// RTexture.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RTexture.h"

RTexture::RTexture(string path)
	: RBaseResource(RT_Texture, path), m_SRV(nullptr), m_Width(0), m_Height(0)
{
}

RTexture::RTexture(string path, ID3D11ShaderResourceView* srv)
	: RBaseResource(RT_Texture, path), m_SRV(srv), m_Width(0), m_Height(0)
{
}

RTexture::~RTexture()
{
	SAFE_RELEASE(m_SRV);
}