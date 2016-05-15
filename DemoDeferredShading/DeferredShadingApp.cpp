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
	m_cbDeferredPointLight.Release();

	for (int i = 0; i < RasterizerState_Count; i++)
	{
		m_RasterizerStates[i]->Release();
	}

	for (int i = 0; i < DeferredBuffer_Count; i++)
	{
		m_DeferredBuffers[i].Release();
	}
	m_ScenePassBuffer.Release();
	m_DepthBuffer.Release();

	m_DebugRenderer.Release();
	m_Scene.Release();

	m_PostProcessor.Release();
	RShaderManager::Instance().UnloadAllShaders();
	RResourceManager::Instance().Destroy();
}

bool DeferredShadingApp::Initialize()
{
	RShaderManager::Instance().LoadShaders("../Shaders");
	RResourceManager::Instance().Initialize();
	m_PostProcessor.Initialize();

	m_Scene.Initialize();
	m_Scene.LoadFromFile("../Assets/DeferredScene.rmap");

	m_Camera.SetPosition(RVec3(-375, 1385, 1200));
	m_Camera.SetupView(65.0f, RRenderer.AspectRatio(), 1.0f, 10000.0f);
	m_CamPitch = 0.65f;
	m_CamYaw = -3.135f;

	RRenderer.SetSamplerState(0, SamplerState_Texture);
	RRenderer.SetSamplerState(2, SamplerState_ShadowDepthComparison);

	m_DeferredBuffers[DB_Color]				= CreateRenderTarget(DXGI_FORMAT_R8G8B8A8_UNORM);
	m_DeferredBuffers[DB_Position]			= CreateRenderTarget(DXGI_FORMAT_R32G32B32A32_FLOAT);
	m_DeferredBuffers[DB_WorldSpaceNormal]	= CreateRenderTarget(DXGI_FORMAT_R32G32B32A32_FLOAT);
	m_DeferredBuffers[DB_ViewSpaceNormal]	= CreateRenderTarget(DXGI_FORMAT_R32G32B32A32_FLOAT);
	m_ScenePassBuffer						= CreateRenderTarget(DXGI_FORMAT_R32G32B32A32_FLOAT);
	m_DepthBuffer = CreateDepthStencilBuffer();

	m_cbDeferredPointLight.Initialize();

	for (int i = 0; i < MAX_LIGHT_COUNT; i++)
	{
		m_PointLights[i].pos = RVec3(Math::RandF(-1500, 750), Math::RandF(50, 900), Math::RandF(-1850, 300));
		m_PointLights[i].r = Math::RandF(50, 200);
		//m_PointLights[i].color = RColor(1, 1, 1);
		m_PointLights[i].color = RColor(Math::RandF(), Math::RandF(), Math::RandF());
		m_PointLights[i].sin_factor = RVec3(Math::RandF(0, 5), 0, Math::RandF(0, 5));
		m_PointLights[i].sin_offset = RVec3(Math::RandF(0, 5), 0, Math::RandF(0, 5));
	}

	CD3D11_RASTERIZER_DESC rastDesc = CD3D11_RASTERIZER_DESC(D3D11_DEFAULT);
	RRenderer.D3DDevice()->CreateRasterizerState(&rastDesc, &m_RasterizerStates[RS_Default]);

	rastDesc.ScissorEnable = true;
	RRenderer.D3DDevice()->CreateRasterizerState(&rastDesc, &m_RasterizerStates[RS_Scissor]);

	m_DebugRenderer.Initialize();

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
	if (RInput.IsKeyDown('Q'))
		moveVec -= RVec3(0.0f, 1.0f, 0.0f) * timer.DeltaTime() * camSpeed;
	if (RInput.IsKeyDown('E'))
		moveVec += RVec3(0.0f, 1.0f, 0.0f) * timer.DeltaTime() * camSpeed;

	RVec3 camPos = m_Camera.GetPosition();
	RMatrix4 cameraMatrix = RMatrix4::CreateXAxisRotation(m_CamPitch * 180 / PI) * RMatrix4::CreateYAxisRotation(m_CamYaw * 180 / PI);

	cameraMatrix.SetTranslation(camPos);
	m_Camera.SetTransform(cameraMatrix);
	m_Camera.TranslateLocal(moveVec);

	RMatrix4 viewMatrix = m_Camera.GetViewMatrix();
	RMatrix4 projMatrix = m_Camera.GetProjectionMatrix();

	// Update scene constant buffer
	SHADER_SCENE_BUFFER cbScene;
	ZeroMemory(&cbScene, sizeof(cbScene));

	cbScene.viewMatrix = viewMatrix;
	cbScene.projMatrix = projMatrix;
	cbScene.viewProjMatrix = viewMatrix * projMatrix;
	cbScene.invProjMatrix = projMatrix.Inverse();
	cbScene.cameraPos = m_Camera.GetPosition();

	m_Scene.cbScene.UpdateContent(&cbScene);
	m_Scene.cbScene.ApplyToShaders();

	// Update light constant buffer
	SHADER_LIGHT_BUFFER cbLight;
	ZeroMemory(&cbLight, sizeof(cbLight));

	// Setup ambient color
	cbLight.HighHemisphereAmbientColor = RVec4(0.5f, 0.5f, 0.4f, 1.0f);
	cbLight.LowHemisphereAmbientColor = RVec4(0.2f, 0.2f, 0.2f, 0.1f);

	cbLight.CameraPos = m_Camera.GetPosition();

	m_Scene.cbLight.UpdateContent(&cbLight);

	// Update material buffer
	SHADER_MATERIAL_BUFFER cbMaterial;
	ZeroMemory(&cbMaterial, sizeof(cbMaterial));

	cbMaterial.SpecularColorAndPower = RVec4(1.0f, 1.0f, 1.0f, 128.0f);
	cbMaterial.GlobalOpacity = 1.0f;
	m_Scene.cbMaterial.UpdateContent(&cbMaterial);
	m_Scene.cbMaterial.ApplyToShaders();

	// Update screen buffer
	SHADER_GLOBAL_BUFFER cbScreen;
	ZeroMemory(&cbScreen, sizeof(cbScreen));

	cbScreen.ScreenSize = RVec2((float)RRenderer.GetClientWidth(), (float)RRenderer.GetClientHeight());
	cbScreen.ClipPlaneNearFar = RVec2(m_Camera.GetNearPlane(), m_Camera.GetFarPlane());

	RMatrix4 texMat = RMatrix4(
		0.5f,	0.0f,	0.0f,	0.0f,
		0.0f,	-0.5f,	0.0f,	0.0f,
		0.0f,	0.0f,	1.0f,	0.0f,
		0.5f,	0.5f,	0.0f,	1.0f
		);
	cbScreen.ViewToTextureSpace = cbScene.projMatrix * texMat;
	cbScreen.UseGammaCorrection = RRenderer.UsingGammaCorrection();

	m_Scene.cbScreen.UpdateContent(&cbScreen);
	m_Scene.cbScreen.ApplyToShaders();


	m_TotalTime = timer.TotalTime();
}

