//=============================================================================
// RSkybox.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#ifndef _RSKYBOX_H
#define _RSKYBOX_H

struct RShader;

class RSkybox
{
public:
	RSkybox();

	void CreateSkybox(const char* skyTextureName);
	void CreateSkybox(RTexture* skyTexture);
	void Release();

	void Draw();

private:
	RMeshElement				m_SkyboxMesh;
	RTexture*					m_SkyboxTexture;
	ID3D11InputLayout*			m_SkyboxIL;
	RShader*					m_SkyboxShader;
};

#endif
