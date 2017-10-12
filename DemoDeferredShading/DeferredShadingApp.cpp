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
	m_Skybox.Release();
	m_cbDeferredPointLight.Release();
	m_cbSSR.Release();

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
	m_CubeDepthBuffer.Release();

	m_Scene.Release();

	m_PostProcessor.Release();
}

bool DeferredShadingApp::Initialize()
{
	srand((unsigned int)time(nullptr));

	m_PostProcessor.Initialize();

	m_Scene.Initialize();
	m_Scene.LoadFromFile("../Assets/ScriptTestMap.rmap");

	m_Camera.SetPosition(RVec3(-375, 1385, 1200));
	m_Camera.SetupView(65.0f, GRenderer.AspectRatio(), 1.0f, 10000.0f);
	m_CamPitch = 0.65f;
	m_CamYaw = -3.135f;

	GRenderer.SetSamplerState(0, SamplerState_Texture);
	GRenderer.SetSamplerState(2, SamplerState_ShadowDepthComparison);

	CreateGBuffers();
	m_CubeDepthBuffer = CreateCubeDepthBuffer();

	m_cbDeferredPointLight.Initialize();
	m_cbSSR.Initialize();

	int shadowCasterCount = 10;
	for (int i = 0; i < MAX_LIGHT_COUNT; i++)
	{
		m_PointLights[i].pos = RVec3(Math::RandF(-1500, 750), Math::RandF(50, 100), Math::RandF(-1850, 300));
		m_PointLights[i].r = Math::RandF(50, 200);
		//m_PointLights[i].color = RColor(1, 1, 1);
		m_PointLights[i].color = RColor(Math::RandF(), Math::RandF(), Math::RandF());
		m_PointLights[i].sin_factor = RVec3(Math::RandF(0, 1), 0, Math::RandF(0, 1));
		m_PointLights[i].sin_offset = RVec3(Math::RandF(0, 5), 0, Math::RandF(0, 5));
		m_PointLights[i].castShadow = (i < shadowCasterCount);
	}

	CD3D11_RASTERIZER_DESC rastDesc = CD3D11_RASTERIZER_DESC(D3D11_DEFAULT);
	GRenderer.D3DDevice()->CreateRasterizerState(&rastDesc, &m_RasterizerStates[RS_Default]);

	rastDesc.ScissorEnable = true;
	GRenderer.D3DDevice()->CreateRasterizerState(&rastDesc, &m_RasterizerStates[RS_Scissor]);

	m_DebugMenu.Initialize();
	m_DebugMenu.AddBoolMenuItem("Deferred Shading",			&m_EnableDeferredShading);
	m_DebugMenu.AddBoolMenuItem("Render Light Position",	&m_RenderLightPos);
	m_DebugMenu.AddIntMenuItem("Point Light Count",			&m_LightCount);
	m_DebugMenu.AddFloatMenuItem("Point Light Radius",		&m_LightRadius, 1.0f);
	m_DebugMenu.AddBoolMenuItem("Point Light Shadow",		&m_EnablePointLightShadow);
	m_DebugMenu.AddBoolMenuItem("Screen Space Reflection",	&m_EnableSSR);
	m_DebugMenu.AddFloatMenuItem("cb_stride",				&cbSSR.cb_stride);
	m_DebugMenu.AddFloatMenuItem("cb_strideZCutoff",		&cbSSR.cb_strideZCutoff,	0.001f);
	m_DebugMenu.AddFloatMenuItem("cb_zThickness",			&cbSSR.cb_zThickness,		0.00001f);
	m_DebugMenu.AddFloatMenuItem("cb_maxSteps",				&cbSSR.cb_maxSteps,			100.0f);
	m_DebugMenu.AddFloatMenuItem("cb_maxDistance",			&cbSSR.cb_maxDistance,		1.0f);
	m_DebugMenu.AddFloatMenuItem("Ambient Intensity",		&m_AmbientIntensity,		0.01f);
	m_DebugMenu.SetEnabled(false);

	m_EnableDeferredShading = true;
	m_LightCount = 10;
	m_RenderLightPos = false;
	m_EnablePointLightShadow = false;
	m_EnableSSR = false;
	cbSSR.cb_stride = 4.0f;
	cbSSR.cb_strideZCutoff = 0.01f;
	cbSSR.cb_zThickness = 0.0005f;
	cbSSR.cb_maxSteps = 300.0f;
	cbSSR.cb_maxDistance = 100.0f;
	m_AmbientIntensity = 0.2f;
	m_LightRadius = 1.0f;

	m_EnvCube = RResourceManager::Instance().LoadDDSTexture("../Assets/powderpeak.dds");
	m_Skybox.CreateSkybox(RResourceManager::Instance().WrapSRV(m_CubeDepthBuffer.SRV));
	//m_Skybox.CreateSkybox(m_EnvCube);

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

	// Update light constant buffer
	SHADER_LIGHT_BUFFER cbLight;
	ZeroMemory(&cbLight, sizeof(cbLight));

	// Setup ambient color
	cbLight.HighHemisphereAmbientColor = RVec4(0.9f, 1.0f, 1.0f, min(1.0f, max(0.0f, m_AmbientIntensity)));
	cbLight.LowHemisphereAmbientColor = RVec4(0.2f, 0.2f, 0.2f, min(1.0f, max(0.0f, m_AmbientIntensity)));

	cbLight.CameraPos = m_Camera.GetPosition();

	RConstantBuffers::cbLight.UpdateContent(&cbLight);

	// Update material buffer
	SHADER_MATERIAL_BUFFER cbMaterial;
	ZeroMemory(&cbMaterial, sizeof(cbMaterial));

	cbMaterial.SpecularColorAndPower = RVec4(1.0f, 1.0f, 1.0f, 128.0f);
	cbMaterial.GlobalOpacity = 1.0f;
	RConstantBuffers::cbMaterial.UpdateContent(&cbMaterial);
	RConstantBuffers::cbMaterial.BindBuffer();

	// Update screen buffer
	SHADER_GLOBAL_BUFFER cbScreen;
	ZeroMemory(&cbScreen, sizeof(cbScreen));

	cbScreen.ScreenSize = RVec4((float)GRenderer.GetClientWidth(), (float)GRenderer.GetClientHeight(),
								1.0f / (float)GRenderer.GetClientWidth(), 1.0f / (float)GRenderer.GetClientHeight());
	cbScreen.ClipPlaneNearFar = RVec2(m_Camera.GetNearPlane(), m_Camera.GetFarPlane());

	RMatrix4 texMat = RMatrix4(
		0.5f,	0.0f,	0.0f,	0.0f,
		0.0f,	-0.5f,	0.0f,	0.0f,
		0.0f,	0.0f,	1.0f,	0.0f,
		0.5f,	0.5f,	0.0f,	1.0f
		);
	cbScreen.ViewToTextureSpace = m_Camera.GetProjectionMatrix() * texMat;
	cbScreen.UseGammaCorrection = GRenderer.UsingGammaCorrection();
	cbScreen.TotalTime = GEngine.GetTimer().TotalTime();

	RConstantBuffers::cbGlobal.UpdateContent(&cbScreen);
	RConstantBuffers::cbGlobal.BindBuffer();

	m_cbSSR.UpdateContent(&cbSSR);
	m_cbSSR.BindBuffer();

	m_TotalTime = timer.TotalTime();

	if (RInput.GetBufferedKeyState(VK_F5) == BKS_Pressed)
		m_DebugMenu.SetEnabled(!m_DebugMenu.GetEnabled());
	m_DebugMenu.Update();
}

