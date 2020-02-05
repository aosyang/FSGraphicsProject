//=============================================================================
// WorkshopApp.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Core/CoreTypes.h"
#include "Core/IApp.h"

class WorkshopApp : public IApp
{
public:
	WorkshopApp();

	virtual bool Initialize() override;
	virtual void UpdateScene(const RTimer& timer) override;
	virtual void RenderScene() override;

	virtual void OnResize(int width, int height) override;
private:
	void UpdateEngineMapList();

	void PostMapLoaded();

private:
	int WindowWidth, WindowHeight;
	bool bShowOpenDialog;

	// A list of paths to all maps in the asset folder
	std::vector<std::string> EngineMaps;
	std::string CurrentMapPath;
};
