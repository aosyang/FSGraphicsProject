#ifndef _RSKYBOX_H
#define _RSKYBOX_H

#include "Rhino.h"

struct RShader;

class RSkybox
{
public:
	RSkybox();

	void CreateSkybox(const wchar_t* skyTextureName);
	void Release();

	void Draw();

private:
	RMeshElement				m_SkyboxMesh;
	ID3D11ShaderResourceView*	m_SkyboxTexture;
	ID3D11InputLayout*			m_SkyboxIL;
	RShader*					m_SkyboxShader;
};

#endif