void DeferredShadingApp::RenderScene()
{
	RMatrix4 viewMatrix = m_Camera.GetViewMatrix();
	RMatrix4 projMatrix = m_Camera.GetProjectionMatrix();

	// Update scene constant buffer
	SHADER_SCENE_BUFFER cbScene;
	ZeroMemory(&cbScene, sizeof(cbScene));

	cbScene.viewMatrix = viewMatrix;
	cbScene.cameraMatrix = m_Camera.GetNodeTransform();
	cbScene.projMatrix = projMatrix;
	cbScene.viewProjMatrix = viewMatrix * projMatrix;
	cbScene.invProjMatrix = projMatrix.Inverse();
	cbScene.cameraPos = m_Camera.GetPosition();

	RConstantBuffers::cbScene.UpdateContent(&cbScene);
	RConstantBuffers::cbScene.BindBuffer();

	if (m_EnableDeferredShading)
	{
		ID3D11RenderTargetView* rtvs[] =
		{
			m_DeferredBuffers[0].View,
			m_DeferredBuffers[1].View, 
			m_DeferredBuffers[2].View, 
			m_DeferredBuffers[3].View,
		};

		GRenderer.SetRenderTargets(DeferredBuffer_Count, rtvs, m_DepthBuffer.View);

		GRenderer.Clear(false, RColor(0, 0, 0));
		GRenderer.ClearRenderTarget(m_DeferredBuffers[DB_Color].View, RColor(0.05f, 0.05f, 0.1f, 0.0f));
		GRenderer.ClearRenderTarget(m_DeferredBuffers[DB_Position].View, RColor(0, 0, 0, 1));
		GRenderer.ClearRenderTarget(m_DeferredBuffers[DB_WorldSpaceNormal].View, RColor(0, 0, 0));
		GRenderer.ClearRenderTarget(m_DeferredBuffers[DB_ViewSpaceNormal].View, RColor(0, 0, 0));

	}
	else
	{
		GRenderer.SetRenderTargets();
		GRenderer.Clear(true, RColor(0.05f, 0.05f, 0.1f));

		m_Skybox.Draw();
		GRenderer.Clear(false, RColor(0, 0, 0));
	}

	RFrustum frustum = m_Camera.GetFrustum();

	GRenderer.SetBlendState(Blend_Opaque);
	GRenderer.D3DImmediateContext()->RSSetState(m_RasterizerStates[RS_Default]);

	if (m_EnableDeferredShading)
	{
		GRenderer.SetDefferedShading(true);

		m_Scene.Render(&frustum);

		GRenderer.SetDefferedShading(false);

		if (m_EnableSSR)
			GRenderer.SetRenderTargets(1, &m_ScenePassBuffer.View, m_DepthBuffer.View);
		else
			GRenderer.SetRenderTargets();
		GRenderer.Clear(true, RColor(0, 0, 0));

		ID3D11ShaderResourceView* gbufferSRV[DeferredBuffer_Count];
		for (int i = 0; i < DeferredBuffer_Count; i++)
		{
			gbufferSRV[i] = m_DeferredBuffers[i].SRV;
		};

		GRenderer.D3DImmediateContext()->PSSetShaderResources(0, DeferredBuffer_Count, gbufferSRV);

		m_PostProcessor.Draw(PPE_DeferredComposition);

		// Render lighting pass
		GRenderer.SetBlendState(Blend_Additive);

		const RMatrix4& viewProjMatrix = m_Camera.GetViewMatrix() * m_Camera.GetProjectionMatrix();

		for (int i = 0; i < min(m_LightCount, MAX_LIGHT_COUNT); i++)
		{
			float x = sinf(m_TotalTime * m_PointLights[i].sin_factor.X() + m_PointLights[i].sin_offset.X()) * 1000.0f;
			float y = sinf(m_TotalTime * m_PointLights[i].sin_factor.Y() + m_PointLights[i].sin_offset.Y()) * 1000.0f;
			float z = sinf(m_TotalTime * m_PointLights[i].sin_factor.Z() + m_PointLights[i].sin_offset.Z()) * 1000.0f;
			RVec3 offset = RVec3(x, y, z);

			RVec3 pos = m_PointLights[i].pos + offset;

			RAabb aabb;
			float r = m_PointLights[i].r * m_LightRadius;
			aabb.pMin = pos - RVec3(r, r, r);
			aabb.pMax = pos + RVec3(r, r, r);

			RVec2 pMin = RVec2(FLT_MAX, FLT_MAX), pMax = RVec2(-FLT_MAX, -FLT_MAX);
			if (aabb.TestPointInsideAabb(m_Camera.GetPosition()))
			{
				pMin.x = 0.0f;
				pMin.y = (float)GRenderer.GetClientHeight();
				pMax.x = (float)GRenderer.GetClientWidth();
				pMax.y = 0.0f;
			}
			else
			{
				RVec3 corners[] =
				{
					RVec3(aabb.pMin.X(), aabb.pMin.Y(), aabb.pMin.Z()),
					RVec3(aabb.pMin.X(), aabb.pMax.Y(), aabb.pMin.Z()),
					RVec3(aabb.pMax.X(), aabb.pMax.Y(), aabb.pMin.Z()),
					RVec3(aabb.pMax.X(), aabb.pMin.Y(), aabb.pMin.Z()),

					RVec3(aabb.pMin.X(), aabb.pMin.Y(), aabb.pMax.Z()),
					RVec3(aabb.pMin.X(), aabb.pMax.Y(), aabb.pMax.Z()),
					RVec3(aabb.pMax.X(), aabb.pMax.Y(), aabb.pMax.Z()),
					RVec3(aabb.pMax.X(), aabb.pMin.Y(), aabb.pMax.Z()),
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

				//GDebugRenderer.DrawLine(RVec3(pMin.x, pMin.y, 0), RVec3(pMin.x, pMax.y, 0));
				//GDebugRenderer.DrawLine(RVec3(pMin.x, pMax.y, 0), RVec3(pMax.x, pMax.y, 0));
				//GDebugRenderer.DrawLine(RVec3(pMax.x, pMax.y, 0), RVec3(pMax.x, pMin.y, 0));
				//GDebugRenderer.DrawLine(RVec3(pMax.x, pMin.y, 0), RVec3(pMin.x, pMin.y, 0));

				pMin.x = (pMin.x + 1.0f) * 0.5f * GRenderer.GetClientWidth();
				pMin.y = (-pMin.y + 1.0f) * 0.5f * GRenderer.GetClientHeight();
				pMax.x = (pMax.x + 1.0f) * 0.5f * GRenderer.GetClientWidth();
				pMax.y = (-pMax.y + 1.0f) * 0.5f * GRenderer.GetClientHeight();
			}

			if (pMin.x > pMax.x || pMax.y > pMin.y)
				continue;

			if (RCollision::TestAabbInsideFrustum(frustum, aabb))
			{
				//GDebugRenderer.DrawSphere(pos, m_PointLights[i].r, m_PointLights[i].color);

				if (m_RenderLightPos)
					GDebugRenderer.DrawSphere(pos, 10.0f, m_PointLights[i].color, 3);

				if (m_EnablePointLightShadow && m_PointLights[i].castShadow)
				{
					RenderPointLightCubemapDepth(pos, r);

					RConstantBuffers::cbScene.UpdateContent(&cbScene);
					RConstantBuffers::cbScene.BindBuffer();

					if (m_EnableSSR)
						GRenderer.SetRenderTargets(1, &m_ScenePassBuffer.View, m_DepthBuffer.View);
					else
						GRenderer.SetRenderTargets();

					float width = static_cast<float>(GRenderer.GetClientWidth());
					float height = static_cast<float>(GRenderer.GetClientHeight());
					D3D11_VIEWPORT vp = { 0.0f, 0.0f, width, height, 0.0f, 1.0f };
					GRenderer.D3DImmediateContext()->RSSetViewports(1, &vp);

					GRenderer.D3DImmediateContext()->PSSetShaderResources(6, 1, &m_CubeDepthBuffer.DepthSRV);
				}

				GRenderer.D3DImmediateContext()->RSSetState(m_RasterizerStates[RS_Scissor]);

				D3D11_RECT rect;
				rect.left = (LONG)pMin.x;
				rect.top = (LONG)pMax.y;
				rect.right = (LONG)pMax.x;
				rect.bottom = (LONG)pMin.y;

				GRenderer.D3DImmediateContext()->RSSetScissorRects(1, &rect);

				SHADER_DEFERRED_POINT_LIGHT_BUFFER cbDeferredPointLight;

				cbDeferredPointLight.DeferredPointLight.PosAndRadius = RVec4(pos, r);
				cbDeferredPointLight.DeferredPointLight.Color = RVec4((float*)&m_PointLights[i].color);
				cbDeferredPointLight.CastShadow = m_EnablePointLightShadow && m_PointLights[i].castShadow;

				m_cbDeferredPointLight.UpdateContent(&cbDeferredPointLight);
				m_cbDeferredPointLight.BindBuffer();

				GRenderer.Clear(false, RColor(0, 0, 0), true);
				m_PostProcessor.Draw(PPE_DeferredPointLightPass);

				if (m_EnablePointLightShadow && m_PointLights[i].castShadow)
				{
					ID3D11ShaderResourceView* nullSRV = nullptr;
					GRenderer.D3DImmediateContext()->PSSetShaderResources(6, 1, &nullSRV);
				}
			}
		}

		// Disable scissor test
		GRenderer.D3DImmediateContext()->RSSetState(m_RasterizerStates[RS_Default]);

		if (m_EnableSSR)
		{
			//RRenderer.D3DImmediateContext()->GenerateMips(m_ScenePassBuffer.SRV);

			GRenderer.SetRenderTargets();
			GRenderer.SetBlendState(Blend_Opaque);

			GRenderer.D3DImmediateContext()->PSSetShaderResources(0, 1, &m_ScenePassBuffer.SRV);
			GRenderer.D3DImmediateContext()->PSSetShaderResources(4, 1, m_EnvCube->GetPtrSRV());

			GRenderer.Clear(false, RColor(0, 0, 0), true);
			m_PostProcessor.Draw(PPE_ScreenSpaceRayTracing);
		}

		ID3D11ShaderResourceView* nullSRV[DeferredBuffer_Count] = { nullptr };
		GRenderer.D3DImmediateContext()->PSSetShaderResources(0, DeferredBuffer_Count, nullSRV);
	}
	else
	{
		m_Scene.Render(&frustum);
	}

	GRenderer.Clear(false, RColor(0, 0, 0));

	GRenderer.SetBlendState(Blend_Opaque);
	GRenderer.D3DImmediateContext()->RSSetState(m_RasterizerStates[RS_Default]);

	SHADER_OBJECT_BUFFER cbObject;
	cbObject.worldMatrix = RMatrix4::IDENTITY;

	RConstantBuffers::cbPerObject.UpdateContent(&cbObject);
	RConstantBuffers::cbPerObject.BindBuffer();

	GDebugRenderer.Render();
	GDebugRenderer.Reset();

	m_DebugMenu.Render();

	GRenderer.Present();
}

void DeferredShadingApp::OnResize(int width, int height)
{
	if (GRenderer.D3DDevice())
	{
		for (int i = 0; i < DeferredBuffer_Count; i++)
		{
			m_DeferredBuffers[i].Release();
		}
		m_ScenePassBuffer.Release();
		m_DepthBuffer.Release();

		m_Camera.SetAspectRatio((float)width / (float)height);

		CreateGBuffers();
	}
}

void DeferredShadingApp::CreateGBuffers()
{
	m_DeferredBuffers[DB_Color]				= CreateRenderTarget(DXGI_FORMAT_R8G8B8A8_UNORM, "G-Buffer Color/AO");
	m_DeferredBuffers[DB_Position]			= CreateRenderTarget(DXGI_FORMAT_R32G32B32A32_FLOAT, "G-Buffer Position/Depth");
	m_DeferredBuffers[DB_WorldSpaceNormal]	= CreateRenderTarget(DXGI_FORMAT_R16G16B16A16_FLOAT, "G-Buffer World Space Normal");
	m_DeferredBuffers[DB_ViewSpaceNormal]	= CreateRenderTarget(DXGI_FORMAT_R16G16B16A16_FLOAT, "G-Buffer View Space Normal");
	m_ScenePassBuffer = CreateRenderTarget(DXGI_FORMAT_R32G32B32A32_FLOAT);
	m_DepthBuffer = CreateDepthStencilBuffer();
}

DeferredRenderBuffer DeferredShadingApp::CreateRenderTarget(DXGI_FORMAT format, const char* debugResourceName/*=nullptr*/)
{
	DeferredRenderBuffer rb;

	D3D11_TEXTURE2D_DESC renderTargetTextureDesc;
	renderTargetTextureDesc.Width = GRenderer.GetClientWidth();
	renderTargetTextureDesc.Height = GRenderer.GetClientHeight();
	renderTargetTextureDesc.MipLevels = 1;
	renderTargetTextureDesc.ArraySize = 1;
	renderTargetTextureDesc.Format = format;
	renderTargetTextureDesc.SampleDesc.Count = 1;
	renderTargetTextureDesc.SampleDesc.Quality = 0;
	renderTargetTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	renderTargetTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	renderTargetTextureDesc.CPUAccessFlags = 0;
	renderTargetTextureDesc.MiscFlags = 0;

	GRenderer.D3DDevice()->CreateTexture2D(&renderTargetTextureDesc, 0, &rb.Buffer);
	GRenderer.D3DDevice()->CreateRenderTargetView(rb.Buffer, 0, &rb.View);

	D3D11_SHADER_RESOURCE_VIEW_DESC rtsrvDesc;
	rtsrvDesc.Format = renderTargetTextureDesc.Format;
	rtsrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	rtsrvDesc.Texture2D.MostDetailedMip = 0;
	rtsrvDesc.Texture2D.MipLevels = 1;

	GRenderer.D3DDevice()->CreateShaderResourceView(rb.Buffer, &rtsrvDesc, &rb.SRV);

#ifdef _DEBUG
	if (debugResourceName)
	{
		rb.Buffer->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)strlen(debugResourceName), debugResourceName);
		rb.View->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)strlen(debugResourceName), debugResourceName);
		rb.SRV->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)strlen(debugResourceName), debugResourceName);
	}