void DeferredShadingApp::RenderScene()
{
#define DISABLE_MRT 0

#if DISABLE_MRT == 0
	ID3D11RenderTargetView* rtvs[] =
	{
		m_DeferredBuffers[0].View,
		m_DeferredBuffers[1].View, 
		m_DeferredBuffers[2].View, 
		m_DeferredBuffers[3].View,
		m_DeferredBuffers[4].View,
	};

	RRenderer.SetRenderTargets(DeferredBuffer_Count, rtvs, m_DepthBuffer.View);
#else
	RRenderer.Clear();
#endif

	RFrustum frustum = m_Camera.GetFrustum();

	RRenderer.Clear(false, RColor(0, 0, 0));
	RRenderer.ClearRenderTarget(m_DeferredBuffers[DB_Color].View, RColor(0.05f, 0.05f, 0.1f, 0.0f));
	RRenderer.ClearRenderTarget(m_DeferredBuffers[DB_Position].View, RColor(0, 0, 0, 1));
	RRenderer.ClearRenderTarget(m_DeferredBuffers[DB_WorldSpaceNormal].View, RColor(0, 0, 0));
	RRenderer.ClearRenderTarget(m_DeferredBuffers[DB_ViewSpaceNormal].View, RColor(0, 0, 0));

	RRenderer.SetBlendState(Blend_Opaque);
	RRenderer.D3DImmediateContext()->RSSetState(m_RasterizerStates[RS_Default]);

#if DISABLE_MRT == 1

	m_Scene.Render(&frustum);

#else
	RRenderer.SetDefferedShading(true);

	m_Scene.Render(&frustum);

	RRenderer.SetDefferedShading(false);

	RRenderer.SetRenderTargets(1, &m_ScenePassBuffer.View, m_DepthBuffer.View);
	RRenderer.Clear(true, RColor(0, 0, 0));

	ID3D11ShaderResourceView* gbufferSRV[DeferredBuffer_Count];
	for (int i=0; i<DeferredBuffer_Count; i++)
	{
		gbufferSRV[i] = m_DeferredBuffers[i].SRV;
	};

	RRenderer.D3DImmediateContext()->PSSetShaderResources(0, DeferredBuffer_Count, gbufferSRV);

	m_PostProcessor.Draw(PPE_DeferredComposition);

	// Render lighting pass
	RRenderer.SetBlendState(Blend_Additive);

	const RMatrix4& viewProjMatrix = m_Camera.GetViewMatrix() * m_Camera.GetProjectionMatrix();

	for (int i = 0; i < MAX_LIGHT_COUNT; i++)
	{
		float x = sinf(m_TotalTime * m_PointLights[i].sin_factor.x + m_PointLights[i].sin_offset.x) * 100.0f;
		float y = sinf(m_TotalTime * m_PointLights[i].sin_factor.y + m_PointLights[i].sin_offset.y) * 100.0f;
		float z = sinf(m_TotalTime * m_PointLights[i].sin_factor.z + m_PointLights[i].sin_offset.z) * 100.0f;
		RVec3 offset = RVec3(x, y, z);

		RVec3 pos = m_PointLights[i].pos + offset;

		RAabb aabb;
		float r = m_PointLights[i].r;
		aabb.pMin = pos - RVec3(r, r, r);
		aabb.pMax = pos + RVec3(r, r, r);

		RVec2 pMin = RVec2(FLT_MAX, FLT_MAX), pMax = RVec2(-FLT_MAX, -FLT_MAX);
		if (aabb.TestPointInsideAabb(m_Camera.GetPosition()))
		{
			pMin.x = 0.0f;
			pMin.y = (float)RRenderer.GetClientHeight();
			pMax.x = (float)RRenderer.GetClientWidth();
			pMax.y = 0.0f;
		}
		else
		{
			RVec3 corners[] =
			{
				RVec3(aabb.pMin.x, aabb.pMin.y, aabb.pMin.z),
				RVec3(aabb.pMin.x, aabb.pMax.y, aabb.pMin.z),
				RVec3(aabb.pMax.x, aabb.pMax.y, aabb.pMin.z),
				RVec3(aabb.pMax.x, aabb.pMin.y, aabb.pMin.z),

				RVec3(aabb.pMin.x, aabb.pMin.y, aabb.pMax.z),
				RVec3(aabb.pMin.x, aabb.pMax.y, aabb.pMax.z),
				RVec3(aabb.pMax.x, aabb.pMax.y, aabb.pMax.z),
				RVec3(aabb.pMax.x, aabb.pMin.y, aabb.pMax.z),
			};

			for (int j = 0; j < 8; j++)
			{
				RVec4 ndc_point = RVec4(corners[j], 1.0f) * viewProjMatrix;
				ndc_point /= ndc_point.w;

				if (ndc_point.x < pMin.x)
					pMin.x = max(-1.0f, ndc_point.x);
				if (ndc_point.x > pMax.x)
					pMax.x = min(1.0f, ndc_point.x);
				if (ndc_point.y < pMin.y)
					pMin.y = max(-1.0f, ndc_point.y);
				if (ndc_point.y > pMax.y)
					pMax.y = min(1.0f, ndc_point.y);
			}

			//m_DebugRenderer.DrawLine(RVec3(pMin.x, pMin.y, 0), RVec3(pMin.x, pMax.y, 0));
			//m_DebugRenderer.DrawLine(RVec3(pMin.x, pMax.y, 0), RVec3(pMax.x, pMax.y, 0));
			//m_DebugRenderer.DrawLine(RVec3(pMax.x, pMax.y, 0), RVec3(pMax.x, pMin.y, 0));
			//m_DebugRenderer.DrawLine(RVec3(pMax.x, pMin.y, 0), RVec3(pMin.x, pMin.y, 0));

			pMin.x = (pMin.x + 1.0f) * 0.5f * RRenderer.GetClientWidth();
			pMin.y = (-pMin.y + 1.0f) * 0.5f * RRenderer.GetClientHeight();
			pMax.x = (pMax.x + 1.0f) * 0.5f * RRenderer.GetClientWidth();
			pMax.y = (-pMax.y + 1.0f) * 0.5f * RRenderer.GetClientHeight();
		}

		if (pMin.x > pMax.x || pMax.y > pMin.y)
			continue;

		if (RCollision::TestAabbInsideFrustum(frustum, aabb))
		{
			//m_DebugRenderer.DrawSphere(pos, m_PointLights[i].r, m_PointLights[i].color);

			RRenderer.D3DImmediateContext()->RSSetState(m_RasterizerStates[RS_Scissor]);

			D3D11_RECT rect;
			rect.left	= (LONG)pMin.x;
			rect.top	= (LONG)pMax.y;
			rect.right	= (LONG)pMax.x;
			rect.bottom	= (LONG)pMin.y;

			RRenderer.D3DImmediateContext()->RSSetScissorRects(1, &rect);

			SHADER_DEFERRED_POINT_LIGHT_BUFFER cbDeferredPointLight;

			cbDeferredPointLight.DeferredPointLight.PosAndRadius = RVec4(pos, m_PointLights[i].r);
			cbDeferredPointLight.DeferredPointLight.Color = RVec4((float*)&m_PointLights[i].color);

			m_cbDeferredPointLight.UpdateContent(&cbDeferredPointLight);
			m_cbDeferredPointLight.ApplyToShaders();

			RRenderer.Clear(false, RColor(0, 0, 0), true);
			m_PostProcessor.Draw(PPE_DeferredPointLightPass);
		}
	}

	// Disable scissor test
	RRenderer.D3DImmediateContext()->RSSetState(m_RasterizerStates[RS_Default]);

	RRenderer.SetRenderTargets();
	RRenderer.SetBlendState(Blend_Opaque);

	RRenderer.D3DImmediateContext()->PSSetShaderResources(0, 1, &m_ScenePassBuffer.SRV);

	RRenderer.Clear(false, RColor(0, 0, 0), true);
	m_PostProcessor.Draw(PPE_ScreenSpaceRayTracing);

	ID3D11ShaderResourceView* nullSRV[DeferredBuffer_Count] = { nullptr };
	RRenderer.D3DImmediateContext()->PSSetShaderResources(0, DeferredBuffer_Count, nullSRV);
#endif

	RRenderer.Clear(false, RColor(0, 0, 0));

	RRenderer.SetBlendState(Blend_Opaque);
	RRenderer.D3DImmediateContext()->RSSetState(m_RasterizerStates[RS_Default]);

	SHADER_OBJECT_BUFFER cbObject;
	cbObject.worldMatrix = RMatrix4::IDENTITY;

	m_Scene.cbPerObject.UpdateContent(&cbObject);
	m_Scene.cbPerObject.ApplyToShaders();

	m_DebugRenderer.Render();
	m_DebugRenderer.Reset();


	RRenderer.Present();
}

