//=============================================================================
// WorkshopApp.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Core/CoreTypes.h"
#include "Core/IApp.h"
#include "RAssetsViewWindow.h"
#include "RAssetEditorWindow.h"
#include "RResourcePreviewBuilder.h"

class RSceneObject;
class RResourceBase;
class RFreeFlyCameraControl;

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

	RFreeFlyCameraControl* GetFreeFlyCamera() const;
	float GetCameraSpeed() const;
	void SetCameraSpeed(float InSpeed);
	void FocusOnSelection();

	// Call this function to set a current selected object so its transforms get updated
	void SetSelectedObject(RSceneObject* InSelected);

	std::vector<RSceneObject*> GetAllSceneObjects() const;

	RVec2 GetMousePositionInViewport() const;
	RRay MakeRayFromViewportPoint(const RVec2& Point)const;
	void SelectSceneObjectAtCursor(const RVec2& Point);

private:

	void DisplaySceneViewWindow();

private:
	int WindowWidth, WindowHeight;
	bool bShowOpenDialog;

	bool bShowAssetEditor;

	RResourcePreviewBuilder ResourcePreviewBuilder;

	RAssetsViewWindow AssetsViewWindow;
	RAssetEditorWindow AssetEditorWindow;

	// A list of paths to all maps in the asset folder
	std::vector<std::string> EngineMaps;
	std::string CurrentMapPath;

	RSceneObject* SelectedObject;

	float PosValueArray[3];
	float RotValueArray[3];
	float ScaleValueArray[3];
};