#endif // _DEBUG


	return rb;
}

DepthStencilBuffer DeferredShadingApp::CreateDepthStencilBuffer()
{
	DepthStencilBuffer db;

	D3D11_TEXTURE2D_DESC renderTargetTextureDesc;
	renderTargetTextureDesc.Width = GRenderer.GetClientWidth();
	renderTargetTextureDesc.Height = GRenderer.GetClientHeight();
	renderTargetTextureDesc.MipLevels = 1;
	renderTargetTextureDesc.ArraySize = 1;
	renderTargetTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	renderTargetTextureDesc.SampleDesc.Count = 1;
	renderTargetTextureDesc.SampleDesc.Quality = 0;
	renderTargetTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	renderTargetTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	renderTargetTextureDesc.CPUAccessFlags = 0;
	renderTargetTextureDesc.MiscFlags = 0;

	GRenderer.D3DDevice()->CreateTexture2D(&renderTargetTextureDesc, 0, &db.Buffer);
	GRenderer.D3DDevice()->CreateDepthStencilView(db.Buffer, 0, &db.View);

	return db;
}

CubeDepthBuffer DeferredShadingApp::CreateCubeDepthBuffer()
{
	CubeDepthBuffer cdb;

	D3D11_TEXTURE2D_DESC cubeTexDesc;
	cubeTexDesc.Width = 256;
	cubeTexDesc.Height = 256;
	cubeTexDesc.MipLevels = 1;
	cubeTexDesc.ArraySize = 6;
	cubeTexDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	cubeTexDesc.SampleDesc.Count = 1;
	cubeTexDesc.SampleDesc.Quality = 0;
	cubeTexDesc.Usage = D3D11_USAGE_DEFAULT;
	cubeTexDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	cubeTexDesc.CPUAccessFlags = 0;
	cubeTexDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	GRenderer.D3DDevice()->CreateTexture2D(&cubeTexDesc, 0, &cdb.Buffer);

	D3D11_RENDER_TARGET_VIEW_DESC cubeRTVDesc;
	cubeRTVDesc.Format = cubeTexDesc.Format;
	cubeRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	cubeRTVDesc.Texture2DArray.ArraySize = 1;
	cubeRTVDesc.Texture2DArray.MipSlice = 0;
	for (int i = 0; i < 6; i++)
	{
		cubeRTVDesc.Texture2DArray.FirstArraySlice = i;
		GRenderer.D3DDevice()->CreateRenderTargetView(cdb.Buffer, &cubeRTVDesc, &cdb.View[i]);
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC cubeSRVDesc;
	cubeSRVDesc.Format = cubeRTVDesc.Format;
	cubeSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	cubeSRVDesc.TextureCube.MipLevels = 1;
	cubeSRVDesc.TextureCube.MostDetailedMip = 0;
	GRenderer.D3DDevice()->CreateShaderResourceView(cdb.Buffer, &cubeSRVDesc, &cdb.SRV);

	cubeTexDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	cubeTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	GRenderer.D3DDevice()->CreateTexture2D(&cubeTexDesc, 0, &cdb.DepthBuffer);

	D3D11_DEPTH_STENCIL_VIEW_DESC cubeDSVDesc;
	cubeDSVDesc.Flags = 0;
	cubeDSVDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	cubeDSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	cubeDSVDesc.Texture2DArray.ArraySize = 1;
	cubeDSVDesc.Texture2DArray.MipSlice = 0;

	for (int i = 0; i < 6; i++)
	{
		cubeDSVDesc.Texture2DArray.FirstArraySlice = i;
		GRenderer.D3DDevice()->CreateDepthStencilView(cdb.DepthBuffer, &cubeDSVDesc, &cdb.DepthView[i]);
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC depthSRVDesc;
	depthSRVDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	depthSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	depthSRVDesc.Texture2D.MostDetailedMip = 0;
	depthSRVDesc.Texture2D.MipLevels = 1;

	GRenderer.D3DDevice()->CreateShaderResourceView(cdb.DepthBuffer, &depthSRVDesc, &cdb.DepthSRV);


	return cdb;
}

void DeferredShadingApp::RenderPointLightCubemapDepth(const RVec3& position, float radius)
{
	static RCamera cubeCameras[6];
	RVec3 targets[6] =
	{
		position + RVec3(1, 0, 0),
		position + RVec3(-1, 0, 0),
		position + RVec3(0, 1, 0),
		position + RVec3(0, -1, 0),
		position + RVec3(0, 0, 1),
		position + RVec3(0, 0, -1),
	};

	RVec3 ups[6] =
	{
		RVec3(0, 1, 0),
		RVec3(0, 1, 0),
		RVec3(0, 0, -1),
		RVec3(0, 0, 1),
		RVec3(0, 1, 0),
		RVec3(0, 1, 0),
	};

	D3D11_VIEWPORT vp = { 0.0f, 0.0f, 256, 256, 0.0f, 1.0f };
	GRenderer.D3DImmediateContext()->RSSetViewports(1, &vp);

	for (int i = 0; i < 6; i++)
	{
		cubeCameras[i].SetPosition(position);
		cubeCameras[i].SetupView(90.0f, 1.0f, 0.1f, radius);
		cubeCameras[i].LookAt(targets[i], ups[i]);

		GRenderer.SetRenderTargets(1, &m_CubeDepthBuffer.View[i], m_CubeDepthBuffer.DepthView[i]);
		GRenderer.Clear();

		// Update scene constant buffer
		SHADER_SCENE_BUFFER cbScene;
		ZeroMemory(&cbScene, sizeof(cbScene));

		cbScene.viewMatrix = cubeCameras[i].GetViewMatrix();
		cbScene.cameraMatrix = cubeCameras[i].GetNodeTransform();
		cbScene.projMatrix = cubeCameras[i].GetProjectionMatrix();
		cbScene.viewProjMatrix = cbScene.viewMatrix * cbScene.projMatrix;
		cbScene.invProjMatrix = cbScene.projMatrix.Inverse();
		cbScene.cameraPos = cubeCameras[i].GetPosition();
		cbScene.shadowViewProjMatrix[0] = cbScene.viewProjMatrix;
		cbScene.cascadedShadowIndex = 0;

		RConstantBuffers::cbScene.UpdateContent(&cbScene);
		RConstantBuffers::cbScene.BindBuffer();

		RFrustum frustum = cubeCameras[i].GetFrustum();
		m_Scene.RenderDepthPass(&frustum);
	}
}
