//=============================================================================
// RSceneManager.cpp by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RSceneManager.h"

void RSceneManager::Initialize()
{
	m_DefaultScene.Initialize();
	RegisterScene(&m_DefaultScene);
}

void RSceneManager::Shutdown()
{
	for (auto Scene : RegisteredScenes)
	{
		Scene->Release();
	}

	RegisteredScenes.clear();
}

void RSceneManager::RegisterScene(RScene* Scene)
{
	// Scene must be valid
	assert(Scene);

	// Scene must not have been registered already
	assert(find(RegisteredScenes.begin(), RegisteredScenes.end(), Scene) == RegisteredScenes.end());

	RegisteredScenes.push_back(Scene);
}

void RSceneManager::UnregisterScene(RScene* Scene)
{
	auto Iter = find(RegisteredScenes.begin(), RegisteredScenes.end(), Scene);
	if (Iter != RegisteredScenes.end())
	{
		RegisteredScenes.erase(Iter);
	}
}

void RSceneManager::Update(float DeltaTime)
{
	// Update all registered scenes
	for (auto Scene : RegisteredScenes)
	{
		Scene->UpdateScene(DeltaTime);
	}
}

void RSceneManager::Update_PostPhysics(float DeltaTime)
{
	// Update all registered scenes
	for (auto Scene : RegisteredScenes)
	{
		Scene->UpdateScene_PostPhysics(DeltaTime);
	}
}
