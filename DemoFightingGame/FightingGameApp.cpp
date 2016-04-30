//=============================================================================
// FightingGameApp.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "FightingGameApp.h"

FightingGameApp::FightingGameApp()
	: m_Player(nullptr), m_DrawHitBound(false)
{

}

FightingGameApp::~FightingGameApp()
{
	SAFE_DELETE(m_Player);
	m_Scene.Release();
	m_DebugRenderer.Release();
	RShaderManager::Instance().UnloadAllShaders();
	RResourceManager::Instance().Destroy();
}

bool FightingGameApp::Initialize()
{
	RResourceManager::Instance().Initialize();
	//RResourceManager::Instance().LoadAllResources();
	RShaderManager::Instance().LoadShaders("../Shaders");

	m_DebugRenderer.Initialize();

	m_Scene.Initialize();
	m_Scene.LoadFromFile("../Assets/TestMap.rmap");

	m_ShadowMap.Initialize(1024, 1024);

	RMatrix4 cameraMatrix = RMatrix4::CreateXAxisRotation(0.09f * 180 / PI) * RMatrix4::CreateYAxisRotation(3.88659930f * 180 / PI);
	cameraMatrix.SetTranslation(RVec3(407.023712f, 339.007507f, 876.396484f));
	m_Camera.SetTransform(cameraMatrix);
	m_Camera.SetupView(65.0f, RRenderer.AspectRatio(), 1.0f, 10000.0f);

	//m_Player = (RSMeshObject*)m_Scene.FindObject("Player");
	//if (m_Player)
	//{
	//	m_Scene.RemoveObjectFromScene(m_Player);
	//}

	m_Player = new PlayerController();
	m_Player->SetScene(&m_Scene);
	m_Player->SetPosition(RVec3(0, 100.0f, 0));
	m_Player->Cache();

	m_AIPlayer = new PlayerController();
	m_AIPlayer->SetScene(&m_Scene);
	m_AIPlayer->SetPosition(RVec3(-465, 50, -760));
	m_AIPlayer->Cache();

	return true;
}

