//=============================================================================
// FSGraphicsProjectApp.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "FSGraphicsProjectApp.h"

#include "Color_PS.csh"
#include "Color_VS.csh"

struct COLOR_VERTEX
{
	XMFLOAT4 pos;
	XMFLOAT4 color;
};

FSGraphicsProjectApp::FSGraphicsProjectApp()
	: m_ColorPrimitiveIL(nullptr),
	  m_ColorPixelShader(nullptr), m_ColorVertexShader(nullptr)
{
}


FSGraphicsProjectApp::~FSGraphicsProjectApp()
{
	SAFE_RELEASE(m_ColorPixelShader);
	SAFE_RELEASE(m_ColorVertexShader);

	m_StarMesh.Release();
	SAFE_RELEASE(m_ColorPrimitiveIL);
}

bool FSGraphicsProjectApp::Initialize()
{
	// Create buffer for star mesh
	COLOR_VERTEX starVertex[12];

	for (int i = 0; i < 10; i++)
	{
		float r = (i % 2 == 0) ? 1.0f : 0.5f;
		starVertex[i] = { XMFLOAT4(sinf(DEG_TO_RAD(i * 36)) * r, cosf(DEG_TO_RAD(i * 36)) * r, 0.0f, 1.0f),
			XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) };
	}

	starVertex[10] = { XMFLOAT4(0.0f, 0.0f, -0.2f, 1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) };
	starVertex[11] = { XMFLOAT4(0.0f, 0.0f, 0.2f, 1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) };

	UINT32 starIndex[] = {
		0, 1, 10, 1, 2, 10, 2, 3, 10, 3, 4, 10, 4, 5, 10, 5, 6, 10, 6, 7, 10, 7, 8, 10, 8, 9, 10, 9, 0, 10,
		1, 0, 11, 2, 1, 11, 3, 2, 11, 4, 3, 11, 5, 4, 11, 6, 5, 11, 7, 6, 11, 8, 7, 11, 9, 8, 11, 0, 9, 11, };

	m_StarMesh.CreateVertexBuffer(starVertex, sizeof(COLOR_VERTEX), 12);
	m_StarMesh.CreateIndexBuffer(starIndex, sizeof(UINT32), sizeof(starIndex) / sizeof(UINT32));

	D3D11_INPUT_ELEMENT_DESC colorVertDesc[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	RRenderer.D3DDevice()->CreateInputLayout(colorVertDesc, 2, Color_VS, sizeof(Color_VS), &m_ColorPrimitiveIL);

	RRenderer.D3DDevice()->CreateVertexShader(Color_VS, sizeof(Color_VS), NULL, &m_ColorVertexShader);
	RRenderer.D3DDevice()->CreatePixelShader(Color_PS, sizeof(Color_PS), NULL, &m_ColorPixelShader);

	D3D11_BUFFER_DESC cbPerObjectDesc;
	ZeroMemory(&cbPerObjectDesc, sizeof(cbPerObjectDesc));
	cbPerObjectDesc.ByteWidth = sizeof(XMFLOAT4X4);
	cbPerObjectDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbPerObjectDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbPerObjectDesc.Usage = D3D11_USAGE_DYNAMIC;

	RRenderer.D3DDevice()->CreateBuffer(&cbPerObjectDesc, NULL, &m_cbPerObject);

	D3D11_BUFFER_DESC cbSceneDesc;
	ZeroMemory(&cbSceneDesc, sizeof(cbSceneDesc));
	cbSceneDesc.ByteWidth = sizeof(XMFLOAT4X4);
	cbSceneDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbSceneDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbSceneDesc.Usage = D3D11_USAGE_DYNAMIC;

	RRenderer.D3DDevice()->CreateBuffer(&cbSceneDesc, NULL, &m_cbScene);

	return true;
}

void FSGraphicsProjectApp::UpdateScene(const RTimer& timer)
{
	XMMATRIX viewMatrix = XMMatrixIdentity();
	XMMATRIX projMatrix = XMMatrixPerspectiveFovLH(90.0f, RRenderer.AspectRatio(), 0.1f, 1000.0f);
	XMFLOAT4X4 viewProj;
	XMStoreFloat4x4(&viewProj, viewMatrix * projMatrix);

	D3D11_MAPPED_SUBRESOURCE subres;
	RRenderer.D3DImmediateContext()->Map(m_cbScene, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
	memcpy(subres.pData, &viewProj, sizeof(viewProj));
	RRenderer.D3DImmediateContext()->Unmap(m_cbScene, 0);
}

void FSGraphicsProjectApp::RenderScene()
{
	RRenderer.Clear();

	RRenderer.D3DImmediateContext()->VSSetConstantBuffers(1, 1, &m_cbScene);

	// Set up object world matrix
	XMMATRIX worldMatrix = XMMatrixTranslation(0.0f, 0.0f, 2.0f);
	XMFLOAT4X4 world;
	XMStoreFloat4x4(&world, worldMatrix);

	D3D11_MAPPED_SUBRESOURCE subres;
	RRenderer.D3DImmediateContext()->Map(m_cbPerObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
	memcpy(subres.pData, &world, sizeof(world));
	RRenderer.D3DImmediateContext()->Unmap(m_cbPerObject, 0);

	RRenderer.D3DImmediateContext()->VSSetConstantBuffers(0, 1, &m_cbPerObject);
	RRenderer.D3DImmediateContext()->PSSetShader(m_ColorPixelShader, NULL, 0);
	RRenderer.D3DImmediateContext()->VSSetShader(m_ColorVertexShader, NULL, 0);
	RRenderer.D3DImmediateContext()->IASetInputLayout(m_ColorPrimitiveIL);

	m_StarMesh.Draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	RRenderer.Present();
}
