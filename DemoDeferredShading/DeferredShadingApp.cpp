//=============================================================================
// DeferredShadingApp.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "DeferredShadingApp.h"


DeferredShadingApp::DeferredShadingApp()
{
}


DeferredShadingApp::~DeferredShadingApp()
{
	for (int i = 0; i < DeferredBuffer_Count; i++)
	{
		m_DeferredBuffers[i].Release();
	}
	m_DepthBuffer.Release();

	m_Scene.Release();
	RShaderManager::Instance().UnloadAllShaders();
	RResourceManager::Instance().Destroy();
}

bool DeferredShadingApp::Initialize()
{
	RShaderManager::Instance().LoadShaders("../Shaders");
	RResourceManager::Instance().Initialize();

	RMesh* mesh = RResourceManager::Instance().LoadFbxMesh("../Assets/AO_Scene.FBX");
	m_MeshObj.SetMesh(mesh);

	m_Scene.Initialize();
	m_Camera.SetPosition(RVec3(0, 0, 0));
	m_Camera.SetupView(65.0f, RRenderer.AspectRatio(), 1.0f, 10000.0f);
	m_CamPitch = 0;
	m_CamYaw = 0;

	RRenderer.SetSamplerState(0, SamplerState_Texture);
	RRenderer.SetSamplerState(2, SamplerState_ShadowDepthComparison);

	// Update light constant buffer
	SHADER_LIGHT_BUFFER cbLight;
	ZeroMemory(&cbLight, sizeof(cbLight));

	// Setup ambient color
	cbLight.HighHemisphereAmbientColor = RVec4(0.1f, 0.1f, 0.1f, 1.0f);
	cbLight.LowHemisphereAmbientColor = RVec4(0.2f, 0.2f, 0.2f, 1.0f);

	m_Scene.cbLight.UpdateContent(&cbLight);

	for (int i = 0; i < DeferredBuffer_Count; i++)
	{
		m_DeferredBuffers[i] = CreateRenderTarget();
	}
	m_DepthBuffer = CreateDepthStencilBuffer();

	return true;
}

void DeferredShadingApp::UpdateScene(const RTimer& timer)
{
	if (RInput.GetBufferedKeyState(VK_RBUTTON) == BKS_Pressed)
	{
		RInput.HideCursor();
		RInput.LockCursor();
	}

	if (RInput.GetBufferedKeyState(VK_RBUTTON) == BKS_Released)
	{
		RInput.ShowCursor();
		RInput.UnlockCursor();
	}

	if (RInput.IsKeyDown(VK_RBUTTON))
	{
		int dx, dy;
		RInput.GetCursorRelPos(dx, dy);
		if (dx || dy)
		{
			m_CamYaw += (float)dx / 200.0f;
			m_CamPitch += (float)dy / 200.0f;
			m_CamPitch = max(-PI / 2, min(PI / 2, m_CamPitch));
		}
	}

	float camSpeed = 100.0f;
	if (RInput.IsKeyDown(VK_LSHIFT))
		camSpeed *= 10.0f;
	RVec3 moveVec(0.0f, 0.0f, 0.0f);
	if (RInput.IsKeyDown('W'))
		moveVec += RVec3(0.0f, 0.0f, 1.0f) * timer.DeltaTime() * camSpeed;
	if (RInput.IsKeyDown('S'))
		moveVec -= RVec3(0.0f, 0.0f, 1.0f) * timer.DeltaTime() * camSpeed;
	if (RInput.IsKeyDown('A'))
		moveVec -= RVec3(1.0f, 0.0f, 0.0f) * timer.DeltaTime() * camSpeed;
	if (RInput.IsKeyDown('D'))
		moveVec += RVec3(1.0f, 0.0f, 0.0f) * timer.DeltaTime() * camSpeed;

	RVec3 camPos = m_Camera.GetPosition();
	RMatrix4 cameraMatrix = RMatrix4::CreateXAxisRotation(m_CamPitch * 180 / PI) * RMatrix4::CreateYAxisRotation(m_CamYaw * 180 / PI);

	cameraMatrix.SetTranslation(camPos);
	m_Camera.SetTransform(cameraMatrix);
	m_Camera.TranslateLocal(moveVec);

	RMatrix4 viewMatrix = m_Camera.GetViewMatrix();
	RMatrix4 projMatrix = m_Camera.GetProjectionMatrix();

	// Update scene constant buffer
	SHADER_SCENE_BUFFER cbScene;

	cbScene.viewMatrix = viewMatrix;
	cbScene.projMatrix = projMatrix;
	cbScene.viewProjMatrix = viewMatrix * projMatrix;
	cbScene.cameraPos = m_Camera.GetPosition();

	m_Scene.cbScene.UpdateContent(&cbScene);
	m_Scene.cbScene.ApplyToShaders();
}

