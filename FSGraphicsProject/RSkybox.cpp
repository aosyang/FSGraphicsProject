//=============================================================================
// RSkybox.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "RSkybox.h"

struct SKYBOX_VERTEX
{
	XMFLOAT3 pos;
};

RSkybox::RSkybox()
	: m_SkyboxTexture(nullptr), m_SkyboxIL(nullptr)
{

}

void RSkybox::CreateSkybox(const wchar_t* skyTextureName)
{
	// Load skybox texture
	CreateDDSTextureFromFile(RRenderer.D3DDevice(), skyTextureName, NULL, &m_SkyboxTexture);

	// Create skybox buffer
	SKYBOX_VERTEX skyboxVertex[] = 
	{
		{ XMFLOAT3(-0.5f, -0.5f, -0.5f) },
		{ XMFLOAT3(-0.5f,  0.5f, -0.5f) },
		{ XMFLOAT3( 0.5f,  0.5f, -0.5f) },
		{ XMFLOAT3( 0.5f, -0.5f, -0.5f) },

		{ XMFLOAT3(-0.5f, -0.5f,  0.5f) },
		{ XMFLOAT3(-0.5f,  0.5f,  0.5f) },
		{ XMFLOAT3( 0.5f,  0.5f,  0.5f) },
		{ XMFLOAT3( 0.5f, -0.5f,  0.5f) },
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

	// Create input layout
	D3D11_INPUT_ELEMENT_DESC vertDesc[] =
	{
		{ "POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	RRenderer.D3DDevice()->CreateInputLayout(vertDesc, 1, m_SkyboxShader->VS_Bytecode, m_SkyboxShader->VS_BytecodeSize, &m_SkyboxIL);
}

void RSkybox::Release()
{
	m_SkyboxMesh.Release();
	SAFE_RELEASE(m_SkyboxTexture);
	SAFE_RELEASE(m_SkyboxIL);
}

void RSkybox::Draw()
{
	RRenderer.D3DImmediateContext()->IASetInputLayout(m_SkyboxIL);
	RRenderer.D3DImmediateContext()->PSSetShaderResources(0, 1, &m_SkyboxTexture);
	m_SkyboxShader->Bind();
	m_SkyboxMesh.Draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}