void DeferredShadingApp::OnResize(int width, int height)
{
	if (RRenderer.D3DDevice())
	{
		for (int i = 0; i < DeferredBuffer_Count; i++)
		{
			m_DeferredBuffers[i].Release();
		}
		m_DepthBuffer.Release();

		m_Camera.SetAspectRatio((float)width / (float)height);

		m_DeferredBuffers[DB_Color] = CreateRenderTarget(DXGI_FORMAT_R8G8B8A8_UNORM);
		m_DeferredBuffers[DB_Position] = CreateRenderTarget(DXGI_FORMAT_R32G32B32A32_FLOAT);
		m_DeferredBuffers[DB_WorldSpaceNormal] = CreateRenderTarget(DXGI_FORMAT_R32G32B32A32_FLOAT);
		m_DeferredBuffers[DB_ViewSpaceNormal] = CreateRenderTarget(DXGI_FORMAT_R32G32B32A32_FLOAT);
		m_ScenePassBuffer = CreateRenderTarget(DXGI_FORMAT_R32G32B32A32_FLOAT);
		m_DepthBuffer = CreateDepthStencilBuffer();
	}
}

DeferredRenderBuffer DeferredShadingApp::CreateRenderTarget(DXGI_FORMAT format)
{
	DeferredRenderBuffer rb;

	D3D11_TEXTURE2D_DESC renderTargetTextureDesc;
	renderTargetTextureDesc.Width = RRenderer.GetClientWidth();
	renderTargetTextureDesc.Height = RRenderer.GetClientHeight();
	renderTargetTextureDesc.MipLevels = 1;
	renderTargetTextureDesc.ArraySize = 1;
	renderTargetTextureDesc.Format = format;
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