void DeferredShadingApp::RenderScene()
{
	RRenderer.Clear();

	SHADER_OBJECT_BUFFER cbObject;
	cbObject.worldMatrix = m_MeshObj.GetNodeTransform();

	m_Scene.cbPerObject.UpdateContent(&cbObject);
	m_Scene.cbPerObject.ApplyToShaders();

	m_MeshObj.Draw();

	RRenderer.Present();
}

void DeferredShadingApp::OnResize(int width, int height)
{

}

DeferredRenderBuffer DeferredShadingApp::CreateRenderTarget()
{
	DeferredRenderBuffer rb;

	D3D11_TEXTURE2D_DESC renderTargetTextureDesc;
	renderTargetTextureDesc.Width = RRenderer.GetClientWidth();
	renderTargetTextureDesc.Height = RRenderer.GetClientHeight();
	renderTargetTextureDesc.MipLevels = 1;
	renderTargetTextureDesc.ArraySize = 1;
	renderTargetTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	renderTargetTextureDesc.SampleDesc.Count = 1;
	renderTargetTextureDesc.SampleDesc.Quality = 0;
	renderTargetTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	renderTargetTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	renderTargetTextureDesc.CPUAccessFlags = 0;
	renderTargetTextureDesc.MiscFlags = 0;

	RRenderer.D3DDevice()->CreateTexture2D(&renderTargetTextureDesc, 0, &rb.Buffer);
	RRenderer.D3DDevice()->CreateRenderTargetView(rb.Buffer, 0, &rb.View);

	D3D11_SHADER_RESOURCE_VIEW_DESC rtsrvDesc;
	rtsrvDesc.Format = renderTargetTextureDesc.Format;
	rtsrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	rtsrvDesc.Texture2D.MostDetailedMip = 0;
	rtsrvDesc.Texture2D.MipLevels = 1;

	RRenderer.D3DDevice()->CreateShaderResourceView(rb.Buffer, &rtsrvDesc, &rb.SRV);

	return rb;
}

DepthStencilBuffer DeferredShadingApp::CreateDepthStencilBuffer()
{
	DepthStencilBuffer db;

	D3D11_TEXTURE2D_DESC renderTargetTextureDesc;
	renderTargetTextureDesc.Width = RRenderer.GetClientWidth();
	renderTargetTextureDesc.Height = RRenderer.GetClientHeight();
	renderTargetTextureDesc.MipLevels = 1;
	renderTargetTextureDesc.ArraySize = 1;
	renderTargetTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	renderTargetTextureDesc.SampleDesc.Count = 1;
	renderTargetTextureDesc.SampleDesc.Quality = 0;
	renderTargetTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	renderTargetTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	renderTargetTextureDesc.CPUAccessFlags = 0;
	renderTargetTextureDesc.MiscFlags = 0;

	RRenderer.D3DDevice()->CreateTexture2D(&renderTargetTextureDesc, 0, &db.Buffer);
	RRenderer.D3DDevice()->CreateDepthStencilView(db.Buffer, 0, &db.View);

	return db;
}