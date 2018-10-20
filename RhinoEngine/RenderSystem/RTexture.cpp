//=============================================================================
// RTexture.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RTexture.h"

const std::string RTexture::InternalTextureName("[Internal]");

RTexture::RTexture(const string& Path)
	: RResourceBase(RT_Texture, Path), m_SRV(nullptr), m_Width(0), m_Height(0)
{
}

RTexture::RTexture(ID3D11ShaderResourceView* srv)
	: RResourceBase(RT_Texture, InternalTextureName), m_SRV(srv), m_Width(0), m_Height(0)
{
}

RTexture::~RTexture()
{
	SAFE_RELEASE(m_SRV);
}
