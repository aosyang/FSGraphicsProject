//=============================================================================
// RAssetEditorWindow.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

class RResourceBase;
class RResourcePreviewBuilder;

class RAssetEditorWindow
{
public:
	RAssetEditorWindow();

	void SetEditingResource(RResourceBase* NewEditingResource);

	void ShowWindow(RResourcePreviewBuilder& PreviewBuilder, RResourceBase* AssetsViewResource);

	/// Visibility of asset editor window
	bool bShowWindow;
private:

	RResourceBase* EditingResource;
};
