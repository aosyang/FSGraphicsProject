//=============================================================================
// RSceneManager.h by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Core/RSingleton.h"
#include "RScene.h"

/// A manager for all active scenes in the engine
class RSceneManager : public RSingleton<RSceneManager>
{
	friend class RSingleton<RSceneManager>;
public:
	/// Initializer the scene manager
	void Initialize();

	/// Shutdown the scene manager
	void Shutdown();

	/// Register a scene to the manager so it will be updated automatically
	void RegisterScene(RScene* Scene);

	/// Unregister a scene from the manager
	void UnregisterScene(RScene* Scene);

	/// Get the default scene for adding new objects to
	RScene* DefaultScene();

	/// Update the scene manager
	void Update(float DeltaTime);

protected:
	RSceneManager() {}
	virtual ~RSceneManager() override {}

	/// The default scene
	RScene m_DefaultScene;

	/// A list of all registered scenes
	std::vector<RScene*> RegisteredScenes;
};

FORCEINLINE RScene* RSceneManager::DefaultScene()
{
	return &m_DefaultScene;
}

#define GSceneManager RSceneManager::Instance()
