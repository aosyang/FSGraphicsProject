//=============================================================================
// FightingGameApp.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "FightingGameApp.h"

FightingGameApp::FightingGameApp()
{

}

FightingGameApp::~FightingGameApp()
{
	m_Scene.Release();
	RShaderManager::Instance().UnloadAllShaders();
	RResourceManager::Instance().Destroy();
}

bool FightingGameApp::Initialize()
{
	RResourceManager::Instance().Initialize();
	//RResourceManager::Instance().LoadAllResources();
	RShaderManager::Instance().LoadShaders("../Shaders");

	m_Scene.Initialize();

	m_Scene.LoadFromFile("../Assets/TestMap.rmap");

	RMatrix4 cameraMatrix = RMatrix4::CreateXAxisRotation(0.09f * 180 / PI) * RMatrix4::CreateYAxisRotation(3.88659930f * 180 / PI);
	cameraMatrix.SetTranslation(RVec3(407.023712f, 339.007507f, 876.396484f));
	m_Camera.SetTransform(cameraMatrix);
	m_Camera.SetupView(65.0f, RRenderer.AspectRatio(), 0.1f, 10000.0f);

	return true;
}

void FightingGameApp::UpdateScene(const RTimer& timer)
{

}

void FightingGameApp::RenderScene()
{

	// Update scene constant buffer
	SHADER_SCENE_BUFFER cbScene;

	cbScene.viewMatrix = m_Camera.GetViewMatrix();
	cbScene.projMatrix = m_Camera.GetProjectionMatrix();
	cbScene.viewProjMatrix = cbScene.viewMatrix * cbScene.projMatrix;
	cbScene.cameraPos = m_Camera.GetPosition();

	m_Scene.cbScene.UpdateContent(&cbScene);
	m_Scene.cbScene.ApplyToShaders();


	// Update light constant buffer
	SHADER_LIGHT_BUFFER cbLight;
	ZeroMemory(&cbLight, sizeof(cbLight));

	// Setup ambient color
	cbLight.HighHemisphereAmbientColor = RVec4(1.0f, 1.0f, 1.0f, 1.0f);
	cbLight.LowHemisphereAmbientColor = RVec4(0.2f, 0.2f, 0.2f, 1.0f);

	m_Scene.cbLight.UpdateContent(&cbLight);
	m_Scene.cbLight.ApplyToShaders();

	RRenderer.SetSamplerState(0, SamplerState_Texture);
	RRenderer.SetSamplerState(2, SamplerState_ShadowDepthComparison);

	RRenderer.Clear();

	m_Scene.Render();

	RRenderer.Present();
}

void FightingGameApp::OnResize(int width, int height)
{

}
