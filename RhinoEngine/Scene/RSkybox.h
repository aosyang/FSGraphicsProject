//=============================================================================
// RSkybox.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "Core/CoreTypes.h"
#include "RenderSystem/RMeshElement.h"

struct RShader;
class RTexture;

class RSkybox
{
public:
	RSkybox();

	void CreateSkybox(const std::string& skyTextureName);
	void CreateSkybox(RTexture* skyTexture);
	void Release();

	void Draw();

private:
	RMeshRenderBuffer			m_SkyboxMesh;
	RTexture*					m_SkyboxTexture;
	ID3D11InputLayout*			m_SkyboxIL;
	RShader*					m_SkyboxShader;
};

