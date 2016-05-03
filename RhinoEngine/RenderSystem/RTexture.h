//=============================================================================
// RTexture.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#ifndef _RTEXTURE_H
#define _RTEXTURE_H

class RTexture : public RBaseResource
{
	friend class RResourceManager;
public:
	RTexture(string path);
	RTexture(string path, ID3D11ShaderResourceView* srv);
	~RTexture();

	ID3D11ShaderResourceView* GetSRV() { return m_SRV; }
	ID3D11ShaderResourceView** GetPtrSRV() { return &m_SRV; }
	UINT GetWidth() const { return m_Width; }
	UINT GetHeight() const { return m_Height; }

private:
	ID3D11ShaderResourceView*	m_SRV;
	UINT						m_Width, m_Height;
};

#endif
