//=============================================================================
// RSkybox.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RSkybox.h"

RSkybox::RSkybox()
	: m_SkyboxTexture(nullptr), m_SkyboxIL(nullptr)
{

}

void RSkybox::CreateSkybox(const char* skyTextureName)
{
	// Load skybox texture
	CreateSkybox(RResourceManager::Instance().FindTexture(skyTextureName));
}

void RSkybox::CreateSkybox(RTexture* skyTexture)
{
	m_SkyboxTexture = skyTexture;

	// Create skybox buffer
	SKYBOX_VERTEX skyboxVertex[] = 
	{
		{ RVec3(-0.5f, -0.5f, -0.5f) },
		{ RVec3(-0.5f,  0.5f, -0.5f) },
		{ RVec3( 0.5f,  0.5f, -0.5f) },
		{ RVec3( 0.5f, -0.5f, -0.5f) },

		{ RVec3(-0.5f, -0.5f,  0.5f) },
		{ RVec3(-0.5f,  0.5f,  0.5f) },
		{ RVec3( 0.5f,  0.5f,  0.5f) },
		{ RVec3( 0.5f, -0.5f,  0.5f) },
	};

	UINT32 skyboxIndex[] =
	{
		0, 2, 1, 0, 3, 2,
		0, 1, 5, 0, 5, 4,
		3, 6, 2, 3, 7, 6,
		4, 5, 6, 4, 6, 7,
		1, 2, 6, 1, 6, 5,
		0, 4, 7, 0, 7, 3,
	};

	m_SkyboxMesh.CreateVertexBuffer(skyboxVertex, sizeof(SKYBOX_VERTEX), sizeof(skyboxVertex) / sizeof(SKYBOX_VERTEX));
	m_SkyboxMesh.CreateIndexBuffer(skyboxIndex, sizeof(UINT32), sizeof(skyboxIndex) / sizeof(UINT32));

	m_SkyboxShader = RShaderManager::Instance().GetShaderResource("Skybox");

	if (!m_SkyboxShader)
	{
		OutputDebugStringA("*** WARNING: Unable to find shader \'Skybox\', skybox will not be rendered properly.\n");
	}

	m_SkyboxIL = RRenderer.GetInputLayout(SKYBOX_VERTEX::GetTypeName());
}

void RSkybox::Release()
{
	m_SkyboxMesh.Release();
}

void RSkybox::Draw()
{
	if (m_SkyboxShader && m_SkyboxTexture)
	{
		RRenderer.D3DImmediateContext()->IASetInputLayout(m_SkyboxIL);
		RRenderer.D3DImmediateContext()->PSSetShaderResources(0, 1, m_SkyboxTexture->GetPtrSRV());
		m_SkyboxShader->Bind();
		m_SkyboxMesh.Draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
}