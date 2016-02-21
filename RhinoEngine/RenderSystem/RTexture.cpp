//=============================================================================
// RTexture.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RTexture.h"

RTexture::RTexture(string path)
	: RBaseResource(RT_Texture, path), m_SRV(nullptr)
{
}

RTexture::RTexture(string path, ID3D11ShaderResourceView* srv)
	: RBaseResource(RT_Texture, path), m_SRV(srv)
{
}

RTexture::~RTexture()
{
	SAFE_RELEASE(m_SRV);
}