void FightingGameApp::UpdateScene(const RTimer& timer)
{
	// Update light constant buffer
	SHADER_LIGHT_BUFFER cbLight;
	ZeroMemory(&cbLight, sizeof(cbLight));

	// Setup ambient color
	cbLight.HighHemisphereAmbientColor = RVec4(0.5f, 0.5f, 0.5f, 1.0f);
	cbLight.LowHemisphereAmbientColor = RVec4(0.2f, 0.2f, 0.2f, 1.0f);

	RVec4 dirLightVec = RVec4(RVec3(0.25f, 1.0f, 0.5f).GetNormalizedVec3(), 1.0f);

	RVec3 sunVec = RVec3(sinf(1.0f) * 0.5f, 0.25f, cosf(1.0) * 0.5f).GetNormalizedVec3() * 2000.0f;
	RMatrix4 shadowViewMatrix = RMatrix4::CreateLookAtViewLH(sunVec, RVec3(0.0f, 0.0f, 0.0f), RVec3(0.0f, 1.0f, 0.0f));

	cbLight.DirectionalLightCount = 1;
	cbLight.DirectionalLight[0].Color = RVec4(1.0f, 1.0f, 0.8f, 1.0f);
	cbLight.DirectionalLight[0].Direction = RVec4(sunVec.GetNormalizedVec3(), 1.0f);


	// Update scene constant buffer
	SHADER_SCENE_BUFFER cbScene;

#if 1
	cbScene.viewMatrix = m_Camera.GetViewMatrix();
	cbScene.projMatrix = m_Camera.GetProjectionMatrix();
	cbScene.viewProjMatrix = cbScene.viewMatrix * cbScene.projMatrix;
	cbScene.cameraPos = m_Camera.GetPosition();

	cbLight.CameraPos = m_Camera.GetPosition();
#else
	RMatrix4 cameraMatrix = RMatrix4::CreateXAxisRotation(0.09f * 180 / PI) * RMatrix4::CreateYAxisRotation(3.88659930f * 180 / PI);
	cameraMatrix.SetTranslation(RVec3(407.023712f, 339.007507f, 876.396484f));

	cbScene.viewMatrix = RMatrix4::CreateLookAtViewLH(RVec3(407.023712f, 339.007507f, 876.396484f), m_Player->GetPosition(), RVec3(0, 1, 0));
	cbScene.projMatrix = RMatrix4::CreatePerspectiveProjectionLH(65.0f, RRenderer.AspectRatio(), 1.0f, 10000.0f);
	cbScene.viewProjMatrix = cbScene.viewMatrix * cbScene.projMatrix;
	cbScene.cameraPos = cameraMatrix.GetTranslation();

	cbLight.CameraPos = cameraMatrix.GetTranslation();
#endif

	m_ShadowMap.SetViewMatrix(shadowViewMatrix);
	m_ShadowMap.SetOrthogonalProjection(5000.0f, 5000.0f, 0.1f, 5000.0f);

	RMatrix4 shadowTransform(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	RMatrix4 shadowViewProjMatrix = m_ShadowMap.GetViewMatrix() * m_ShadowMap.GetProjectionMatrix();
	cbScene.shadowViewProjMatrix = shadowViewProjMatrix;
	shadowViewProjMatrix *= shadowTransform;
	cbScene.shadowViewProjBiasedMatrix = shadowViewProjMatrix;

	m_Scene.cbScene.UpdateContent(&cbScene);
	m_Scene.cbScene.ApplyToShaders();

	m_Scene.cbLight.UpdateContent(&cbLight);
	m_Scene.cbLight.ApplyToShaders();

	SHADER_MATERIAL_BUFFER cbMaterial;
	ZeroMemory(&cbMaterial, sizeof(cbMaterial));

	cbMaterial.SpecularColorAndPower = RVec4(1.0f, 1.0f, 1.0f, 512.0f);
	cbMaterial.GlobalOpacity = 1.0f;
	
	m_Scene.cbMaterial.UpdateContent(&cbMaterial);
	m_Scene.cbMaterial.ApplyToShaders();

	if (RInput.GetBufferedKeyState('P') == BKS_Pressed)
		m_DrawHitBound = !m_DrawHitBound;

	if (m_Player)
	{
		if (RInput.GetBufferedKeyState('R') == BKS_Pressed)
		{
			m_Player->SetPosition(RVec3(0, 100.0f, 0));
			m_AIPlayer->SetPosition(RVec3(-465, 50, -760));
		}

		m_Player->PreUpdate(timer);
		
		RVec3 moveVec = RVec3(0, 0, 0);

		RVec3 charRight = m_Camera.GetNodeTransform().GetRight();
		RVec3 charForward = charRight.Cross(RVec3(0, 1, 0));

		if (m_Player->GetBehavior() == BHV_Running ||
			m_Player->GetBehavior() == BHV_Idle)
		{
			if (RInput.IsKeyDown('W')) moveVec += charForward;
			if (RInput.IsKeyDown('S')) moveVec -= charForward;
			if (RInput.IsKeyDown('A')) moveVec -= charRight;
			if (RInput.IsKeyDown('D')) moveVec += charRight;

			if (moveVec.SquaredMagitude() > 0.0f)
			{
				moveVec.Normalize();
				moveVec *= timer.DeltaTime() * 500.0f;

				m_Player->SetBehavior(BHV_Running);
			}
			else
			{
				m_Player->SetBehavior(BHV_Idle);
			}

			if (RInput.GetBufferedKeyState(VK_SPACE) == BKS_Pressed)
			{
				m_Player->SetBehavior(BHV_SpinAttack);
			}

			if (RInput.GetBufferedKeyState('J') == BKS_Pressed)
			{
				m_Player->SetBehavior(BHV_Punch);
			}
		}

		if (m_Player->GetBehavior() == BHV_Running ||
			m_Player->GetBehavior() == BHV_Idle ||
			m_Player->GetBehavior() == BHV_Kick)
		{
			if (RInput.GetBufferedKeyState('K') == BKS_Pressed)
			{
				m_Player->SetBehavior(BHV_Kick);
			}
		}

		if (RInput.GetBufferedKeyState('C') == BKS_Pressed)
			m_Player->SetBehavior(BHV_HitDown);

		moveVec += RVec3(0, -1000.0f * timer.DeltaTime(), 0);
		m_Player->UpdateMovement(timer, moveVec);

		RMatrix4 playerTranslation = RMatrix4::CreateTranslation(m_Player->GetNodeTransform().GetTranslation());
		RMatrix4 cameraTransform = RMatrix4::CreateTranslation(0.0f, 500.0f, 300.0f) * playerTranslation;

		RAabb camAabb;
		RVec3 lookTarget = m_Player->GetPosition() + RVec3(0, 125, 0);
		camAabb.pMin = RVec3(-5, -5, -5) + lookTarget;
		camAabb.pMax = RVec3(5, 5, 5) + lookTarget;

		RVec3 camVec = cameraTransform.GetTranslation() - lookTarget;
		camVec = m_Scene.TestMovingAabbWithScene(camAabb, camVec);

		//m_Camera.SetTransform(cameraTransform);
		static RVec3 actualCamVec;
		actualCamVec = RVec3::Lerp(actualCamVec, camVec, 5.0f * timer.DeltaTime());
		m_Camera.SetPosition(actualCamVec + lookTarget);
		m_Camera.LookAt(lookTarget);

		//playerAabb.pMin = RVec3(-50.0f, 0.0f, -50.0f) + m_Player->GetPosition();
		//playerAabb.pMax = RVec3(50.0f, 150.0f, 50.0f) + m_Player->GetPosition();
		//m_DebugRenderer.DrawAabb(playerAabb);

		//m_DebugRenderer.DrawFrustum(m_Camera.GetFrustum());

		//RSphere s = { RVec3(0, 100, 0), 50.0f };
		//RCapsule cap = m_Player->GetCollisionShape();
		//RColor color = RColor(0, 1, 1);
		//if (RCollision::TestSphereWithCapsule(s, cap))
		//{
		//	color = RColor(1, 0, 0);
		//}
		//m_DebugRenderer.DrawSphere(s.center, s.radius, color);
		//m_DebugRenderer.DrawSphere(cap.start, cap.radius, color);
		//m_DebugRenderer.DrawSphere(cap.end, cap.radius, color);


		if (m_Player->GetBehavior() == BHV_SpinAttack &&
			m_Player->GetBehaviorTime() > 0.3f &&
			m_Player->GetBehaviorTime() < 0.6f)
		{
			RSphere hit_sphere;
			hit_sphere.center = m_Player->GetPosition() + m_Player->GetNodeTransform().GetForward() * 50 + RVec3(0, 50, 0);
			hit_sphere.radius = 50.0f;
			if (m_DrawHitBound)
				m_DebugRenderer.DrawSphere(hit_sphere.center, hit_sphere.radius);

			if (RCollision::TestSphereWithCapsule(hit_sphere, m_AIPlayer->GetCollisionShape()))
			{
				if (m_AIPlayer->GetBehavior() != BHV_HitDown)
				{
					RVec3 relVec = hit_sphere.center - m_AIPlayer->GetPosition();
					relVec.y = 0.0f;
					RVec3 playerForward = m_Player->GetNodeTransform().GetForward();
					if (playerForward.Dot(relVec) >= 0)
						relVec = -playerForward;
					relVec.Normalize();

					m_AIPlayer->SetPlayerRotation(RAD_TO_DEG(atan2f(relVec.x, relVec.z)));
				}
				m_AIPlayer->SetBehavior(BHV_HitDown);
			}
		}

		if (m_Player->GetBehavior() == BHV_Punch &&
			m_Player->GetBehaviorTime() > 0.1f &&
			m_Player->GetBehaviorTime() < 0.3f)
		{
			RSphere hit_sphere;
			hit_sphere.center = m_Player->GetPosition() + m_Player->GetNodeTransform().GetForward() * 50 + RVec3(0, 100, 0);
			hit_sphere.radius = 20.0f;
			if (m_DrawHitBound)
				m_DebugRenderer.DrawSphere(hit_sphere.center, hit_sphere.radius);

			if (RCollision::TestSphereWithCapsule(hit_sphere, m_AIPlayer->GetCollisionShape()))
			{
				if (m_AIPlayer->GetBehavior() != BHV_HitDown &&
					m_AIPlayer->GetBehavior() != BHV_Hit)
				{
					m_AIPlayer->SetBehavior(BHV_Hit);
				}
			}
		}

		if (m_Player->GetBehavior() == BHV_Kick &&
			m_Player->GetBehaviorTime() > 0.1f &&
			m_Player->GetBehaviorTime() < 0.3f)
		{
			RSphere hit_sphere;
			hit_sphere.center = m_Player->GetPosition() + m_Player->GetNodeTransform().GetForward() * 50 + RVec3(0, 100, 0);
			hit_sphere.radius = 50.0f;
			if (m_DrawHitBound)
				m_DebugRenderer.DrawSphere(hit_sphere.center, hit_sphere.radius);

			if (RCollision::TestSphereWithCapsule(hit_sphere, m_AIPlayer->GetCollisionShape()))
			{
				if (m_AIPlayer->GetBehavior() != BHV_HitDown &&
					m_AIPlayer->GetBehavior() != BHV_Hit)
				{
					m_AIPlayer->SetBehavior(BHV_Hit);
				}
			}
		}

		if (m_Player->GetBehavior() == BHV_BackKick &&
			m_Player->GetBehaviorTime() > 0.1f &&
			m_Player->GetBehaviorTime() < 0.3f)
		{
			RSphere hit_sphere;
			hit_sphere.center = m_Player->GetPosition() + m_Player->GetNodeTransform().GetForward() * 30 + RVec3(0, 100, 0);
			hit_sphere.radius = 50.0f;
			if (m_DrawHitBound)
				m_DebugRenderer.DrawSphere(hit_sphere.center, hit_sphere.radius);

			if (RCollision::TestSphereWithCapsule(hit_sphere, m_AIPlayer->GetCollisionShape()))
			{
				if (m_AIPlayer->GetBehavior() != BHV_HitDown)
				{
					RVec3 relVec = hit_sphere.center - m_AIPlayer->GetPosition();
					relVec.y = 0.0f;
					RVec3 playerForward = m_Player->GetNodeTransform().GetForward();
					if (playerForward.Dot(relVec) >= 0)
						relVec = -playerForward;
					relVec.Normalize();
					
					m_AIPlayer->SetPlayerRotation(RAD_TO_DEG(atan2f(relVec.x, relVec.z)));
					m_AIPlayer->SetBehavior(BHV_HitDown);
				}
			}
		}

		m_Player->PostUpdate(timer);
	}

	// Update AI player
	m_AIPlayer->PreUpdate(timer);
	RVec3 moveVec = RVec3(0, 0, 0);
	moveVec += RVec3(0, -1000.0f * timer.DeltaTime(), 0);
	m_AIPlayer->UpdateMovement(timer, moveVec);
	m_AIPlayer->PostUpdate(timer);
}

void FightingGameApp::RenderScene()
{
	RRenderer.SetSamplerState(0, SamplerState_Texture);
	RRenderer.SetSamplerState(2, SamplerState_ShadowDepthComparison);

	float width = static_cast<float>(RRenderer.GetClientWidth());
	float height = static_cast<float>(RRenderer.GetClientHeight());
	D3D11_VIEWPORT vp = { 0.0f, 0.0f, width, height, 0.0f, 1.0f };


	SHADER_OBJECT_BUFFER cbObject;

	for (int pass = 0; pass < 2; pass++)
	{
		if (pass == 0)
		{
			ID3D11ShaderResourceView* nullSRV[] = { nullptr };
			RRenderer.D3DImmediateContext()->PSSetShaderResources(2, 1, nullSRV);
			
			m_ShadowMap.SetupRenderTarget();
		}
		else
		{
			RRenderer.SetRenderTarget();
			RRenderer.D3DImmediateContext()->RSSetViewports(1, &vp);

			ID3D11ShaderResourceView* shadowMapSRV[] = { m_ShadowMap.GetRenderTargetDepthSRV() };
			RRenderer.D3DImmediateContext()->PSSetShaderResources(2, 1, shadowMapSRV);
		}

		RRenderer.Clear();

		if (pass == 0)
		{
			m_Scene.RenderDepthPass();
		}
		else
		{
			RFrustum frustum = m_Camera.GetFrustum();
			m_Scene.Render(&frustum);
		}

		if (m_Player)
		{
			cbObject.worldMatrix = m_Player->GetNodeTransform();
			m_Scene.cbPerObject.UpdateContent(&cbObject);
			m_Scene.cbPerObject.ApplyToShaders();
			if (pass == 0)
			{
				m_Player->DrawDepthPass();
				m_AIPlayer->DrawDepthPass();
			}
			else
			{
				RRenderer.SetBlendState(Blend_AlphaBlending);
				m_Player->Draw();
				m_AIPlayer->Draw();
				RRenderer.SetBlendState(Blend_Opaque);
			}
		}
	}

	cbObject.worldMatrix = RMatrix4::IDENTITY;
	m_Scene.cbPerObject.UpdateContent(&cbObject);
	m_Scene.cbPerObject.ApplyToShaders();
	m_DebugRenderer.Render();
	m_DebugRenderer.Reset();

	RRenderer.Present();
}

void FightingGameApp::OnResize(int width, int height)
{
	m_Camera.SetAspectRatio((float)width / (float)height);
}
